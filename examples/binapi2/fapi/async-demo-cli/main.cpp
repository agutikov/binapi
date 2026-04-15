// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-async-demo-cli — async demonstration client and usage example for
// the binapi2 fapi library.  Each source file in this directory shows a different
// aspect of the library (REST, WebSocket API, streams, local order book).
//
// Structure:
//   * main.cpp builds a single CLI::App and wires in the global options.
//   * Each cmd_*.cpp exposes a single `register_cmd_*` function that attaches
//     its subcommands (with options + callbacks) to the top-level app.
//   * Subcommand callbacks stash a `selected_cmd::factory` coroutine that this
//     file awaits after parsing is complete.

#include "common.hpp"
#include "cmd_market_data.hpp"
#include "cmd_account.hpp"
#include "cmd_trade.hpp"
#include "cmd_convert.hpp"
#include "cmd_ws_api.hpp"
#include "cmd_stream.hpp"
#include "cmd_user_stream.hpp"
#include "cmd_order_book.hpp"
#include "cmd_pipeline_order_book.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/streams/detail/sinks/spdlog_sink.hpp>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/this_coro.hpp>
#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>

#include <future>
#include <memory>

boost::cobalt::main co_main(int argc, char* argv[])
{
    CLI::App app{ "binapi2-fapi async demonstration client" };
    app.set_help_flag("-h,--help", "Print this help");
    app.require_subcommand(1);

    // ── global options ────────────────────────────────────────────────

    int verbose_count = 0;
    bool live = false;
    bool testnet = false;
    app.add_flag("-v", verbose_count,
                 "Verbosity: -v json, -vv json+transport, -vvv json+headers");
    app.add_flag("-l,--live,--prod", live,    "Use production endpoints");
    app.add_flag("--testnet",        testnet, "Use testnet endpoints (default)");
    app.add_option("-S,--save-request",    demo::save_request_file,  "Save request to file");
    app.add_option("-R,--save-response",   demo::save_response_file, "Save response body to file");
    app.add_option("-r,--record",          demo::record_file,        "Record raw stream frames to JSONL file");
    app.add_option("-K,--secrets",         demo::secrets_source,
                   "Secret source: libsecret:<profile> (default), env, systemd-creds:<dir>");
    app.add_option("-L,--log-file",        demo::log_file,           "Log to file");
    app.add_option("-F,--file-loglevel",   demo::file_loglevel,
                   "File log level (trace/debug/info/warn/error/off)");
    app.add_option("-O,--stdout-loglevel", demo::stdout_loglevel,
                   "Stdout log level (trace/debug/info/warn/error/off)");

    // ── subcommand registration (one call per cmd_*.hpp) ─────────────

    demo::selected_cmd sel;
    demo::register_cmd_market_data       (app, sel);
    demo::register_cmd_account           (app, sel);
    demo::register_cmd_trade             (app, sel);
    demo::register_cmd_convert           (app, sel);
    demo::register_cmd_ws_api            (app, sel);
    demo::register_cmd_stream            (app, sel);
    demo::register_cmd_user_stream       (app, sel);
    demo::register_cmd_order_book        (app, sel);
    demo::register_cmd_pipeline_order_book(app, sel);

    // ── parse ─────────────────────────────────────────────────────────

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        co_return app.exit(e);
    }

    demo::verbosity = verbose_count;
    if (live) demo::use_testnet = false;
    if (testnet) demo::use_testnet = true;

    if (!sel.factory) {
        // require_subcommand(1) should prevent this, but guard anyway.
        spdlog::error("no subcommand selected");
        co_return 1;
    }

    demo::init_logging();

    // Set up stream recording if --record was specified.
    //
    // Uses the single-executor async recorder: the WebSocket stream runs on
    // this coroutine's executor, so push/drain are colocated and cheap.
    std::unique_ptr<binapi2::fapi::streams::async_spdlog_stream_recorder> recorder;
    std::future<void> recorder_future;
    if (auto rec_logger = spdlog::get("rec")) {
        recorder = std::make_unique<binapi2::fapi::streams::async_spdlog_stream_recorder>(4096);
        demo::record_buffer = &recorder->add_stream(
            binapi2::fapi::streams::sinks::spdlog_sink(rec_logger));
        auto rec_exec = co_await boost::cobalt::this_coro::executor;
        recorder_future = boost::cobalt::spawn(
            rec_exec, recorder->run(), boost::asio::use_future);
        spdlog::info("recording stream frames to {}", demo::record_file);
    }

    // Create config and load credentials.
    auto cfg = demo::make_config();
    co_await demo::async_load_secrets(cfg);
    binapi2::futures_usdm_api c(std::move(cfg));

    // Install SIGINT/SIGTERM handler for graceful shutdown.
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code ec, int sig) {
        if (ec) return; // cancelled
        spdlog::info("signal {} received, shutting down...", sig);
        ioc.stop();
    });

    int rc = co_await sel.factory(c);

    signals.cancel();
    if (recorder) {
        recorder->close();
        if (recorder_future.valid()) recorder_future.get();
    }
    demo::shutdown_logging();
    co_return rc;
}
