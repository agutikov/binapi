// SPDX-License-Identifier: Apache-2.0
//
// Account commands — authenticated REST endpoints.

#include "cmd_account.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

// ---------------------------------------------------------------------------
// Existing commands (keep custom summaries for account-info and balances)
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_account_info(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->account.async_execute(types::account_information_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("feeTier={} canTrade={} assets={} positions={}",
                 r->feeTier.value_or(-1), r->canTrade.value_or(false),
                 r->assets.size(), r->positions.size());
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_balances(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
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
    types::position_risk_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_income_history(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::income_history_request_t req;
    if (!args.empty()) req.symbol = args[0];
    if (args.size() >= 2) req.limit = std::stoi(args[1]);
    co_return co_await exec_account(c, req);
}

// ---------------------------------------------------------------------------
// New account commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_account_config(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_account(c, types::account_config_request_t{});
}

boost::cobalt::task<int> cmd_symbol_config(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::symbol_config_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_multi_assets_mode(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_account(c, types::get_multi_assets_mode_request_t{});
}

boost::cobalt::task<int> cmd_position_mode(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_account(c, types::get_position_mode_request_t{});
}

boost::cobalt::task<int> cmd_rate_limit_order(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_account(c, types::rate_limit_order_request_t{});
}

boost::cobalt::task<int> cmd_leverage_bracket(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::leverage_bracket_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_commission_rate(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: commission-rate <symbol>"); co_return 1; }
    types::commission_rate_request_t req;
    req.symbol = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_bnb_burn(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_account(c, types::get_bnb_burn_request_t{});
}

boost::cobalt::task<int> cmd_toggle_bnb_burn(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: toggle-bnb-burn <true|false>"); co_return 1; }
    types::toggle_bnb_burn_request_t req;
    req.feeBurn = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_quantitative_rules(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::quantitative_rules_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_pm_account_info(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: pm-account-info <asset>"); co_return 1; }
    types::pm_account_info_request_t req;
    req.asset = args[0];
    co_return co_await exec_account(c, req);
}

boost::cobalt::task<int> cmd_download_id_transaction(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_id<types::download_id_transaction_request_t>(
        c, args, "download-id-transaction <startTime> <endTime>");
}

boost::cobalt::task<int> cmd_download_link_transaction(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_link<types::download_link_transaction_request_t>(
        c, args, "download-link-transaction <downloadId>");
}

boost::cobalt::task<int> cmd_download_id_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_id<types::download_id_order_request_t>(
        c, args, "download-id-order <startTime> <endTime>");
}

boost::cobalt::task<int> cmd_download_link_order(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_link<types::download_link_order_request_t>(
        c, args, "download-link-order <downloadId>");
}

boost::cobalt::task<int> cmd_download_id_trade(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_id<types::download_id_trade_request_t>(
        c, args, "download-id-trade <startTime> <endTime>");
}

boost::cobalt::task<int> cmd_download_link_trade(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_download_link<types::download_link_trade_request_t>(
        c, args, "download-link-trade <downloadId>");
}

} // namespace demo
