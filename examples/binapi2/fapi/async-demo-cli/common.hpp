// SPDX-License-Identifier: Apache-2.0
//
// binapi2-fapi-async-demo-cli: async demonstration client for the binapi2 fapi library.

#pragma once

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace binapi2::fapi { class client; }
namespace binapi2::fapi::detail { template<typename T> class threadsafe_stream_buffer; }

namespace demo {

using args_t = std::vector<std::string>;

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
inline ::binapi2::fapi::detail::threadsafe_stream_buffer<std::string>* record_buffer = nullptr;

// Secret provider: "libsecret" (default), "env" (deprecated), "systemd-creds:<dir>"
inline std::string secrets_source;

// File logging.
inline std::string log_file;
inline std::string file_loglevel;
inline std::string stdout_loglevel;

// Command function pointer type: async, takes client& and args.
using command_fn = boost::cobalt::task<int> (*)(binapi2::futures_usdm_api&, const args_t&);

// Initialize async spdlog (call once from main).
void init_logging();

// Flush and shutdown all async loggers (call before exit).
void shutdown_logging();

// Build config (credentials loaded separately via async_load_secrets).
binapi2::fapi::config make_config();

// Async stdout output — non-blocking, enqueues to spdlog "out" logger.
template<typename... Args>
void out(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    if (auto logger = spdlog::get("out"))
        logger->info(fmt, std::forward<Args>(args)...);
}

// Print error details via spdlog.
void print_error(const binapi2::fapi::error& err);

// Print a value as pretty JSON via async output.
template<typename T>
void print_json(const T& value)
{
    auto json = glz::write<glz::opts{ .prettify = true }>(value);
    if (json) {
        out("{}", *json);
    }
}

// Log a value as JSON via spdlog debug.
template<typename T>
void log_json(const T& value)
{
    auto json = glz::write_json(value);
    if (json) {
        spdlog::debug("json: {}", *json);
    }
}

// Check result, print error or JSON, return exit code.
template<typename T>
int handle_result(const binapi2::fapi::result<T>& r)
{
    if (!r) {
        print_error(r.err);
        return 1;
    }
    if (verbosity >= 1) {
        print_json(*r);
    }
    return 0;
}

// Parse an enum from a string using glaze metadata (e.g. "BUY" -> order_side_t::buy).
// Enums whose glz::meta uses uppercase keys (BUY, LIMIT, GTC) accept input case-insensitively.
// Kline intervals (1m, 1h, 1M) are parsed as-is.
template<typename E>
E parse_enum(std::string_view s)
{
    // glz::read_json expects a quoted JSON string: "BUY"
    std::string quoted = "\"" + std::string(s) + "\"";
    E value{};
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    // Retry with uppercase (handles case-insensitive input for BUY/SELL/LIMIT/etc.)
    std::string upper(s);
    std::ranges::transform(upper, upper.begin(), ::toupper);
    quoted = "\"" + upper + "\"";
    if (!glz::read_json(value, quoted)) {
        return value;
    }

    throw std::invalid_argument("unknown " + std::string(typeid(E).name()) + ": " + std::string(s));
}

// Find a --key (or -k short) value pair in args, return value or empty.
std::string_view find_flag(const args_t& args, std::string_view key, std::string_view short_key);

// Load API credentials from the configured secret source into cfg.
// Uses libsecret by default, falls back to env vars if --secrets env.
boost::cobalt::task<void> async_load_secrets(binapi2::fapi::config& cfg);

// ---------------------------------------------------------------------------
// REST service executors — one per service group
// ---------------------------------------------------------------------------

template<typename Request>
boost::cobalt::task<int> exec_market_data(binapi2::futures_usdm_api& c, Request req)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->market_data.async_execute(req);
    co_return handle_result(r);
}

template<typename Request>
boost::cobalt::task<int> exec_account(binapi2::futures_usdm_api& c, Request req)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->account.async_execute(req);
    co_return handle_result(r);
}

template<typename Request>
boost::cobalt::task<int> exec_trade(binapi2::futures_usdm_api& c, Request req)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

// Note: convert_service has no generic async_execute (traits-dispatched via pipeline).
// Each convert command calls the pipeline directly in cmd_convert.cpp.

