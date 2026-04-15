// SPDX-License-Identifier: Apache-2.0
//
// Sink-parameterised executors. These are the shared replacement for the
// `exec_*` templates that used to live in `async-demo-cli/common.hpp`.
//
// Each executor takes:
//
//   1. the `futures_usdm_api` facade to create a REST / WS / stream client,
//   2. a typed request / subscription value,
//   3. a `result_sink&` the executor emits through.
//
// The CLI passes a `spdlog_sink`; the UI passes a capture sink that fills
// a `request_capture` / `stream_capture` struct. Neither executor touches
// any global state — everything it needs is in its arguments.

#pragma once

#include "result_sink.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>

#include <atomic>
#include <string>

namespace binapi2::demo {

namespace detail {

/// Common success tail: pretty-print, push through `on_response_json`,
/// call `on_done(0)`, return 0.
template<typename T>
int emit_success(const T& value, result_sink& sink)
{
    auto json = glz::write<glz::opts{ .prettify = true }>(value);
    if (json) sink.on_response_json(*json);
    sink.on_done(0);
    return 0;
}

/// Common failure tail: emit the error, call `on_done(1)`, return 1.
inline int emit_failure(const binapi2::fapi::error& err, result_sink& sink)
{
    sink.on_error(err);
    sink.on_done(1);
    return 1;
}

} // namespace detail

// ---------------------------------------------------------------------------
// REST executors — one per service group on `futures_usdm_api`.
// ---------------------------------------------------------------------------

template<typename Request>
boost::cobalt::task<int>
exec_market_data(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) co_return detail::emit_failure(rest.err, sink);
    auto r = co_await (*rest)->market_data.async_execute(req);
    if (!r) co_return detail::emit_failure(r.err, sink);
    co_return detail::emit_success(*r, sink);
}

template<typename Request>
boost::cobalt::task<int>
exec_account(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) co_return detail::emit_failure(rest.err, sink);
    auto r = co_await (*rest)->account.async_execute(req);
    if (!r) co_return detail::emit_failure(r.err, sink);
    co_return detail::emit_success(*r, sink);
}

template<typename Request>
boost::cobalt::task<int>
exec_trade(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) co_return detail::emit_failure(rest.err, sink);
    auto r = co_await (*rest)->trade.async_execute(req);
    if (!r) co_return detail::emit_failure(r.err, sink);
    co_return detail::emit_success(*r, sink);
}

template<typename Request>
boost::cobalt::task<int>
exec_user_data_streams(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) co_return detail::emit_failure(rest.err, sink);
    auto r = co_await (*rest)->user_data_streams.async_execute(req);
    if (!r) co_return detail::emit_failure(r.err, sink);
    co_return detail::emit_success(*r, sink);
}

// Note: `convert_service` has no generic `async_execute` — each convert
// command dispatches via the rest pipeline directly, so we don't expose
// a shared `exec_convert`. The surface files (CLI's cmd_convert.cpp,
// UI's commands_convert.cpp) call `(*rest)->rest_pipeline().async_execute`
// inline and report the result via the sink themselves.

// ---------------------------------------------------------------------------
// WebSocket API executors.
// ---------------------------------------------------------------------------

/// Public WS API call: connect → execute → close.
///
/// Emits a `status=<N>` info line (preserving the CLI's original
/// behaviour) then the pretty-printed typed result if one is present.
template<typename Request>
boost::cobalt::task<int>
exec_ws_public(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) co_return detail::emit_failure(ws.err, sink);
    if (auto conn = co_await (*ws)->async_connect(); !conn) {
        co_await (*ws)->async_close();
        co_return detail::emit_failure(conn.err, sink);
    }
    auto r = co_await (*ws)->async_execute(req);
    if (!r) {
        co_await (*ws)->async_close();
        co_return detail::emit_failure(r.err, sink);
    }

    sink.on_info("status=" + std::to_string(r->status));
    int rc = 0;
    if (r->result) {
        rc = detail::emit_success(*r->result, sink);
    } else {
        sink.on_done(0);
    }
    co_await (*ws)->async_close();
    co_return rc;
}

/// Signed WS API call: connect → logon → execute → close.
template<typename Request>
boost::cobalt::task<int>
exec_ws_signed(binapi2::futures_usdm_api& c, Request req, result_sink& sink)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) co_return detail::emit_failure(ws.err, sink);
    if (auto conn = co_await (*ws)->async_connect(); !conn) {
        co_await (*ws)->async_close();
        co_return detail::emit_failure(conn.err, sink);
    }
    if (auto l = co_await (*ws)->async_session_logon(); !l) {
        co_await (*ws)->async_close();
        co_return detail::emit_failure(l.err, sink);
    }
    auto r = co_await (*ws)->async_execute(req);
    if (!r) {
        co_await (*ws)->async_close();
        co_return detail::emit_failure(r.err, sink);
    }

    sink.on_info("status=" + std::to_string(r->status));
    int rc = 0;
    if (r->result) {
        rc = detail::emit_success(*r->result, sink);
    } else {
        sink.on_done(0);
    }
    co_await (*ws)->async_close();
    co_return rc;
}

// ---------------------------------------------------------------------------
// Market stream executor.
// ---------------------------------------------------------------------------

/// Pump an already-subscribed generator's events through `sink` until
/// exhausted, or until `*stop` becomes true (checked between events;
/// pass `nullptr` for no stop token).
///
/// Exposed as a free template so the CLI can wrap it with its
/// record-buffer-attaching variant (see `async-demo-cli/common.hpp`)
/// without duplicating the event loop.
template<typename Generator>
boost::cobalt::task<int>
run_stream(Generator gen, result_sink& sink,
           const std::atomic<bool>* stop = nullptr)
{
    while (gen) {
        if (stop && stop->load(std::memory_order_relaxed)) break;
        auto event = co_await gen;
        if (!event) co_return detail::emit_failure(event.err, sink);
        auto json = glz::write<glz::opts{ .prettify = true }>(*event);
        if (json) sink.on_response_json(*json);
    }
    sink.on_done(0);
    co_return 0;
}

/// Subscribe to a market stream and pump events through `sink`. Simple
/// wrapper around `run_stream` for surfaces that don't need to touch
/// the `market_stream` handle before subscribing.
template<typename Subscription>
boost::cobalt::task<int>
exec_stream(binapi2::futures_usdm_api& c, Subscription sub, result_sink& sink,
            const std::atomic<bool>* stop = nullptr)
{
    auto streams = c.create_market_stream();
    co_return co_await run_stream(streams->subscribe(sub), sink, stop);
}

} // namespace binapi2::demo
