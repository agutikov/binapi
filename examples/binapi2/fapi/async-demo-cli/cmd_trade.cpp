// SPDX-License-Identifier: Apache-2.0
//
// Trade commands — authenticated order management and position configuration.

#include "cmd_trade.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>

#include <spdlog/spdlog.h>

#include <sstream>

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

    return req;
}

// ---------------------------------------------------------------------------
// Existing commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_new_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: new-order <symbol> <side> <type> [-q Q] [-p P] [-t TIF]"); co_return 1;
    }
    co_return co_await exec_trade(c, make_order_request(args));
}

boost::cobalt::task<int> cmd_test_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: test-order <symbol> <side> <type> [-q Q] [-p P] [-t TIF]"); co_return 1;
    }
    auto order = make_order_request(args);
    types::test_order_request_t req;
    req.symbol = order.symbol;
    req.side = order.side;
    req.type = order.type;
    req.quantity = order.quantity;
    req.price = order.price;
    req.timeInForce = order.timeInForce;
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_cancel_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: cancel-order <symbol> <orderId>"); co_return 1; }
    types::cancel_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_query_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: query-order <symbol> <orderId>"); co_return 1; }
    types::query_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_open_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::all_open_orders_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_trade(c, req);
}

// ---------------------------------------------------------------------------
// New trade commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_modify_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) {
        spdlog::error("usage: modify-order <symbol> <side> <orderId> -q Q -p P"); co_return 1;
    }
    types::modify_order_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.orderId = std::stoull(args[2]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_cancel_multiple_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: cancel-multiple-orders <symbol> <id1,id2,...>"); co_return 1; }
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::cancel_multiple_orders_request_t req;
    req.symbol = args[0];
    std::vector<std::uint64_t> ids;
    std::istringstream ss(args[1]);
    std::string tok;
    while (std::getline(ss, tok, ','))
        ids.push_back(std::stoull(tok));
    req.orderIdList = std::move(ids);
    auto r = co_await (*rest)->trade.async_cancel_batch_orders(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_cancel_all_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: cancel-all-orders <symbol>"); co_return 1; }
    types::cancel_all_open_orders_request_t req;
    req.symbol = args[0];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_auto_cancel(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: auto-cancel <symbol> <countdownMs>"); co_return 1; }
    types::auto_cancel_request_t req;
    req.symbol = args[0];
    req.countdownTime = std::stoull(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_query_open_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: query-open-order <symbol> <orderId>"); co_return 1; }
    types::query_open_order_request_t req;
    req.symbol = args[0];
    req.orderId = std::stoull(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_all_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: all-orders <symbol> [limit]"); co_return 1; }
    types::all_orders_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_position_info_v3(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::position_info_v3_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_adl_quantile(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::adl_quantile_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_force_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::force_orders_request_t req;
    if (!args.empty()) req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_account_trades(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: account-trades <symbol> [limit]"); co_return 1; }
    types::account_trade_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_change_position_mode(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: change-position-mode <true|false>"); co_return 1; }
    types::change_position_mode_request_t req;
    req.dualSidePosition = args[0];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_change_multi_assets_mode(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: change-multi-assets-mode <true|false>"); co_return 1; }
    types::change_multi_assets_mode_request_t req;
    req.multiAssetsMargin = args[0];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_change_leverage(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: change-leverage <symbol> <leverage>"); co_return 1; }
    types::change_leverage_request_t req;
    req.symbol = args[0];
    req.leverage = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_change_margin_type(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: change-margin-type <symbol> <ISOLATED|CROSSED>"); co_return 1; }
    types::change_margin_type_request_t req;
    req.symbol = args[0];
    req.marginType = args[1];
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_modify_isolated_margin(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 3) { spdlog::error("usage: modify-isolated-margin <symbol> <amount> <1|2>"); co_return 1; }
    types::modify_isolated_margin_request_t req;
    req.symbol = args[0];
    req.amount = types::decimal_t(std::string(args[1]));
    req.type = static_cast<types::delta_type_t>(std::stoi(args[2]));
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_position_margin_history(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: position-margin-history <symbol> [limit]"); co_return 1; }
    types::position_margin_history_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_order_modify_history(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: order-modify-history <symbol> [orderId]"); co_return 1; }
    types::order_modify_history_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.orderId = std::stoull(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_new_algo_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 4) {
        spdlog::error("usage: new-algo-order <symbol> <side> <type> <algoType> -q Q [-p P]"); co_return 1;
    }
    types::new_algo_order_request_t req;
    req.symbol = args[0];
    req.side = parse_enum<types::order_side_t>(args[1]);
    req.type = parse_enum<types::order_type_t>(args[2]);
    req.algoType = parse_enum<types::algo_type_t>(args[3]);
    if (auto v = find_flag(args, "--quantity", "-q"); !v.empty())
        req.quantity = types::decimal_t(std::string(v));
    if (auto v = find_flag(args, "--price", "-p"); !v.empty())
        req.price = types::decimal_t(std::string(v));
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_cancel_algo_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: cancel-algo-order <algoId>"); co_return 1; }
    types::cancel_algo_order_request_t req;
    req.algoId = std::stoull(args[0]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_query_algo_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: query-algo-order <algoId>"); co_return 1; }
    types::query_algo_order_request_t req;
    req.algoId = std::stoull(args[0]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_all_algo_orders(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: all-algo-orders <symbol> [limit]"); co_return 1; }
    types::all_algo_orders_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_trade(c, req);
}

boost::cobalt::task<int> cmd_open_algo_orders(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_trade(c, types::open_algo_orders_request_t{});
}

boost::cobalt::task<int> cmd_cancel_all_algo_orders(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_trade(c, types::cancel_all_algo_orders_request_t{});
}

boost::cobalt::task<int> cmd_tradfi_perps(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_trade(c, types::tradfi_perps_request_t{});
}

} // namespace demo
