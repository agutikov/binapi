// SPDX-License-Identifier: Apache-2.0
//
// WebSocket API commands — JSON-RPC over WebSocket.

#include "cmd_ws_api.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

// ---------------------------------------------------------------------------
// Existing commands (ws-logon keeps its custom body; others refactored)
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_ws_logon(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }

    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }

    auto r = co_await (*ws)->async_session_logon();
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }

    spdlog::info("session logon ok, status={}", r->status);
    if (verbosity >= 1 && r->result) print_json(*r->result);

    co_await (*ws)->async_close();
    co_return 0;
}

boost::cobalt::task<int> cmd_ws_account_status(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_account_status_request_t{});
}

boost::cobalt::task<int> cmd_ws_order_place(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: ws-order-place <symbol> <side> <type> [-q Q] [-p P] [-t TIF]"); co_return 1;
    }
    types::websocket_api_order_place_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.type = parse_enum<types::order_type_t>(args[2]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--tif", "-t"); !v.empty())
        req.timeInForce = parse_enum<types::time_in_force_t>(v);
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_order_cancel(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: ws-order-cancel <symbol> <orderId>"); co_return 1; }
    types::websocket_api_order_cancel_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);
    co_return co_await exec_ws_signed(c, req);
}

// ---------------------------------------------------------------------------
// New WS API commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_ws_book_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) {
        co_return co_await exec_ws_public(c, types::websocket_api_book_tickers_request_t{});
    }
    types::websocket_api_book_ticker_request_t req;
    req.symbol = args[0];
    co_return co_await exec_ws_public(c, req);
}

boost::cobalt::task<int> cmd_ws_price_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) {
        co_return co_await exec_ws_public(c, types::websocket_api_price_tickers_request_t{});
    }
    types::websocket_api_price_ticker_request_t req;
    req.symbol = args[0];
    co_return co_await exec_ws_public(c, req);
}

boost::cobalt::task<int> cmd_ws_order_query(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: ws-order-query <symbol> <orderId>"); co_return 1; }
    types::websocket_api_order_query_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_order_modify(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: ws-order-modify <symbol> <side> <orderId> -q Q -p P"); co_return 1;
    }
    types::websocket_api_order_modify_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.orderId = std::stoull(args[2]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_position(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::websocket_api_position_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_account_status_v2(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_account_status_v2_request_t{});
}

boost::cobalt::task<int> cmd_ws_account_balance(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_account_balance_request_t{});
}

boost::cobalt::task<int> cmd_ws_algo_order_place(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 4) {
        spdlog::error("usage: ws-algo-order-place <symbol> <side> <type> <algoType> -q Q [-p P]"); co_return 1;
    }
    types::websocket_api_algo_order_place_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.type = parse_enum<types::order_type_t>(args[2]);
    req.algoType = parse_enum<types::algo_type_t>(args[3]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_algo_order_cancel(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: ws-algo-order-cancel <algoId>"); co_return 1; }
    types::websocket_api_algo_order_cancel_request_t req;
    req.algoId = std::stoull(args[0]);
    co_return co_await exec_ws_signed(c, req);
}

boost::cobalt::task<int> cmd_ws_user_stream_start(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_user_data_stream_start_request_t{});
}

boost::cobalt::task<int> cmd_ws_user_stream_ping(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_user_data_stream_ping_request_t{});
}

boost::cobalt::task<int> cmd_ws_user_stream_stop(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_ws_signed(c, types::ws_user_data_stream_stop_request_t{});
}

} // namespace demo
