// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API commands — authenticated JSON-RPC over WebSocket.
// Demonstrates: connect/close lifecycle, session_logon(), account_status(),
// and generic execute() with ws endpoint traits for order placement/cancel.

#include "cmd_ws_api.hpp"

#include <binapi2/fapi/websocket_api/client.hpp>
#include <binapi2/fapi/types/decimal.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

int cmd_ws_logon(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::websocket_api::client ws{ io, make_config() };

    spdlog::debug("connecting to WebSocket API");
    if (auto c = ws.connect(); !c) { print_error(c.err); return 1; }
    spdlog::debug("connected, performing session logon");

    auto r = ws.session_logon();
    if (!r) { print_error(r.err); (void)ws.close(); return 1; }

    spdlog::info("session logon ok, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    (void)ws.close();
    return 0;
}

int cmd_ws_account_status(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::websocket_api::client ws{ io, make_config() };

    spdlog::debug("connecting to WebSocket API");
    if (auto c = ws.connect(); !c) { print_error(c.err); return 1; }

    spdlog::debug("session logon");
    auto logon = ws.session_logon();
    if (!logon) { print_error(logon.err); (void)ws.close(); return 1; }

    spdlog::debug("querying account status");
    auto r = ws.account_status();
    if (!r) { print_error(r.err); (void)ws.close(); return 1; }

    if (r->result)
        spdlog::info("feeTier={} canTrade={}", r->result->feeTier, r->result->canTrade);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    (void)ws.close();
    return 0;
}

int cmd_ws_order_place(const args_t& args)
{
    if (args.size() < 3) {
        std::cerr << "usage: ws-order-place <symbol> <side> <type> [--quantity Q] [--price P]\n";
        return 1;
    }

    boost::asio::io_context io;
    binapi2::fapi::websocket_api::client ws{ io, make_config() };

    spdlog::debug("connecting to WebSocket API");
    if (auto c = ws.connect(); !c) { print_error(c.err); return 1; }
    spdlog::debug("session logon");
    if (auto l = ws.session_logon(); !l) { print_error(l.err); (void)ws.close(); return 1; }

    binapi2::fapi::websocket_api::client::order_place_request req;
    req.symbol = args[0];
    req.side = parse_enum<binapi2::fapi::types::order_side>(args[1]);
    req.type = parse_enum<binapi2::fapi::types::order_type>(args[2]);
    if (auto v = find_flag(args, "--quantity"); !v.empty())
        req.quantity = binapi2::fapi::types::decimal(std::string(v));
    if (auto v = find_flag(args, "--price"); !v.empty())
        req.price = binapi2::fapi::types::decimal(std::string(v));

    spdlog::info("placing order via WS API: {} {} {}", req.symbol,
                 to_string(req.side), to_string(req.type));
    auto r = ws.execute(req);
    if (!r) { print_error(r.err); (void)ws.close(); return 1; }

    spdlog::info("order placed, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    (void)ws.close();
    return 0;
}

int cmd_ws_order_cancel(const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: ws-order-cancel <symbol> <orderId>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::websocket_api::client ws{ io, make_config() };

    spdlog::debug("connecting to WebSocket API");
    if (auto c = ws.connect(); !c) { print_error(c.err); return 1; }
    spdlog::debug("session logon");
    if (auto l = ws.session_logon(); !l) { print_error(l.err); (void)ws.close(); return 1; }

    binapi2::fapi::websocket_api::client::order_cancel_request req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::info("cancelling order {} on {} via WS API", *req.orderId, req.symbol);
    auto r = ws.execute(req);
    if (!r) { print_error(r.err); (void)ws.close(); return 1; }

    spdlog::info("order cancelled, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    (void)ws.close();
    return 0;
}

} // namespace demo
