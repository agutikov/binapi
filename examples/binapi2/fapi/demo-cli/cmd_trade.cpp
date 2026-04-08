// SPDX-License-Identifier: Apache-2.0
//
// Trade commands — authenticated order management.
// Demonstrates: execute() with trade request types, named method test_order(),
// and flag-based argument parsing for order parameters.

#include "cmd_trade.hpp"

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

static binapi2::fapi::types::new_order_request_t
make_order_request(const args_t& args)
{
    binapi2::fapi::types::new_order_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<binapi2::fapi::types::order_side_t>(args[1]);
    req.type = parse_enum<binapi2::fapi::types::order_type_t>(args[2]);

    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = binapi2::fapi::types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = binapi2::fapi::types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--tif", "-t"); !v.empty())
        req.timeInForce = parse_enum<binapi2::fapi::types::time_in_force_t>(v);

    spdlog::debug("order request: symbol={} side={} type={}",
                  req.symbol, to_string(req.side), to_string(req.type));
    return req;
}

int cmd_new_order(const args_t& args)
{
    if (args.size() < 3) {
        std::cerr << "usage: new-order <symbol> <side> <type> [-q|--quantity Q] [-p|--price P] [-t|--tif TIF]\n";
        return 1;
    }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    auto req = make_order_request(args);
    spdlog::info("placing order...");
    auto r = client.trade.execute(req);
    return handle_result(r);
}

int cmd_test_order(const args_t& args)
{
    if (args.size() < 3) {
        std::cerr << "usage: test-order <symbol> <side> <type> [-q|--quantity Q] [-p|--price P] [-t|--tif TIF]\n";
        return 1;
    }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    auto req = make_order_request(args);
    spdlog::info("placing test order (validation only)...");
    auto r = client.trade.test_order(req);
    return handle_result(r);
}

int cmd_cancel_order(const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: cancel-order <symbol> <orderId>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::cancel_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::info("cancelling order {} on {}", *req.orderId, req.symbol);
    auto r = client.trade.execute(req);
    return handle_result(r);
}

int cmd_query_order(const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: query-order <symbol> <orderId>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::query_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::debug("querying order {} on {}", *req.orderId, req.symbol);
    auto r = client.trade.execute(req);
    return handle_result(r);
}

int cmd_open_orders(const args_t& args)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::all_open_orders_request_t req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("querying open orders symbol={}", req.symbol.value_or("(all)"));
    auto r = client.trade.execute(req);
    return handle_result(r);
}

} // namespace demo
