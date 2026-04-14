// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — institutional-grade recorder example.
//
// Step 1 scaffolding: parses CLI args + YAML, installs a SIGINT/SIGTERM
// handler on the cobalt::main executor, logs startup, and exits cleanly.
// Subsequent steps will add the screener, selector, detail monitor, REST
// sync, and stats runner. See docs/binapi2/plans/async_recorder.md.

#include "config.hpp"
#include "screener.hpp"
#include "status_reporter.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <csignal>
#include <filesystem>
#include <memory>
#include <vector>

namespace ar = binapi2::examples::async_recorder;

namespace {

/// @brief Run the selected screener (real or debug) and, on return, close
///        the status_reporter so its `run()` can exit cleanly. Named
///        rather than a lambda because capturing coroutine lambdas crash
///        GCC 15 on this codebase.
boost::cobalt::task<int>
screener_and_close_status(const ar::recorder_config& cfg,
                          ar::status_reporter& status)
{
    int rc = 0;
    if (!cfg.debug_stream.empty()) {
        spdlog::info("mode: debug_screener (single stream: {})", cfg.debug_stream);
        rc = co_await ar::debug_screener_run(cfg, status);
    } else {
        spdlog::info("mode: screener (three all-market feeds)");
        rc = co_await ar::screener_run(cfg, status);
    }
    status.close();
    co_return rc;
}

} // namespace

boost::cobalt::main co_main(int argc, char* argv[])
{
    auto maybe_cfg = ar::parse_args(argc, argv);
    if (!maybe_cfg) co_return 0;  // --help / --print-config
    auto& cfg = *maybe_cfg;

    std::error_code ec;
    std::filesystem::create_directories(cfg.root_dir, ec);
    if (ec) {
        spdlog::error("cannot create root_dir {}: {}",
                      cfg.root_dir.string(), ec.message());
        co_return 1;
    }

    auto lvl = spdlog::level::from_str(cfg.loglevel);
    if (lvl == spdlog::level::off && cfg.loglevel != "off")
        lvl = spdlog::level::trace;  // fallback on typo

    // Build a logger with stdout + optional file sink.
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (!cfg.logfile.empty()) {
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            cfg.logfile.string(), /*truncate=*/true));
    }
    auto logger = std::make_shared<spdlog::logger>(
        "async-recorder", sinks.begin(), sinks.end());
    logger->set_level(lvl);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_default_logger(logger);
    spdlog::set_level(lvl);

    spdlog::info("async-recorder starting");
    if (!cfg.logfile.empty())
        spdlog::info("logging to file: {}", cfg.logfile.string());
    spdlog::info("root_dir: {}", cfg.root_dir.string());
    spdlog::info("network : {}", cfg.testnet ? "testnet" : "live");
    spdlog::info("depth   : {} (levels={}, with_depth={})",
                 cfg.depth_mode == ar::depth_mode_t::partial ? "partial" : "full",
                 cfg.depth_levels, cfg.with_depth);

    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code err, int sig) {
        if (err) return;
        spdlog::info("signal {} received, shutting down", sig);
        ioc.stop();
    });

    // App-wide periodic status logger. Each stage registers its own
    // state source; the reporter formats and emits one spdlog line per
    // interval (default 10 s, configurable via --stats-seconds).
    ar::status_reporter status(ioc, std::chrono::seconds(cfg.stats_interval_seconds));

    // Run the selected screener concurrently with the status reporter;
    // the helper closes the reporter after the screener returns so the
    // join below unwinds cleanly.
    co_await boost::cobalt::join(
        screener_and_close_status(cfg, status),
        status.run());

    signals.cancel();
    co_return 0;
}
