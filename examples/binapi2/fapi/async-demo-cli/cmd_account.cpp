// SPDX-License-Identifier: Apache-2.0
//
// Account commands — authenticated REST endpoints.

#include "cmd_account.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

template<typename Request>
CLI::App* add_noarg(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_account(c, Request{}, sink);
        };
    });
    return sub;
}

template<typename Request>
CLI::App* add_optional_symbol(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            Request req;
            if (!opts->symbol.empty()) req.symbol = opts->symbol;
            co_return co_await lib::exec_account(c, std::move(req), sink);
        };
    });
    return sub;
}

} // namespace

void register_cmd_account(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Account";

    // account-info: bespoke summary — typed field access, not just JSON.
    auto* info = app.add_subcommand("account-info", "Account information (auth)");
    info->group(group);
    info->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            auto rest = co_await c.create_rest_client();
            if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
            auto r = co_await (*rest)->account.async_execute(types::account_information_request_t{});
            if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
            sink.on_info(
                "feeTier=" + std::to_string(r->feeTier.value_or(-1))
                + " canTrade=" + (r->canTrade.value_or(false) ? "true" : "false")
                + " assets=" + std::to_string(r->assets.size())
                + " positions=" + std::to_string(r->positions.size()));
            if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                sink.on_response_json(*j);
            sink.on_done(0);
            co_return 0;
        };
    });

    // balances: non-zero filter as a multi-line summary when -v is off.
    auto* balances = app.add_subcommand("balances", "Account balances (auth)");
    balances->group(group);
    balances->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            auto rest = co_await c.create_rest_client();
            if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
            auto r = co_await (*rest)->account.async_execute(types::balances_request_t{});
            if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
            sink.on_info("balances: " + std::to_string(r->size()) + " entries");
            if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                sink.on_response_json(*j);
            // Fallback summary when JSON channel is gated off (verbosity == 0).
            if (verbosity < 1) {
                for (auto& b : *r) {
                    if (b.balance.is_zero()) continue;
                    out("  {}  balance: {}  available: {}", b.asset, b.balance, b.availableBalance);
                }
            }
            sink.on_done(0);
            co_return 0;
        };
    });

    add_optional_symbol<types::position_risk_request_t>     (app, "position-risk",      "Position risk [symbol] (auth)",    sel)->group(group);
    add_optional_symbol<types::symbol_config_request_t>     (app, "symbol-config",      "Symbol config [symbol] (auth)",    sel)->group(group);
    add_optional_symbol<types::leverage_bracket_request_t>  (app, "leverage-bracket",   "Leverage brackets [symbol] (auth)", sel)->group(group);
    add_optional_symbol<types::quantitative_rules_request_t>(app, "quantitative-rules", "Quantitative rules [symbol] (auth)", sel)->group(group);

    // income-history: optional symbol + optional --limit.
    {
        struct opts_t { std::string symbol; std::optional<int> limit; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("income-history", "Income history [symbol] (auth)");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->add_option("limit", opts->limit, "Result limit (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::income_history_request_t req;
                if (!opts->symbol.empty()) req.symbol = opts->symbol;
                req.limit = opts->limit;
                co_return co_await lib::exec_account(c, std::move(req), sink);
            };
        });
    }

    add_noarg<types::account_config_request_t>      (app, "account-config",     "Account config (auth)",         sel)->group(group);
    add_noarg<types::get_multi_assets_mode_request_t>(app, "multi-assets-mode",  "Get multi-assets mode (auth)",  sel)->group(group);
    add_noarg<types::get_position_mode_request_t>   (app, "position-mode",      "Get position mode (auth)",      sel)->group(group);
    add_noarg<types::rate_limit_order_request_t>    (app, "rate-limit-order",   "Rate limit order count (auth)", sel)->group(group);
    add_noarg<types::get_bnb_burn_request_t>        (app, "bnb-burn",           "Get BNB burn status (auth)",    sel)->group(group);

    // commission-rate: required <symbol>.
    {
        auto opts = std::make_shared<lib::symbol_opts>();
        auto* sub = app.add_subcommand("commission-rate", "Commission rate (auth)");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                co_return co_await lib::exec_account(
                    c, lib::make_symbol_request<types::commission_rate_request_t>(*opts), sink);
            };
        });
    }

    // toggle-bnb-burn: required <true|false>.
    {
        struct opts_t { std::string enable; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("toggle-bnb-burn", "Toggle BNB burn (auth)");
        sub->group(group);
        sub->add_option("enable", opts->enable, "true|false")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::toggle_bnb_burn_request_t req;
                req.feeBurn = opts->enable;
                co_return co_await lib::exec_account(c, std::move(req), sink);
            };
        });
    }

    // pm-account-info: required <asset>.
    {
        struct opts_t { std::string asset; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("pm-account-info", "Portfolio margin info <asset> (auth)");
        sub->group(group);
        sub->add_option("asset", opts->asset, "Asset (e.g. BTC, USDT)")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::pm_account_info_request_t req;
                req.asset = opts->asset;
                co_return co_await lib::exec_account(c, std::move(req), sink);
            };
        });
    }

    // download-id / download-link families.
    add_download_id_sub  <types::download_id_transaction_request_t>  (app, "download-id-transaction",  "Download ID transaction (auth)",  sel)->group(group);
    add_download_link_sub<types::download_link_transaction_request_t>(app, "download-link-transaction","Download link transaction (auth)",sel)->group(group);
    add_download_id_sub  <types::download_id_order_request_t>        (app, "download-id-order",        "Download ID order (auth)",        sel)->group(group);
    add_download_link_sub<types::download_link_order_request_t>      (app, "download-link-order",      "Download link order (auth)",      sel)->group(group);
    add_download_id_sub  <types::download_id_trade_request_t>        (app, "download-id-trade",        "Download ID trade (auth)",        sel)->group(group);
    add_download_link_sub<types::download_link_trade_request_t>      (app, "download-link-trade",      "Download link trade (auth)",      sel)->group(group);
}

} // namespace demo
