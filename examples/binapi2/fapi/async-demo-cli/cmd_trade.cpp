// SPDX-License-Identifier: Apache-2.0
//
// Trade commands — authenticated order management.
// Demonstrates: async_execute() with trade request types, test_order_request_t,
// and flag-based argument parsing for order parameters.

#include "cmd_trade.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

static types::new_order_request_t
make_order_request(const args_t& args)
{
    types::new_order_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.type = parse_enum<types::order_type_t>(args[2]);

    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--tif", "-t"); !v.empty())
        req.timeInForce = parse_enum<types::time_in_force_t>(v);

    spdlog::debug("order request: symbol={} side={} type={}",
                  req.symbol, to_string(req.side), to_string(req.type));
    return req;
}

boost::cobalt::task<int> cmd_new_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: new-order <symbol> <side> <type> [-q|--quantity Q] [-p|--price P] [-t|--tif TIF]"); co_return 1;
    }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }

    auto req = make_order_request(args);
    spdlog::info("placing order...");
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_test_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: test-order <symbol> <side> <type> [-q|--quantity Q] [-p|--price P] [-t|--tif TIF]"); co_return 1;
    }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }

    auto order = make_order_request(args);
    types::test_order_request_t req;
    req.symbol = order.symbol;
    req.side = order.side;
    req.type = order.type;
    req.quantity = order.quantity;
    req.price = order.price;
    req.timeInForce = order.timeInForce;

    spdlog::info("placing test order (validation only)...");
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_cancel_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: cancel-order <symbol> <orderId>"); co_return 1; }

    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::cancel_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::info("cancelling order {} on {}", *req.orderId, req.symbol);
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_query_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: query-order <symbol> <orderId>"); co_return 1; }

    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::query_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);

    spdlog::debug("querying order {} on {}", *req.orderId, req.symbol);
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_open_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::all_open_orders_request_t req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("querying open orders symbol={}", req.symbol.value_or("(all)"));
    auto r = co_await (*rest)->trade.async_execute(req);
    co_return handle_result(r);
}

} // namespace demo
