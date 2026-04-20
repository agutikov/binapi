// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API command registry. Mirrors `rest/commands_*.cpp` but
// dispatches through `binapi2::demo::exec_ws_public` / `exec_ws_signed`.

#include "commands.hpp"

#include <binapi2/demo_commands/enum_parse.hpp>
#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>

#include <glaze/glaze.hpp>

#include <spdlog/spdlog.h>

namespace demo_ui::ws {

namespace R = demo_ui::rest;

namespace lib   = binapi2::demo;
namespace types = binapi2::fapi::types;

// ── Bespoke session-logon coroutine ─────────────────────────────────

boost::cobalt::task<void>
ws_logon_coro(worker& wrk,
              std::shared_ptr<capture_sink> sink,
              std::shared_ptr<request_capture> cap)
{
    active_capture_guard guard(wrk, cap.get());
    auto ws = co_await wrk.api().create_ws_api_client();
    if (!ws) { sink->on_error(ws.err); sink->on_done(1); co_return; }
    if (auto c = co_await (*ws)->async_connect(); !c) {
        co_await (*ws)->async_close();
        sink->on_error(c.err); sink->on_done(1); co_return;
    }
    auto r = co_await (*ws)->async_session_logon();
    if (!r) {
        co_await (*ws)->async_close();
        sink->on_error(r.err); sink->on_done(1); co_return;
    }
    sink->on_info("session logon ok, status=" + std::to_string(r->status));
    if (r->result) {
        if (auto j = glz::write<glz::opts{ .prettify = true }>(*r->result); j)
            sink->on_response_json(*j);
    }
    co_await (*ws)->async_close();
    sink->on_done(0);
}

namespace {

// ── Per-service coroutine templates ─────────────────────────────────

template<typename Request>
boost::cobalt::task<void>
run_ws_pub(worker& wrk,
           std::shared_ptr<capture_sink> sink,
           std::shared_ptr<request_capture> cap,
           Request req)
{
    active_capture_guard guard(wrk, cap.get());
    co_await lib::exec_ws_public(wrk.api(), std::move(req), *sink);
}

template<typename Request>
boost::cobalt::task<void>
run_ws_sig(worker& wrk,
           std::shared_ptr<capture_sink> sink,
           std::shared_ptr<request_capture> cap,
           Request req)
{
    active_capture_guard guard(wrk, cap.get());
    co_await lib::exec_ws_signed(wrk.api(), std::move(req), *sink);
}

// ── Spawn helper ─────────────────────────────────────────────────────

template<typename Request, auto SpawnFn>
void spawn_ws(const R::cmd_ctx& c, Request req)
{
    R::reset_capture(*c.cap);
    auto sink = std::make_shared<capture_sink>(c.cap, c.wrk, c.state);
    prefill_request_json(*c.cap, req);
    boost::cobalt::spawn(c.wrk.io().get_executor(),
                         SpawnFn(c.wrk, std::move(sink), c.cap, std::move(req)),
                         boost::asio::use_future);
}

template<typename Request>
void run_pub(const R::cmd_ctx& c, Request req)
{
    spawn_ws<Request, &run_ws_pub<Request>>(c, std::move(req));
}

template<typename Request>
void run_sig(const R::cmd_ctx& c, Request req)
{
    spawn_ws<Request, &run_ws_sig<Request>>(c, std::move(req));
}

// ── Command implementations ──────────────────────────────────────────

// Session logon — bespoke (no typed request, no exec_ws_*).
void cmd_ws_logon(const R::cmd_ctx& c)
{
    R::reset_capture(*c.cap);
    auto sink = std::make_shared<capture_sink>(c.cap, c.wrk, c.state);
    boost::cobalt::spawn(c.wrk.io().get_executor(),
                         ws_logon_coro(c.wrk, std::move(sink), c.cap),
                         boost::asio::use_future);
}

// Signed, no-args queries.
void cmd_ws_account_status    (const R::cmd_ctx& c) { run_sig(c, types::ws_account_status_request_t{}); }
void cmd_ws_account_status_v2 (const R::cmd_ctx& c) { run_sig(c, types::ws_account_status_v2_request_t{}); }
void cmd_ws_account_balance   (const R::cmd_ctx& c) { run_sig(c, types::ws_account_balance_request_t{}); }
void cmd_ws_user_stream_start (const R::cmd_ctx& c) { run_sig(c, types::ws_user_data_stream_start_request_t{}); }
void cmd_ws_user_stream_ping  (const R::cmd_ctx& c) { run_sig(c, types::ws_user_data_stream_ping_request_t{}); }
void cmd_ws_user_stream_stop  (const R::cmd_ctx& c) { run_sig(c, types::ws_user_data_stream_stop_request_t{}); }

// Public optional-symbol tickers — dispatch to "all" endpoint when symbol
// is blank, singular endpoint otherwise (mirrors the CLI behaviour).
void cmd_ws_book_ticker(const R::cmd_ctx& c)
{
    if (c.form.symbol.empty()) {
        run_pub(c, types::websocket_api_book_tickers_request_t{});
    } else {
        types::websocket_api_book_ticker_request_t req;
        req.symbol = c.form.symbol;
        run_pub(c, std::move(req));
    }
}

void cmd_ws_price_ticker(const R::cmd_ctx& c)
{
    if (c.form.symbol.empty()) {
        run_pub(c, types::websocket_api_price_tickers_request_t{});
    } else {
        types::websocket_api_price_ticker_request_t req;
        req.symbol = c.form.symbol;
        run_pub(c, std::move(req));
    }
}

// Order management (signed).
void cmd_ws_order_place(const R::cmd_ctx& c)
{
    types::websocket_api_order_place_request_t req;
    req.symbol = c.form.symbol;
    req.side = lib::parse_enum<types::order_side_t>(c.form.side);
    req.type = lib::parse_enum<types::order_type_t>(c.form.order_type);
    if (!c.form.quantity.empty()) req.quantity = R::parse_decimal(c.form.quantity);
    if (!c.form.price.empty())    req.price    = R::parse_decimal(c.form.price);
    if (!c.form.tif.empty())      req.timeInForce = lib::parse_enum<types::time_in_force_t>(c.form.tif);
    run_sig(c, std::move(req));
}

void cmd_ws_order_query(const R::cmd_ctx& c)
{
    types::websocket_api_order_query_request_t req;
    req.symbol  = c.form.symbol;
    req.orderId = R::parse_optional_u64(c.form.order_id);
    run_sig(c, std::move(req));
}

void cmd_ws_order_cancel(const R::cmd_ctx& c)
{
    types::websocket_api_order_cancel_request_t req;
    req.symbol  = c.form.symbol;
    req.orderId = R::parse_optional_u64(c.form.order_id);
    run_sig(c, std::move(req));
}

void cmd_ws_order_modify(const R::cmd_ctx& c)
{
    types::websocket_api_order_modify_request_t req;
    req.symbol   = c.form.symbol;
    req.side     = lib::parse_enum<types::order_side_t>(c.form.side);
    req.orderId  = R::parse_optional_u64(c.form.order_id);
    req.quantity = R::parse_decimal(c.form.quantity);
    req.price    = R::parse_decimal(c.form.price);
    run_sig(c, std::move(req));
}

void cmd_ws_position(const R::cmd_ctx& c)
{
    types::websocket_api_position_request_t req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    run_sig(c, std::move(req));
}

// Algo orders (signed).
void cmd_ws_algo_order_place(const R::cmd_ctx& c)
{
    types::websocket_api_algo_order_place_request_t req;
    req.symbol = c.form.symbol;
    req.side = lib::parse_enum<types::order_side_t>(c.form.side);
    req.type = lib::parse_enum<types::order_type_t>(c.form.order_type);
    req.algoType = lib::parse_enum<types::algo_type_t>(c.form.algo_type);
    if (!c.form.quantity.empty()) req.quantity = R::parse_decimal(c.form.quantity);
    if (!c.form.price.empty())    req.price    = R::parse_decimal(c.form.price);
    run_sig(c, std::move(req));
}

void cmd_ws_algo_order_cancel(const R::cmd_ctx& c)
{
    types::websocket_api_algo_order_cancel_request_t req;
    req.algoId = R::parse_optional_u64(c.form.algo_id);
    run_sig(c, std::move(req));
}

// ── Registry table ──────────────────────────────────────────────────

constexpr R::rest_command entries[] = {
    { "ws-logon",               "Session logon",                          R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_logon },
    { "ws-account-status",      "Account status",                         R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_account_status },
    { "ws-account-status-v2",   "Account status v2",                      R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_account_status_v2 },
    { "ws-account-balance",     "Account balance",                        R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_account_balance },

    { "ws-book-ticker",         "Book ticker [symbol]",                   R::command_group::ws_public, R::form_kind::symbol_opt,        &cmd_ws_book_ticker },
    { "ws-price-ticker",        "Price ticker [symbol]",                  R::command_group::ws_public, R::form_kind::symbol_opt,        &cmd_ws_price_ticker },

    { "ws-order-place",         "Place order",                            R::command_group::ws_signed, R::form_kind::order_form,        &cmd_ws_order_place },
    { "ws-order-query",         "Query order",                            R::command_group::ws_signed, R::form_kind::symbol_order_id,   &cmd_ws_order_query },
    { "ws-order-cancel",        "Cancel order",                           R::command_group::ws_signed, R::form_kind::symbol_order_id,   &cmd_ws_order_cancel },
    { "ws-order-modify",        "Modify order",                           R::command_group::ws_signed, R::form_kind::modify_order_form, &cmd_ws_order_modify },

    { "ws-position",            "Position info [symbol]",                 R::command_group::ws_signed, R::form_kind::symbol_opt,        &cmd_ws_position },

    { "ws-algo-order-place",    "Place algo order",                       R::command_group::ws_signed, R::form_kind::algo_order_form,   &cmd_ws_algo_order_place },
    { "ws-algo-order-cancel",   "Cancel algo order",                      R::command_group::ws_signed, R::form_kind::algo_id_form,      &cmd_ws_algo_order_cancel },

    { "ws-user-stream-start",   "Start user data stream",                 R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_user_stream_start },
    { "ws-user-stream-ping",    "Ping user data stream",                  R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_user_stream_ping },
    { "ws-user-stream-stop",    "Stop user data stream",                  R::command_group::ws_signed, R::form_kind::no_args,           &cmd_ws_user_stream_stop },
};

} // namespace

std::span<const R::rest_command> ws_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::ws