template<typename Request>
boost::cobalt::task<int> exec_user_data_streams(binapi2::futures_usdm_api& c, Request req)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->user_data_streams.async_execute(req);
    co_return handle_result(r);
}

// ---------------------------------------------------------------------------
// WebSocket API executors
// ---------------------------------------------------------------------------

/// Public WS API call: connect → execute → print → close.
template<typename Request>
boost::cobalt::task<int> exec_ws_public(binapi2::futures_usdm_api& c, Request req)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    auto r = co_await (*ws)->async_execute(req);
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }
    spdlog::info("status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);
    co_await (*ws)->async_close();
    co_return 0;
}

/// Signed WS API call: connect → logon → execute → print → close.
template<typename Request>
boost::cobalt::task<int> exec_ws_signed(binapi2::futures_usdm_api& c, Request req)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    if (auto l = co_await (*ws)->async_session_logon(); !l) {
        print_error(l.err);
        co_await (*ws)->async_close();
        co_return 1;
    }
    auto r = co_await (*ws)->async_execute(req);
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }
    spdlog::info("status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);
    co_await (*ws)->async_close();
    co_return 0;
}

// ---------------------------------------------------------------------------
// Market stream executor
// ---------------------------------------------------------------------------

/// Subscribe to a market stream and print events in a loop.
template<typename Subscription>
boost::cobalt::task<int> exec_stream(binapi2::futures_usdm_api& c, Subscription sub)
{
    auto streams = c.create_market_stream();
    if (record_buffer) streams->connection().attach_buffer(*record_buffer);
    auto gen = streams->subscribe(sub);
    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        print_json(*event);
    }
    co_return 0;
}

// ---------------------------------------------------------------------------
// Argument-shape helpers for command families
// ---------------------------------------------------------------------------

/// Kline command: parse <symbol> <interval> [limit], execute via market_data.
template<typename Request>
boost::cobalt::task<int> cmd_kline(binapi2::futures_usdm_api& c, const args_t& args,
                                   std::string_view usage)
{
    if (args.size() < 2) { spdlog::error("usage: {}", usage); co_return 1; }
    Request req;
    req.symbol = args[0];
    req.interval = parse_enum<binapi2::fapi::types::kline_interval_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);
    co_return co_await exec_market_data(c, req);
}

/// Pair-kline command: parse <pair> <interval> [limit], execute via market_data.
template<typename Request>
boost::cobalt::task<int> cmd_pair_kline(binapi2::futures_usdm_api& c, const args_t& args,
                                        std::string_view usage)
{
    if (args.size() < 2) { spdlog::error("usage: {}", usage); co_return 1; }
    Request req;
    req.pair = args[0];
    req.interval = parse_enum<binapi2::fapi::types::kline_interval_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);
    co_return co_await exec_market_data(c, req);
}

/// Futures analytics command: parse <symbol> <period> [limit], execute via market_data.
template<typename Request>
boost::cobalt::task<int> cmd_futures_analytics(binapi2::futures_usdm_api& c, const args_t& args,
                                               std::string_view usage)
{
    if (args.size() < 2) { spdlog::error("usage: {}", usage); co_return 1; }
    Request req;
    req.symbol = args[0];
    req.period = parse_enum<binapi2::fapi::types::kline_interval_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);
    co_return co_await exec_market_data(c, req);
}

/// Download-id command: parse <startTime> <endTime> as epoch-ms, execute via account.
template<typename Request>
boost::cobalt::task<int> cmd_download_id(binapi2::futures_usdm_api& c, const args_t& args,
                                         std::string_view usage)
{
    if (args.size() < 2) { spdlog::error("usage: {}", usage); co_return 1; }
    Request req;
    req.startTime = binapi2::fapi::types::timestamp_ms_t{ std::stoull(args[0]) };
    req.endTime = binapi2::fapi::types::timestamp_ms_t{ std::stoull(args[1]) };
    co_return co_await exec_account(c, req);
}

/// Download-link command: parse <downloadId>, execute via account.
template<typename Request>
boost::cobalt::task<int> cmd_download_link(binapi2::futures_usdm_api& c, const args_t& args,
                                           std::string_view usage)
{
    if (args.empty()) { spdlog::error("usage: {}", usage); co_return 1; }
    Request req;
    req.downloadId = args[0];
    co_return co_await exec_account(c, req);
}

} // namespace demo
