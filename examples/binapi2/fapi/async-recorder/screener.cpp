// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 1: screener.
//
// Debug-only shape for now: runs a SINGLE all-market stream end-to-end to
// exercise the market_stream -> stream_buffer -> rotating_file_sink path.
// Once this is stable, the multi-stream screener from §2 of the plan will
// be layered back on top. Models itself on
// examples/binapi2/fapi/async-demo-cli/cmd_stream.cpp (exec_stream).

#include "screener.hpp"
#include "status_reporter.hpp"

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp>
#include <binapi2/fapi/streams/market_stream.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <chrono>
#include <future>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace fapi = binapi2::fapi;
namespace streams = binapi2::fapi::streams;
namespace types = binapi2::fapi::types;

namespace binapi2::examples::async_recorder {

namespace {

/// @brief Frame counters exposed to the status_reporter. All writes and
///        reads happen on the same executor, so plain size_t is safe.
struct screener_state
{
    std::size_t bookTicker_frames{ 0 };
    std::size_t markPriceArr_frames{ 0 };
    std::size_t tickerArr_frames{ 0 };

    [[nodiscard]] std::string format() const
    {
        return "bookTicker=" + std::to_string(bookTicker_frames) +
               " markPriceArr=" + std::to_string(markPriceArr_frames) +
               " tickerArr=" + std::to_string(tickerArr_frames);
    }
};

streams::sinks::rotating_file_sink_config
make_rfs_cfg(const recorder_config& cfg, const std::string& subdir)
{
    std::filesystem::path dir = cfg.root_dir / "screener" / subdir;
    std::filesystem::create_directories(dir);

    streams::sinks::rotating_file_sink_config rfs;
    rfs.dir = std::move(dir);
    rfs.basename = subdir;
    rfs.extension = ".jsonl";
    rfs.max_size_bytes = cfg.rotation_size_bytes;
    rfs.max_seconds = cfg.rotation_seconds;
    rfs.compress = true;
    return rfs;
}

/// @brief Drive a market_stream generator for one fixed subscription,
///        pushing raw frames into the recorder's buffer via attach_buffer
///        and tallying frame counts into `counter` so the status_reporter
///        can read them. Closes the recorder on exit so the drain
///        coroutine can finish (used by the single-stream debug path).
template<class Subscription>
boost::cobalt::task<void>
run_one(streams::market_stream& ms,
        fapi::detail::stream_buffer<std::string>& rec_buf,
        streams::async_rotating_file_stream_recorder& recorder,
        Subscription sub,
        const char* label,
        std::size_t& counter)
{
    ms.connection().attach_buffer(rec_buf);
    spdlog::info("screener[{}]: subscribing", label);

    auto gen = ms.subscribe(sub);
    while (gen) {
        auto ev = co_await gen;
        if (!ev) {
            spdlog::warn("screener[{}]: stream ended: {}", label, ev.err.message);
            break;
        }
        ++counter;
    }

    spdlog::info("screener[{}]: {} frames total, closing recorder", label, counter);
    recorder.close();
}

/// @brief Drive one market_stream generator without touching the recorder
///        lifecycle — used by the real multi-stream screener, where close
///        is handled externally once every read loop has exited.
template<class Subscription>
boost::cobalt::task<void>
run_feed(streams::market_stream& ms,
         fapi::detail::stream_buffer<std::string>& rec_buf,
         Subscription sub,
         const char* label,
         std::size_t& counter)
{
    ms.connection().attach_buffer(rec_buf);
    spdlog::info("screener[{}]: subscribing", label);

    auto gen = ms.subscribe(sub);
    while (gen) {
        auto ev = co_await gen;
        if (!ev) {
            spdlog::warn("screener[{}]: stream ended: {}", label, ev.err.message);
            break;
        }
        ++counter;
    }
    spdlog::info("screener[{}]: {} frames total", label, counter);
}

/// @brief Poll up to three spawned read-loop futures; when all that were
///        actually started (valid()) have finished, close the recorder so
///        its drain can unwind.
boost::cobalt::task<void>
watch_then_close(std::future<void>& f1,
                 std::future<void>& f2,
                 std::future<void>& f3,
                 boost::asio::io_context& ioc,
                 streams::async_rotating_file_stream_recorder& recorder)
{
    boost::asio::steady_timer timer(ioc);
    while (true) {
        timer.expires_after(std::chrono::milliseconds(200));
        co_await timer.async_wait(boost::cobalt::use_op);
        auto ready = [](std::future<void>& f) {
            return !f.valid() ||
                   f.wait_for(std::chrono::seconds(0)) ==
                       std::future_status::ready;
        };
        if (ready(f1) && ready(f2) && ready(f3)) break;
    }
    spdlog::info("screener: all feeds done, closing recorder");
    recorder.close();
}

} // namespace

// ---------------------------------------------------------------------------
// Debug screener — single all-market feed, for isolating pipeline bugs.
// ---------------------------------------------------------------------------

boost::cobalt::task<int>
debug_screener_run(const recorder_config& cfg, status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    fapi::config net_cfg =
        cfg.testnet ? fapi::config::testnet_config() : fapi::config{};

    const std::string subdir =
        cfg.debug_stream.empty() ? std::string("bookTicker") : cfg.debug_stream;

    streams::async_rotating_file_stream_recorder recorder(16384);
    auto& buf = recorder.add_stream(streams::sinks::rotating_file_sink(
        ioc, make_rfs_cfg(cfg, subdir)));

    streams::market_stream ms(net_cfg);

    screener_state state;
    status.add_source("screener", [&state]() { return state.format(); });

    // Branching is on the subscription *type*, so each arm instantiates
    // its own run_one template and then co_awaits a 2-task cobalt::join.
    if (subdir == "markPriceArr") {
        co_await boost::cobalt::join(
            run_one(ms, buf, recorder,
                    types::all_market_mark_price_subscription{ .every_1s = true },
                    "markPriceArr", state.markPriceArr_frames),
            recorder.run());
    }
    else if (subdir == "tickerArr") {
        co_await boost::cobalt::join(
            run_one(ms, buf, recorder,
                    types::all_market_ticker_subscription{}, "tickerArr",
                    state.tickerArr_frames),
            recorder.run());
    }
    else /* bookTicker / default */ {
        co_await boost::cobalt::join(
            run_one(ms, buf, recorder,
                    types::all_book_ticker_subscription{}, "bookTicker",
                    state.bookTicker_frames),
            recorder.run());
    }

    spdlog::info("debug_screener: drain complete");
    co_return 0;
}

// ---------------------------------------------------------------------------
// Real screener — three all-market feeds recorded in parallel.
// ---------------------------------------------------------------------------

boost::cobalt::task<int>
screener_run(const recorder_config& cfg, status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    fapi::config net_cfg =
        cfg.testnet ? fapi::config::testnet_config() : fapi::config{};

    streams::async_rotating_file_stream_recorder recorder(16384);
    auto& buf_bt = recorder.add_stream(streams::sinks::rotating_file_sink(
        ioc, make_rfs_cfg(cfg, "bookTicker")));
    auto& buf_mp = recorder.add_stream(streams::sinks::rotating_file_sink(
        ioc, make_rfs_cfg(cfg, "markPriceArr")));
    auto& buf_tk = recorder.add_stream(streams::sinks::rotating_file_sink(
        ioc, make_rfs_cfg(cfg, "tickerArr")));

    streams::market_stream ms_bt(net_cfg);
    streams::market_stream ms_mp(net_cfg);
    streams::market_stream ms_tk(net_cfg);

    screener_state state;
    status.add_source("screener", [&state]() { return state.format(); });

    // Spawn each read loop as an independent future on the same executor,
    // then join the watcher + drain (2-task cobalt::join, which is stable).
    auto f_bt = boost::cobalt::spawn(
        exec,
        run_feed(ms_bt, buf_bt,
                 types::all_book_ticker_subscription{}, "bookTicker",
                 state.bookTicker_frames),
        boost::asio::use_future);
    auto f_mp = boost::cobalt::spawn(
        exec,
        run_feed(ms_mp, buf_mp,
                 types::all_market_mark_price_subscription{ .every_1s = true },
                 "markPriceArr", state.markPriceArr_frames),
        boost::asio::use_future);
    auto f_tk = boost::cobalt::spawn(
        exec,
        run_feed(ms_tk, buf_tk,
                 types::all_market_ticker_subscription{}, "tickerArr",
                 state.tickerArr_frames),
        boost::asio::use_future);

    co_await boost::cobalt::join(
        watch_then_close(f_bt, f_mp, f_tk, ioc, recorder),
        recorder.run());

    // Surface any exceptions from the read-loop futures.
    if (f_bt.valid()) {
        try { f_bt.get(); } catch (const std::exception& e) {
            spdlog::warn("screener[bookTicker] exception: {}", e.what());
        }
    }
    if (f_mp.valid()) {
        try { f_mp.get(); } catch (const std::exception& e) {
            spdlog::warn("screener[markPriceArr] exception: {}", e.what());
        }
    }
    if (f_tk.valid()) {
        try { f_tk.get(); } catch (const std::exception& e) {
            spdlog::warn("screener[tickerArr] exception: {}", e.what());
        }
    }

    spdlog::info("screener: drain complete");
    co_return 0;
}

} // namespace binapi2::examples::async_recorder
