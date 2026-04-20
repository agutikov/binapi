// SPDX-License-Identifier: Apache-2.0
//
// Market data command registry — public, no authentication required.

#include "commands.hpp"

#include <binapi2/demo_commands/enum_parse.hpp>
#include <binapi2/fapi/types/market_data.hpp>

namespace demo_ui::rest {

namespace {

// ── No-args ──────────────────────────────────────────────────────

void cmd_ping             (const cmd_ctx& c) { run_market(c, types::ping_request_t{}); }
void cmd_server_time      (const cmd_ctx& c) { run_market(c, types::server_time_request_t{}); }
void cmd_book_tickers     (const cmd_ctx& c) { run_market(c, types::book_tickers_request_t{}); }
void cmd_price_tickers    (const cmd_ctx& c) { run_market(c, types::price_tickers_request_t{}); }
void cmd_price_tickers_v2 (const cmd_ctx& c) { run_market(c, types::price_tickers_v2_request_t{}); }
void cmd_ticker_24hrs     (const cmd_ctx& c) { run_market(c, types::ticker_24hrs_request_t{}); }
void cmd_mark_prices      (const cmd_ctx& c) { run_market(c, types::mark_prices_request_t{}); }
void cmd_funding_rate_info(const cmd_ctx& c) { run_market(c, types::funding_rate_info_request_t{}); }
void cmd_trading_schedule (const cmd_ctx& c) { run_market(c, types::trading_schedule_request_t{}); }

// ── Symbol required ──────────────────────────────────────────────

template<typename Req>
void run_symbol_md(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    run_market(c, std::move(req));
}
void cmd_book_ticker       (const cmd_ctx& c) { run_symbol_md<types::book_ticker_request_t>(c); }
void cmd_price_ticker      (const cmd_ctx& c) { run_symbol_md<types::price_ticker_request_t>(c); }
void cmd_price_ticker_v2   (const cmd_ctx& c) { run_symbol_md<types::price_ticker_v2_request_t>(c); }
void cmd_ticker_24hr       (const cmd_ctx& c) { run_symbol_md<types::ticker_24hr_request_t>(c); }
void cmd_mark_price        (const cmd_ctx& c) { run_symbol_md<types::mark_price_request_t>(c); }
void cmd_open_interest     (const cmd_ctx& c) { run_symbol_md<types::open_interest_request_t>(c); }
void cmd_index_constituents(const cmd_ctx& c) { run_symbol_md<types::index_constituents_request_t>(c); }

// ── Optional symbol ──────────────────────────────────────────────

template<typename Req>
void run_symbol_opt_md(const cmd_ctx& c)
{
    Req req;
    if (!c.form.symbol.empty()) req.symbol = c.form.symbol;
    run_market(c, std::move(req));
}
void cmd_exchange_info       (const cmd_ctx& c) { run_symbol_opt_md<types::exchange_info_request_t>(c); }
void cmd_composite_index_info(const cmd_ctx& c) { run_symbol_opt_md<types::composite_index_info_request_t>(c); }
void cmd_asset_index         (const cmd_ctx& c) { run_symbol_opt_md<types::asset_index_request_t>(c); }
void cmd_adl_risk            (const cmd_ctx& c) { run_symbol_opt_md<types::adl_risk_request_t>(c); }

void cmd_insurance_fund(const cmd_ctx& c)
{
    if (c.form.symbol.empty()) {
        run_market(c, types::insurance_funds_request_t{});
    } else {
        types::insurance_fund_request_t req;
        req.symbol = c.form.symbol;
        run_market(c, std::move(req));
    }
}

// ── Symbol + optional limit ──────────────────────────────────────

template<typename Req>
void run_symbol_limit_md(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    req.limit = parse_optional_int(c.form.limit);
    run_market(c, std::move(req));
}
void cmd_order_book        (const cmd_ctx& c) { run_symbol_limit_md<types::order_book_request_t>(c); }
void cmd_recent_trades     (const cmd_ctx& c) { run_symbol_limit_md<types::recent_trades_request_t>(c); }
void cmd_aggregate_trades  (const cmd_ctx& c) { run_symbol_limit_md<types::aggregate_trades_request_t>(c); }
void cmd_historical_trades (const cmd_ctx& c) { run_symbol_limit_md<types::historical_trades_request_t>(c); }
void cmd_funding_rate      (const cmd_ctx& c) { run_symbol_limit_md<types::funding_rate_history_request_t>(c); }
void cmd_rpi_depth         (const cmd_ctx& c) { run_symbol_limit_md<types::rpi_depth_request_t>(c); }

// ── Pair ─────────────────────────────────────────────────────────

void cmd_delivery_price(const cmd_ctx& c)
{
    types::delivery_price_request_t req;
    req.pair = c.form.pair;
    run_market(c, std::move(req));
}

// ── Klines (symbol + interval + optional limit) ──────────────────

template<typename Req>
void run_kline_md(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    req.interval = lib::parse_enum<types::kline_interval_t>(c.form.interval);
    req.limit = parse_optional_int(c.form.limit);
    run_market(c, std::move(req));
}
void cmd_klines               (const cmd_ctx& c) { run_kline_md<types::klines_request_t>(c); }
void cmd_mark_price_klines    (const cmd_ctx& c) { run_kline_md<types::mark_price_klines_request_t>(c); }
void cmd_premium_index_klines (const cmd_ctx& c) { run_kline_md<types::premium_index_klines_request_t>(c); }

template<typename Req>
void run_pair_kline_md(const cmd_ctx& c)
{
    Req req;
    req.pair = c.form.pair;
    req.interval = lib::parse_enum<types::kline_interval_t>(c.form.interval);
    req.limit = parse_optional_int(c.form.limit);
    run_market(c, std::move(req));
}
void cmd_continuous_kline (const cmd_ctx& c) { run_pair_kline_md<types::continuous_kline_request_t>(c); }
void cmd_index_price_kline(const cmd_ctx& c) { run_pair_kline_md<types::index_price_kline_request_t>(c); }

// ── Analytics (symbol + kline_interval period + optional limit) ──

template<typename Req>
void run_analytics_md(const cmd_ctx& c)
{
    Req req;
    req.symbol = c.form.symbol;
    req.period = lib::parse_enum<types::kline_interval_t>(c.form.period);
    req.limit = parse_optional_int(c.form.limit);
    run_market(c, std::move(req));
}
void cmd_oi_stats         (const cmd_ctx& c) { run_analytics_md<types::open_interest_statistics_request_t>(c); }
void cmd_top_ls_account   (const cmd_ctx& c) { run_analytics_md<types::top_long_short_account_ratio_request_t>(c); }
void cmd_top_ls_trader    (const cmd_ctx& c) { run_analytics_md<types::top_trader_long_short_ratio_request_t>(c); }
void cmd_long_short_ratio (const cmd_ctx& c) { run_analytics_md<types::long_short_ratio_request_t>(c); }
void cmd_taker_volume     (const cmd_ctx& c) { run_analytics_md<types::taker_buy_sell_volume_request_t>(c); }

// ── Basis (pair + futures_data_period_t + optional limit) ────────

void cmd_basis(const cmd_ctx& c)
{
    types::basis_request_t req;
    req.pair = c.form.pair;
    req.period = lib::parse_enum<types::futures_data_period_t>(c.form.period);
    req.limit = parse_optional_int(c.form.limit);
    run_market(c, std::move(req));
}

constexpr rest_command entries[] = {
    // No-args
    { "ping",               "Test API connectivity",     command_group::market_data, form_kind::no_args,      &cmd_ping },
    { "server-time",        "Server time",               command_group::market_data, form_kind::no_args,      &cmd_server_time },
    { "book-tickers",       "All book tickers",          command_group::market_data, form_kind::no_args,      &cmd_book_tickers },
    { "price-tickers",      "All price tickers",         command_group::market_data, form_kind::no_args,      &cmd_price_tickers },
    { "price-tickers-v2",   "All price tickers v2",      command_group::market_data, form_kind::no_args,      &cmd_price_tickers_v2 },
    { "ticker-24hrs",       "All 24hr tickers",          command_group::market_data, form_kind::no_args,      &cmd_ticker_24hrs },
    { "mark-prices",        "All mark prices",           command_group::market_data, form_kind::no_args,      &cmd_mark_prices },
    { "funding-rate-info",  "Funding rate info (all)",   command_group::market_data, form_kind::no_args,      &cmd_funding_rate_info },
    { "trading-schedule",   "Trading schedule",          command_group::market_data, form_kind::no_args,      &cmd_trading_schedule },

    // Symbol required
    { "book-ticker",        "Book ticker",               command_group::market_data, form_kind::symbol,       &cmd_book_ticker },
    { "price-ticker",       "Price ticker",              command_group::market_data, form_kind::symbol,       &cmd_price_ticker },
    { "price-ticker-v2",    "Price ticker v2",           command_group::market_data, form_kind::symbol,       &cmd_price_ticker_v2 },
    { "ticker-24hr",        "24hr ticker",               command_group::market_data, form_kind::symbol,       &cmd_ticker_24hr },
    { "mark-price",         "Mark price",                command_group::market_data, form_kind::symbol,       &cmd_mark_price },
    { "open-interest",      "Open interest",             command_group::market_data, form_kind::symbol,       &cmd_open_interest },
    { "index-constituents", "Index constituents",        command_group::market_data, form_kind::symbol,       &cmd_index_constituents },

    // Optional symbol
    { "exchange-info",        "Exchange info [symbol]",        command_group::market_data, form_kind::symbol_opt,   &cmd_exchange_info },
    { "composite-index-info", "Composite index info [symbol]", command_group::market_data, form_kind::symbol_opt,   &cmd_composite_index_info },
    { "asset-index",          "Asset index [symbol]",          command_group::market_data, form_kind::symbol_opt,   &cmd_asset_index },
    { "adl-risk",             "ADL risk [symbol]",             command_group::market_data, form_kind::symbol_opt,   &cmd_adl_risk },
    { "insurance-fund",       "Insurance fund [symbol]",       command_group::market_data, form_kind::symbol_opt,   &cmd_insurance_fund },

    // Symbol + optional limit
    { "order-book",        "Order book",            command_group::market_data, form_kind::symbol_limit, &cmd_order_book },
    { "recent-trades",     "Recent trades",         command_group::market_data, form_kind::symbol_limit, &cmd_recent_trades },
    { "aggregate-trades",  "Aggregate trades",      command_group::market_data, form_kind::symbol_limit, &cmd_aggregate_trades },
    { "historical-trades", "Historical trades",     command_group::market_data, form_kind::symbol_limit, &cmd_historical_trades },
    { "funding-rate",      "Funding rate history",  command_group::market_data, form_kind::symbol_limit, &cmd_funding_rate },
    { "rpi-depth",         "RPI depth",             command_group::market_data, form_kind::symbol_limit, &cmd_rpi_depth },

    // Pair
    { "delivery-price",    "Delivery price",        command_group::market_data, form_kind::pair,         &cmd_delivery_price },

    // Klines
    { "klines",               "Klines",                     command_group::market_data, form_kind::kline,        &cmd_klines },
    { "mark-price-klines",    "Mark price klines",          command_group::market_data, form_kind::kline,        &cmd_mark_price_klines },
    { "premium-index-klines", "Premium index klines",       command_group::market_data, form_kind::kline,        &cmd_premium_index_klines },
    { "continuous-kline",     "Continuous contract kline",  command_group::market_data, form_kind::pair_kline,   &cmd_continuous_kline },
    { "index-price-kline",    "Index price kline",          command_group::market_data, form_kind::pair_kline,   &cmd_index_price_kline },

    // Analytics
    { "open-interest-stats",  "Open interest statistics",   command_group::market_data, form_kind::analytics,    &cmd_oi_stats },
    { "top-ls-account-ratio", "Top L/S account ratio",      command_group::market_data, form_kind::analytics,    &cmd_top_ls_account },
    { "top-ls-trader-ratio",  "Top L/S trader ratio",       command_group::market_data, form_kind::analytics,    &cmd_top_ls_trader },
    { "long-short-ratio",     "Global L/S ratio",           command_group::market_data, form_kind::analytics,    &cmd_long_short_ratio },
    { "taker-volume",         "Taker buy/sell volume",      command_group::market_data, form_kind::analytics,    &cmd_taker_volume },
    { "basis",                "Basis",                      command_group::market_data, form_kind::basis,        &cmd_basis },
};

} // namespace

std::span<const rest_command> market_data_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::rest
