// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 3: per-symbol detail monitor.

#include "detail.hpp"

#include "selector.hpp"
#include "status_reporter.hpp"

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp>
#include <binapi2/fapi/streams/dynamic_market_stream.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fapi = binapi2::fapi;
namespace streams = binapi2::fapi::streams;
namespace types = binapi2::fapi::types;

namespace binapi2::examples::async_recorder {

namespace {

using rfs = streams::sinks::rotating_file_sink;

/// @brief All per-symbol sinks the detail monitor writes to.
///        Step 6 scope: two Tier-0 streams per symbol.
struct per_symbol_sinks
{
    std::unique_ptr<rfs> agg_trade;
    std::unique_ptr<rfs> book_ticker;
    std::size_t frames_routed{ 0 };
};

streams::sinks::rotating_file_sink_config
make_detail_rfs_cfg(const recorder_config& cfg,
                    const std::string& symbol_upper,
                    const std::string& stream_type)
{
    std::filesystem::path dir = cfg.root_dir / "detail" / symbol_upper / stream_type;
    std::filesystem::create_directories(dir);

    streams::sinks::rotating_file_sink_config rfs_cfg;
    rfs_cfg.dir = std::move(dir);
    rfs_cfg.basename = stream_type;
    rfs_cfg.extension = ".jsonl";
    rfs_cfg.max_size_bytes = cfg.rotation_size_bytes;
    rfs_cfg.max_seconds = cfg.rotation_seconds;
    rfs_cfg.compress = true;
    return rfs_cfg;
}

/// @brief Extract the combined-endpoint stream header from a raw frame,
///        returning `{symbol_lower, stream_type}`. Returns nullopt if
///        the frame isn't in the `{"stream":"X@Y",...}` shape — cheap
///        substring search, not a full JSON parse.
std::optional<std::pair<std::string, std::string>>
parse_stream_header(std::string_view frame)
{
    static constexpr std::string_view key = "\"stream\":\"";
    auto k = frame.find(key);
    if (k == std::string_view::npos) return std::nullopt;
    auto begin = k + key.size();
    auto end = frame.find('"', begin);
    if (end == std::string_view::npos) return std::nullopt;

    std::string_view topic{ frame.data() + begin, end - begin };
    auto at = topic.find('@');
    if (at == std::string_view::npos) return std::nullopt;

    return std::make_pair(std::string(topic.substr(0, at)),
                          std::string(topic.substr(at + 1)));
}

std::string to_upper_copy(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
        out.push_back(static_cast<char>(
            std::toupper(static_cast<unsigned char>(c))));
    return out;
}

/// @brief Holds all detail-monitor state. Members are accessed from
///        two coroutines (drain + subscription manager) on the same
///        executor, so no synchronisation is needed.
struct detail_state
{
    fapi::detail::stream_buffer<std::string> record_buf;
    std::unordered_map<std::string, per_symbol_sinks> sinks{};
    std::unordered_set<std::string> subscribed{};
    bool closed{ false };

    explicit detail_state(std::size_t capacity) : record_buf(capacity) {}
};

boost::cobalt::task<void>
drain_loop(detail_state& st)
{
    while (true) {
        auto r = co_await st.record_buf.async_read();
        if (!r) break;

        const auto& frame = *r;
        auto parsed = parse_stream_header(frame);
        if (!parsed) continue;
        const auto upper = to_upper_copy(parsed->first);

        auto sym_it = st.sinks.find(upper);
        if (sym_it == st.sinks.end()) continue;  // unsubscribed mid-flight
        auto& psink = sym_it->second;

        const auto& type = parsed->second;
        if (type == "aggTrade" && psink.agg_trade) {
            co_await (*psink.agg_trade)(frame);
            ++psink.frames_routed;
        } else if (type == "bookTicker" && psink.book_ticker) {
            co_await (*psink.book_ticker)(frame);
            ++psink.frames_routed;
        }
    }
}

/// @brief Drive the connection's read loop so raw frames keep flowing
///        into the attached record_buf. Free function (not a lambda)
///        to dodge the GCC 15 coroutine-lambda ICE.
boost::cobalt::task<void>
connection_read_loop(streams::dynamic_market_stream& d)
{
    while (true) {
        auto r = co_await d.connection().async_read_text();
        if (!r) break;
    }
}

boost::cobalt::task<void>
manage_subs_loop(const recorder_config& cfg,
                 selector& sel,
                 streams::dynamic_market_stream& dyn,
                 detail_state& st,
                 boost::asio::io_context& ioc)
{
    boost::asio::steady_timer timer(ioc);
    const auto interval = std::chrono::seconds(
        std::max<std::uint64_t>(1, cfg.stats_interval_seconds));

    while (!st.closed) {
        timer.expires_after(interval);
        try {
            co_await timer.async_wait(boost::cobalt::use_op);
        } catch (const std::exception&) {
            break;  // shutdown — timer cancelled
        }

        // Snapshot the selector's active set and diff against what
        // we already have open.
        const auto& target = sel.active();

        std::vector<std::string> to_add;
        std::vector<std::string> to_remove;
        for (const auto& s : target)
            if (!st.subscribed.count(s)) to_add.push_back(s);
        for (const auto& s : st.subscribed)
            if (!target.count(s)) to_remove.push_back(s);

        // -- Admit new symbols: open sinks then SUBSCRIBE ------------------
        for (const auto& sym : to_add) {
            per_symbol_sinks psink;
            psink.agg_trade = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "aggTrade"));
            psink.book_ticker = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "bookTicker"));
            st.sinks.emplace(sym, std::move(psink));

