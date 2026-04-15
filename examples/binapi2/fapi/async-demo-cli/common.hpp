// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-async-demo-cli: async demonstration client for the
// binapi2 fapi library.
//
// The bulk of the command logic (typed executors, option structs, request
// builders, enum parsing) lives in the shared `binapi2_demo_commands`
// library and is consumed by both this CLI and `async-demo-ui`.  This
// header is the CLI-only layer: global state for CLI flags, spdlog
// plumbing, a CLI-local stream helper that attaches the recording
// buffer, and CLI11-shaped `add_*_sub` registration templates used by
// the individual `cmd_*.cpp` files.

#pragma once

#include <binapi2/demo_commands/builders.hpp>
#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/demo_commands/opts.hpp>
#include <binapi2/demo_commands/result_sink.hpp>
#include <binapi2/demo_commands/spdlog_sink.hpp>

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/error.hpp>

#include <boost/cobalt/task.hpp>

#include <CLI/CLI.hpp>
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>

namespace binapi2::fapi::detail { template<typename T> class stream_buffer; }

namespace demo {

// ---------------------------------------------------------------------------
// Global state (CLI-only flags)
// ---------------------------------------------------------------------------

// Verbosity: 0 = summary only, 1 = print JSON, 2 = print JSON + HTTP details.
inline int verbosity = 0;
inline bool use_testnet = true;

// Save request/response bodies to files.
inline std::string save_request_file;
inline std::string save_response_file;

// Record raw WebSocket stream frames to a JSONL file.
inline std::string record_file;

// Recording buffer — set by main if --record is used.
// Stream commands attach this to their connection if non-null.
inline ::binapi2::fapi::detail::stream_buffer<std::string>* record_buffer = nullptr;

// Secret provider: "libsecret" (default), "env" (deprecated), "systemd-creds:<dir>"
inline std::string secrets_source;

// File logging.
inline std::string log_file;
inline std::string file_loglevel;
inline std::string stdout_loglevel;

// ---------------------------------------------------------------------------
// Selected-command machinery
// ---------------------------------------------------------------------------
//
// CLI11 parses the command line once at the top level; each subcommand's
// callback stashes a `factory` here — a coroutine that runs the command body
// once we have a connected futures_usdm_api client.  `co_main` then awaits
// it, passing in the spdlog-backed `result_sink` it built from the parsed
// `-v`/`-vv`/`-vvv` flags.

using cmd_fn = std::function<boost::cobalt::task<int>(
    binapi2::futures_usdm_api&, binapi2::demo::result_sink&)>;

struct selected_cmd
{
    cmd_fn factory;
};

// ---------------------------------------------------------------------------
// CLI-only services
// ---------------------------------------------------------------------------

/// Initialise async spdlog loggers (the default console logger, the
/// prefix-free `"out"` logger used for JSON bodies, and the `"rec"` logger
/// used for frame recording when `--record` is set). Call once from main.
void init_logging();

/// Flush and shutdown all async loggers (call before exit).
void shutdown_logging();

/// Build a `binapi2::fapi::config` honouring `use_testnet`, the
/// save-request/save-response file paths, and the transport logging hooks.
/// Credentials are loaded separately via `async_load_secrets`.
binapi2::fapi::config make_config();

/// Load API credentials from the configured secret source into `cfg`.
/// Uses libsecret:demo by default, falls back to env vars if `--secrets env`.
boost::cobalt::task<void> async_load_secrets(binapi2::fapi::config& cfg);

/// Non-blocking stdout output for custom-body commands — enqueues into the
/// prefix-free `"out"` async spdlog logger registered by `init_logging()`.
/// Used by commands like `order-book` / `balances` that print multi-line
/// summaries the generic sink path doesn't cover.
template<typename... Args>
void out(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    if (auto logger = spdlog::get("out"))
        logger->info(fmt, std::forward<Args>(args)...);
}

/// Pretty-print a value as JSON into the `"out"` logger. Kept for the few
/// custom-body commands that need to inline a JSON body without going
/// through the `result_sink`.
template<typename T>
void print_json(const T& value)
{
    auto json = glz::write<glz::opts{ .prettify = true }>(value);
    if (json) {
        out("{}", *json);
    }
}

/// CLI-local stream executor: wraps the library's `run_stream` with the
/// record-buffer attach step, which is driven by the CLI-only
/// `record_buffer` global. Used by the `cmd_stream.cpp` helpers instead
/// of `binapi2::demo::exec_stream`.
template<typename Subscription>
boost::cobalt::task<int>
exec_stream_with_recorder(binapi2::futures_usdm_api& c,
                          Subscription sub,
                          binapi2::demo::result_sink& sink)
{
    auto streams = c.create_market_stream();
    if (record_buffer) streams->connection().attach_buffer(*record_buffer);
    co_return co_await binapi2::demo::run_stream(streams->subscribe(sub), sink);
}

// ---------------------------------------------------------------------------
// CLI11 subcommand registration helpers
// ---------------------------------------------------------------------------
//
// Each helper attaches a subcommand with options and a callback to the
// parent `CLI::App`, binding its options to a shared opts struct from
// `binapi2_demo_commands`. The callback stashes a factory in `selected_cmd`
// that calls the shared request builder + executor.

/// `<symbol> <interval> [limit]` — klines family. The limit is an
/// optional positional (matching the demo scripts' usage:
/// `klines BTCUSDT 1h 5`).
template<typename Request>
inline CLI::App*
add_kline_sub(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<binapi2::demo::kline_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol",   opts->symbol,   "Trading symbol")->required();
    sub->add_option("interval", opts->interval, "Kline interval (1m,5m,1h,1d,…)")->required();
    sub->add_option("limit",    opts->limit,    "Number of bars (optional)");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             binapi2::demo::result_sink& sink)
            -> boost::cobalt::task<int> {
            co_return co_await binapi2::demo::exec_market_data(
                c, binapi2::demo::make_kline_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// `<pair> <interval> [limit]` — pair kline family.
template<typename Request>
inline CLI::App*
add_pair_kline_sub(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<binapi2::demo::pair_kline_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("pair",     opts->pair,     "Pair (e.g. BTCUSDT)")->required();
    sub->add_option("interval", opts->interval, "Kline interval")->required();
    sub->add_option("limit",    opts->limit,    "Number of bars (optional)");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             binapi2::demo::result_sink& sink)
            -> boost::cobalt::task<int> {
            co_return co_await binapi2::demo::exec_market_data(
                c, binapi2::demo::make_pair_kline_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// `<symbol> <period> [limit]` — futures analytics family.
template<typename Request>
inline CLI::App*
add_analytics_sub(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<binapi2::demo::analytics_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->add_option("period", opts->period, "Period (5m,15m,30m,1h,…)")->required();
    sub->add_option("limit",  opts->limit,  "Number of bars (optional)");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             binapi2::demo::result_sink& sink)
            -> boost::cobalt::task<int> {
            co_return co_await binapi2::demo::exec_market_data(
                c, binapi2::demo::make_analytics_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// `<start> <end>` download-id request (epoch-ms range), via account.
template<typename Request>
inline CLI::App*
add_download_id_sub(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<binapi2::demo::download_id_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("start", opts->start_time, "Start time (epoch-ms)")->required();
    sub->add_option("end",   opts->end_time,   "End time (epoch-ms)")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             binapi2::demo::result_sink& sink)
            -> boost::cobalt::task<int> {
            co_return co_await binapi2::demo::exec_account(
                c, binapi2::demo::make_download_id_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// `<downloadId>` download-link request, via account.
template<typename Request>
inline CLI::App*
add_download_link_sub(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<binapi2::demo::download_link_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("downloadId", opts->download_id,
                    "Download ID from a previous download-id call")
        ->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             binapi2::demo::result_sink& sink)
            -> boost::cobalt::task<int> {
            co_return co_await binapi2::demo::exec_account(
                c, binapi2::demo::make_download_link_request<Request>(*opts), sink);
        };
    });
    return sub;
}

} // namespace demo
