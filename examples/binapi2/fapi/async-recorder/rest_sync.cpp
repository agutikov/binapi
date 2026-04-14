// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 4: REST periodic sync.

#include "rest_sync.hpp"

#include "selector.hpp"
#include "status_reporter.hpp"

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/client.hpp>
#include <binapi2/fapi/rest/services/market_data.hpp>
#include <binapi2/fapi/types/enums.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <glaze/glaze.hpp>

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fapi = binapi2::fapi;
namespace types = binapi2::fapi::types;

namespace binapi2::examples::async_recorder {

namespace {

std::string now_iso()
{
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count() %
                    1000;
    std::tm tm{};
    ::gmtime_r(&t, &tm);
    char buf[80];
    std::snprintf(buf, sizeof(buf),
                  "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long>(ms));
    return buf;
}

/// @brief Append one wrapped line `{"t":"<iso>","d":<response>}` to the
///        per-endpoint JSONL file. The file is opened per call (tiny
///        traffic — one line per tick per symbol — so no point
///        keeping handles around).
template<class Response>
void append_response(const std::filesystem::path& dir,
                     const std::string& file,
                     const Response& response)
{
    std::filesystem::create_directories(dir);
    const auto path = dir / file;

    auto json = glz::write_json(response);
    if (!json) {
        spdlog::warn("rest_sync: failed to serialise response for {}",
                     path.string());
        return;
    }

    std::ofstream out(path, std::ios::app);
    if (!out) {
        spdlog::warn("rest_sync: cannot open {} for append", path.string());
        return;
    }
    out << "{\"t\":\"" << now_iso() << "\",\"d\":" << *json << "}\n";
}

/// @brief Aggregated counters exposed to the status_reporter.
struct rest_stats
{
    std::size_t funding_ok{ 0 };
    std::size_t funding_err{ 0 };
    std::size_t klines_ok{ 0 };
    std::size_t klines_err{ 0 };
    std::size_t oi_ok{ 0 };
    std::size_t oi_err{ 0 };
    std::size_t lsr_ok{ 0 };
    std::size_t lsr_err{ 0 };

    [[nodiscard]] std::string format() const
    {
        return "fund=" + std::to_string(funding_ok) + "/" + std::to_string(funding_err) +
               " kln=" + std::to_string(klines_ok) + "/" + std::to_string(klines_err) +
               " oi=" + std::to_string(oi_ok) + "/" + std::to_string(oi_err) +
               " lsr=" + std::to_string(lsr_ok) + "/" + std::to_string(lsr_err);
    }
};

// -- Per-endpoint fetchers --------------------------------------------------

boost::cobalt::task<void>
fetch_funding_rate(fapi::rest::client& client,
                   const std::filesystem::path& root,
                   rest_stats& stats)
{
    types::funding_rate_history_request_t req;  // no symbol => all symbols
    req.limit = 1000;
    auto r = co_await client.market_data.async_execute(req);
    if (!r) {
        ++stats.funding_err;
        spdlog::warn("rest_sync/funding: {}", r.err.message);
        co_return;
    }
    append_response(root / "rest" / "fundingRate", "fundingRate.jsonl", *r);
    ++stats.funding_ok;
    spdlog::info("rest_sync/funding: {} entries", r->size());
}

boost::cobalt::task<void>
fetch_klines_for(fapi::rest::client& client,
                 const std::filesystem::path& root,
                 const std::string& symbol,
                 rest_stats& stats)
{
    types::klines_request_t req;
    req.symbol = symbol;
    req.interval = types::kline_interval_t::m1;
    req.limit = 2;  // just the last closed + current bar
    auto r = co_await client.market_data.async_execute(req);
    if (!r) {
        ++stats.klines_err;
        co_return;
    }
    append_response(root / "rest" / "klines_1m", symbol + ".jsonl", *r);
    ++stats.klines_ok;
}

boost::cobalt::task<void>
fetch_open_interest_hist_for(fapi::rest::client& client,
                             const std::filesystem::path& root,
                             const std::string& symbol,
                             rest_stats& stats)
{
    types::open_interest_statistics_request_t req;
    req.symbol = symbol;
    req.period = types::kline_interval_t::m5;
    req.limit = 1;
    auto r = co_await client.market_data.async_execute(req);
    if (!r) {
        ++stats.oi_err;
        co_return;
    }
    append_response(root / "rest" / "openInterestHist", symbol + ".jsonl", *r);
    ++stats.oi_ok;
}

boost::cobalt::task<void>
fetch_long_short_ratio_for(fapi::rest::client& client,
                           const std::filesystem::path& root,
                           const std::string& symbol,
                           rest_stats& stats)
{
    types::long_short_ratio_request_t req;
    req.symbol = symbol;
    req.period = types::kline_interval_t::m15;
    req.limit = 1;
    auto r = co_await client.market_data.async_execute(req);
    if (!r) {
        ++stats.lsr_err;
        co_return;
    }
    append_response(root / "rest" / "longShortRatio", symbol + ".jsonl", *r);
    ++stats.lsr_ok;
}

} // namespace

boost::cobalt::task<void>
rest_sync_run(const recorder_config& cfg,
              selector& sel,
              status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    // Bring up our own REST client. A fresh HTTP connection per stage
    // keeps lifetimes independent of the streams stages.
    fapi::config net_cfg =
        cfg.testnet ? fapi::config::testnet_config() : fapi::config{};
    binapi2::futures_usdm_api api(std::move(net_cfg));

    auto rc = co_await api.create_rest_client();
    if (!rc) {
        spdlog::error("rest_sync: connect failed: {}", rc.err.message);
        co_return;
    }
    auto& client = **rc;
    spdlog::info("rest_sync: REST client connected");

    rest_stats stats;
    status.add_source("rest", [&stats, &sel]() {
        std::string s = stats.format();
        s += " active=" + std::to_string(sel.active().size());
        return s;
    });

    // Schedule: each endpoint has a deadline. The master timer ticks
    // every few seconds and fires whichever endpoints are due. The
    // initial deadline is `now` so every endpoint fires once on
    // startup, which is useful for short smoke runs.
    using clock = std::chrono::steady_clock;
    auto now = clock::now();
    auto next_funding = now;
    auto next_klines  = now;
    auto next_oi      = now;
    auto next_lsr     = now;

    const auto per_funding = std::chrono::hours(1);
    const auto per_klines  = std::chrono::minutes(1);
    const auto per_oi      = std::chrono::minutes(5);
    const auto per_lsr     = std::chrono::minutes(15);

    const auto tick = std::chrono::seconds(
        std::max<std::uint64_t>(1, cfg.stats_interval_seconds));

    boost::asio::steady_timer timer(ioc);

    while (true) {
        timer.expires_after(tick);
        try {
            co_await timer.async_wait(boost::cobalt::use_op);
        } catch (const std::exception&) {
            spdlog::info("rest_sync: shutdown");
            break;
        }

        now = clock::now();

        if (now >= next_funding) {
            co_await fetch_funding_rate(client, cfg.root_dir, stats);
            next_funding = now + per_funding;
        }

        // Snapshot the active set ONCE per tick so we don't walk a
        // mutating container and so the per-endpoint loops see the
        // same set.
        std::vector<std::string> active_snapshot(sel.active().begin(),
                                                 sel.active().end());

        if (now >= next_klines) {
            for (const auto& s : active_snapshot)
                co_await fetch_klines_for(client, cfg.root_dir, s, stats);
            next_klines = now + per_klines;
        }

        if (now >= next_oi) {
            for (const auto& s : active_snapshot)
                co_await fetch_open_interest_hist_for(client, cfg.root_dir, s, stats);
            next_oi = now + per_oi;
        }

        if (now >= next_lsr) {
            for (const auto& s : active_snapshot)
                co_await fetch_long_short_ratio_for(client, cfg.root_dir, s, stats);
            next_lsr = now + per_lsr;
        }
    }
}

} // namespace binapi2::examples::async_recorder