            auto r = co_await dyn.async_subscribe(
                types::aggregate_trade_subscription{ .symbol = sym },
                types::book_ticker_subscription{ .symbol = sym });
            if (!r) {
                spdlog::warn("detail[{}]: subscribe failed: {}", sym, r.err.message);
                st.sinks.erase(sym);
                continue;
            }
            st.subscribed.insert(sym);
            spdlog::info("detail[{}]: subscribed aggTrade+bookTicker", sym);
        }

        // -- Evict symbols: UNSUBSCRIBE then drop sinks --------------------
        for (const auto& sym : to_remove) {
            auto r = co_await dyn.async_unsubscribe(
                types::aggregate_trade_subscription{ .symbol = sym },
                types::book_ticker_subscription{ .symbol = sym });
            if (!r)
                spdlog::warn("detail[{}]: unsubscribe failed: {}",
                             sym, r.err.message);
            // Drop sinks — destructors flush + spawn final zstd.
            st.sinks.erase(sym);
            st.subscribed.erase(sym);
            spdlog::info("detail[{}]: unsubscribed, sinks closed", sym);
        }
    }

    spdlog::info("detail: subscription manager exiting");
    st.closed = true;
    st.record_buf.close();
}

} // namespace

boost::cobalt::task<void>
detail_monitor_run(const recorder_config& cfg,
                   selector& sel,
                   status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    fapi::config net_cfg =
        cfg.testnet ? fapi::config::testnet_config() : fapi::config{};

    streams::dynamic_market_stream dyn(net_cfg);
    detail_state st(16384);

    // Attach the single shared buffer: every incoming frame is pushed
    // by async_read_text in the connection's read path.
    dyn.connection().attach_buffer(st.record_buf);

    if (auto conn = co_await dyn.async_connect(); !conn) {
        spdlog::error("detail: connect failed: {}", conn.err.message);
        co_return;
    }
    spdlog::info("detail: connected to /stream endpoint");

    // Spawn the connection's own read loop as a future so the drain
    // buffer gets fed. async_read_text side-bands every frame into
    // record_buf via attach_buffer.
    auto f_read = boost::cobalt::spawn(
        exec, connection_read_loop(dyn), boost::asio::use_future);

    status.add_source("detail", [&st, &sel]() {
        std::string s = "subs=" + std::to_string(st.subscribed.size());
        s += "/" + std::to_string(sel.cfg().max_active);
        s += " buf=[push=" + std::to_string(st.record_buf.pushed_total());
        s += " drn=" + std::to_string(st.record_buf.drained_total());
        s += " occ=" + std::to_string(st.record_buf.occupancy());
        s += " hw=" + std::to_string(st.record_buf.high_water()) + "]";
        return s;
    });

    // Join drain + manage loops. 2-task cobalt::join is stable.
    co_await boost::cobalt::join(
        drain_loop(st),
        manage_subs_loop(cfg, sel, dyn, st, ioc));

    // Tear down the WS connection so the read_loop future completes.
    (void)co_await dyn.async_close();
    if (f_read.valid()) {
        try { f_read.get(); } catch (...) {}
    }
    spdlog::info("detail: done");
}

} // namespace binapi2::examples::async_recorder
