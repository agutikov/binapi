// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API commands — authenticated JSON-RPC over WebSocket.
// Demonstrates: async connect/close lifecycle, async_session_logon(),
// async_execute() with ws endpoint traits for account status and order management.

#include "cmd_ws_api.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_ws_logon(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }

    spdlog::debug("connecting to WebSocket API");
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    spdlog::debug("connected, performing session logon");

    auto r = co_await (*ws)->async_session_logon();
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }

    spdlog::info("session logon ok, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    co_await (*ws)->async_close();
    co_return 0;
}

boost::cobalt::task<int> cmd_ws_account_status(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }

    spdlog::debug("connecting to WebSocket API");
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }

    spdlog::debug("session logon");
    auto logon = co_await (*ws)->async_session_logon();
    if (!logon) { print_error(logon.err); co_await (*ws)->async_close(); co_return 1; }

    spdlog::debug("querying account status");
    auto r = co_await (*ws)->async_execute(types::ws_account_status_request_t{});
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }

    if (r->result)
        spdlog::info("feeTier={} canTrade={}", r->result->feeTier, r->result->canTrade);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    co_await (*ws)->async_close();
    co_return 0;
}

boost::cobalt::task<int> cmd_ws_order_place(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: ws-order-place <symbol> <side> <type> [--quantity Q] [--price P]"); co_return 1;
    }

    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }

    spdlog::debug("connecting to WebSocket API");
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    spdlog::debug("session logon");
    if (auto l = co_await (*ws)->async_session_logon(); !l) { print_error(l.err); co_await (*ws)->async_close(); co_return 1; }

    types::websocket_api_order_place_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.type = parse_enum<types::order_type_t>(args[2]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));

    spdlog::info("placing order via WS API: {} {} {}", req.symbol,
                 to_string(req.side), to_string(req.type));
    auto r = co_await (*ws)->async_execute(req);
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }

    spdlog::info("order placed, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    co_await (*ws)->async_close();
    co_return 0;
}

boost::cobalt::task<int> cmd_ws_order_cancel(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: ws-order-cancel <symbol> <orderId>"); co_return 1; }

    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }

    spdlog::debug("connecting to WebSocket API");
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    spdlog::debug("session logon");
    if (auto l = co_await (*ws)->async_session_logon(); !l) { print_error(l.err); co_await (*ws)->async_close(); co_return 1; }

    types::websocket_api_order_cancel_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::info("cancelling order {} on {} via WS API", *req.orderId, req.symbol);
    auto r = co_await (*ws)->async_execute(req);
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }

    spdlog::info("order cancelled, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    co_await (*ws)->async_close();
    co_return 0;
}

} // namespace demo
