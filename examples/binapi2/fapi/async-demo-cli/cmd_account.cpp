// SPDX-License-Identifier: Apache-2.0
//
// Account commands — authenticated REST endpoints (read-only).
// Demonstrates: parameterless auth endpoints and async_execute() with auth request types.

#include "cmd_account.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_account_info(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    spdlog::debug("executing account_information_t request");
    auto r = co_await (*rest)->account.async_execute(types::account_information_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("feeTier={} canTrade={} assets={} positions={}",
                 r->feeTier, r->canTrade, r->assets.size(), r->positions.size());
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_balances(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    spdlog::debug("executing balances request");
    auto r = co_await (*rest)->account.async_execute(types::balances_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("balances: {} entries", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& b : *r) {
            if (b.balance.is_zero()) continue;
            out("  {}  balance: {}  available: {}", b.asset, b.balance, b.availableBalance);
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_position_risk(binapi2::futures_usdm_api& c, const args_t& args)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::position_risk_request_t req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("executing position_risk_t request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = co_await (*rest)->account.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_income_history(binapi2::futures_usdm_api& c, const args_t& args)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::income_history_request_t req;
    if (!args.empty()) req.symbol = args[0];
    if (args.size() >= 2) req.limit = std::stoi(args[1]);

    spdlog::debug("executing income_history request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = co_await (*rest)->account.async_execute(req);
    co_return handle_result(r);
}

} // namespace demo
