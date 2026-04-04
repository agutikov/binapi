// SPDX-License-Identifier: Apache-2.0
//
// Account commands — authenticated REST endpoints (read-only).
// Demonstrates: parameterless auth endpoints and execute() with auth request types.

#include "cmd_account.hpp"

#include <binapi2/fapi/client.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

int cmd_account_info(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing account_information request");
    auto r = client.account.account_information();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("feeTier={} canTrade={} assets={} positions={}",
                 r->feeTier, r->canTrade, r->assets.size(), r->positions.size());
    if (verbosity >= 1) print_json(*r);
    return 0;
}

int cmd_balances(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing balances request");
    auto r = client.account.balances();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("balances: {} entries", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& b : *r) {
            if (b.balance.is_zero()) continue;
            std::cout << "  " << b.asset << "  balance: " << b.balance
                      << "  available: " << b.availableBalance << '\n';
        }
    }
    return 0;
}

int cmd_position_risk(const args_t& args)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::position_risk_request req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("executing position_risk request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = client.account.execute(req);
    return handle_result(r);
}

int cmd_income_history(const args_t& args)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::income_history_request req;
    if (!args.empty()) req.symbol = args[0];
    if (args.size() >= 2) req.limit = std::stoi(args[1]);

    spdlog::debug("executing income_history request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = client.account.execute(req);
    return handle_result(r);
}

} // namespace demo
