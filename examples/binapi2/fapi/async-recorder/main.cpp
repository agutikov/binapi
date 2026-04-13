// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — institutional-grade recorder example.
//
// Step 1 scaffolding: parses CLI args + YAML, installs a SIGINT/SIGTERM
// handler on the cobalt::main executor, logs startup, and exits cleanly.
// Subsequent steps will add the screener, selector, detail monitor, REST
// sync, and stats runner. See docs/binapi2/plans/async_recorder.md.

#include "config.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <spdlog/spdlog.h>

#include <csignal>
#include <filesystem>

namespace ar = binapi2::examples::async_recorder;

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

    spdlog::set_level(spdlog::level::info);
    spdlog::info("async-recorder starting");
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

    // Step 1 stub: nothing to run yet. Future steps will launch
    // screener_run / selector_run / detail_run / rest_sync_run / stats_run
    // here and race them against the signal_set.
    spdlog::info("scaffolding only — exiting. Pass --print-config to dump config.");

    signals.cancel();
    co_return 0;
}
