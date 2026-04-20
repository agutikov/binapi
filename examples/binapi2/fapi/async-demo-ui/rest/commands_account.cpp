// SPDX-License-Identifier: Apache-2.0
//
// Account command registry — authenticated REST endpoints.

#include "commands.hpp"

#include <binapi2/fapi/types/account.hpp>

namespace demo_ui::rest {

namespace {

// ── No-args ──────────────────────────────────────────────────────

void cmd_account_info      (const cmd_ctx& c) { run_acct(c, types::account_information_request_t{}); }
void cmd_balances          (const cmd_ctx& c) { run_acct(c, types::balances_request_t{}); }
void cmd_account_config    (const cmd_ctx& c) { run_acct(c, types::account_config_request_t{}); }
void cmd_multi_assets_mode (const cmd_ctx& c) { run_acct(c, types::get_multi_assets_mode_request_t{}); }
void cmd_position_mode     (const cmd_ctx& c) { run_acct(c, types::get_position_mode_request_t{}); }
void cmd_rate_limit_order  (const cmd_ctx& c) { run_acct(c, types::rate_limit_order_request_t{}); }
void cmd_bnb_burn          (const cmd_ctx& c) { run_acct(c, types::get_bnb_burn_request_t{}); }

// ── Optional symbol ──────────────────────────────────────────────

template<typename Req>
void run_symbol_opt_acct(const cmd_ctx& c)
{
    Req req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    run_acct(c, std::move(req));
}
void cmd_position_risk      (const cmd_ctx& c) { run_symbol_opt_acct<types::position_risk_request_t>(c); }
void cmd_symbol_config      (const cmd_ctx& c) { run_symbol_opt_acct<types::symbol_config_request_t>(c); }
void cmd_leverage_bracket   (const cmd_ctx& c) { run_symbol_opt_acct<types::leverage_bracket_request_t>(c); }
void cmd_quantitative_rules (const cmd_ctx& c) { run_symbol_opt_acct<types::quantitative_rules_request_t>(c); }

// ── Symbol required ──────────────────────────────────────────────

void cmd_commission_rate(const cmd_ctx& c)
{
    types::commission_rate_request_t req;
    req.symbol = c.form.symbol;
    run_acct(c, std::move(req));
}

// ── Optional symbol + optional limit ─────────────────────────────

void cmd_income_history(const cmd_ctx& c)
{
    types::income_history_request_t req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    req.limit = parse_optional_int(c.form.limit);
    run_acct(c, std::move(req));
}

// ── Single-string boolean (toggle-bnb-burn) ──────────────────────

void cmd_toggle_bnb_burn(const cmd_ctx& c)
{
    types::toggle_bnb_burn_request_t req;
    req.feeBurn = c.form.bool_flag;
    run_acct(c, std::move(req));
}

// ── Asset (pm-account-info) ──────────────────────────────────────

void cmd_pm_account_info(const cmd_ctx& c)
{
    types::pm_account_info_request_t req;
    req.asset = c.form.asset;
    run_acct(c, std::move(req));
}

// ── Download-id range (start+end time) ───────────────────────────

template<typename Req>
void run_download_id(const cmd_ctx& c)
{
    Req req;
    req.startTime = types::timestamp_ms_t{ parse_u64(c.form.start_time) };
    req.endTime   = types::timestamp_ms_t{ parse_u64(c.form.end_time) };
    run_acct(c, std::move(req));
}
void cmd_dl_id_transaction(const cmd_ctx& c) { run_download_id<types::download_id_transaction_request_t>(c); }
void cmd_dl_id_order      (const cmd_ctx& c) { run_download_id<types::download_id_order_request_t>(c); }
void cmd_dl_id_trade      (const cmd_ctx& c) { run_download_id<types::download_id_trade_request_t>(c); }

// ── Download link (download_id) ──────────────────────────────────

template<typename Req>
void run_download_link(const cmd_ctx& c)
{
    Req req;
    req.downloadId = c.form.download_id;
    run_acct(c, std::move(req));
}
void cmd_dl_link_transaction(const cmd_ctx& c) { run_download_link<types::download_link_transaction_request_t>(c); }
void cmd_dl_link_order      (const cmd_ctx& c) { run_download_link<types::download_link_order_request_t>(c); }
void cmd_dl_link_trade      (const cmd_ctx& c) { run_download_link<types::download_link_trade_request_t>(c); }

constexpr rest_command entries[] = {
    // Queries (no args)
    { "account-info",      "Account information",            command_group::account, form_kind::no_args,          &cmd_account_info },
    { "balances",          "Account balances",               command_group::account, form_kind::no_args,          &cmd_balances },
    { "account-config",    "Account config",                 command_group::account, form_kind::no_args,          &cmd_account_config },
    { "multi-assets-mode", "Get multi-assets mode",          command_group::account, form_kind::no_args,          &cmd_multi_assets_mode },
    { "position-mode",     "Get position mode",              command_group::account, form_kind::no_args,          &cmd_position_mode },
    { "rate-limit-order",  "Rate limit order count",         command_group::account, form_kind::no_args,          &cmd_rate_limit_order },
    { "bnb-burn",          "Get BNB burn status",            command_group::account, form_kind::no_args,          &cmd_bnb_burn },

    // Optional symbol
    { "position-risk",      "Position risk [symbol]",        command_group::account, form_kind::symbol_opt,       &cmd_position_risk },
    { "symbol-config",      "Symbol config [symbol]",        command_group::account, form_kind::symbol_opt,       &cmd_symbol_config },
    { "leverage-bracket",   "Leverage brackets [symbol]",    command_group::account, form_kind::symbol_opt,       &cmd_leverage_bracket },
    { "quantitative-rules", "Quantitative rules [symbol]",   command_group::account, form_kind::symbol_opt,       &cmd_quantitative_rules },

    // Symbol-required
    { "commission-rate", "Commission rate",                  command_group::account, form_kind::symbol,           &cmd_commission_rate },

    // Optional-symbol + optional-limit
    { "income-history", "Income history [symbol] [limit]",   command_group::account, form_kind::symbol_opt_limit, &cmd_income_history },

    // Config changes
    { "toggle-bnb-burn", "Toggle BNB burn",                  command_group::account, form_kind::bool_toggle,      &cmd_toggle_bnb_burn },

    // Portfolio margin
    { "pm-account-info", "Portfolio margin info <asset>",    command_group::account, form_kind::asset_form,       &cmd_pm_account_info },

    // Download IDs
    { "download-id-transaction", "Download ID transaction",  command_group::account, form_kind::download_id_range, &cmd_dl_id_transaction },
    { "download-id-order",       "Download ID order",        command_group::account, form_kind::download_id_range, &cmd_dl_id_order },
    { "download-id-trade",       "Download ID trade",        command_group::account, form_kind::download_id_range, &cmd_dl_id_trade },

    // Download links
    { "download-link-transaction", "Download link transaction", command_group::account, form_kind::download_link, &cmd_dl_link_transaction },
    { "download-link-order",       "Download link order",       command_group::account, form_kind::download_link, &cmd_dl_link_order },
    { "download-link-trade",       "Download link trade",       command_group::account, form_kind::download_link, &cmd_dl_link_trade },
};

} // namespace

std::span<const rest_command> account_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::rest
