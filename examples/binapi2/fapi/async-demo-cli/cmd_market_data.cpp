// SPDX-License-Identifier: Apache-2.0
//
// Market data commands — public REST endpoints, no authentication required.

#include "cmd_market_data.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

// ── thin wrappers around the shared library for this file's shapes ──

/// Zero-argument market-data subcommand.
template<typename Request>
CLI::App* add_noarg(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_market_data(c, Request{}, sink);
        };
    });
    return sub;
}

/// `<symbol>` (required) market-data subcommand.
template<typename Request>
CLI::App* add_symbol(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_market_data(
                c, lib::make_symbol_request<Request>(*opts), sink);
        };
    });
    return sub;
}

/// `[symbol]` (optional, positional) market-data subcommand.
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
            co_return co_await lib::exec_market_data(c, std::move(req), sink);
        };
    });
    return sub;
}

/// `<symbol> [--limit N]` market-data subcommand.
template<typename Request>
CLI::App* add_symbol_limit(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<lib::symbol_limit_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->add_option("-l,--limit", opts->limit, "Result limit");
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c,
                             lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_market_data(
                c, lib::make_symbol_limit_request<Request>(*opts), sink);
        };
    });
    return sub;
}

} // namespace

void register_cmd_market_data(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Market Data";

    // ── custom-body commands that don't fit the generic exec path ─────

    auto* ping = app.add_subcommand("ping", "Test API connectivity");
    ping->group(group);
    ping->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            auto rest = co_await c.create_rest_client();
            if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
            auto r = co_await (*rest)->market_data.async_execute(types::ping_request_t{});
            if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
            sink.on_info("pong");
            if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                sink.on_response_json(*j);
            sink.on_done(0);
            co_return 0;
        };
    });

    auto* time_cmd = app.add_subcommand("time", "Get server time");
    time_cmd->group(group);
    time_cmd->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            auto rest = co_await c.create_rest_client();
            if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
            auto r = co_await (*rest)->market_data.async_execute(types::server_time_request_t{});
            if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }
            sink.on_info("server time: " + std::to_string(r->serverTime.value));
            if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                sink.on_response_json(*j);
            sink.on_done(0);
            co_return 0;
        };
    });

    // order-book has a bespoke non-JSON summary when verbosity is off.
    {
        struct opts_t { std::string symbol; std::optional<int> limit; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("order-book", "Order book <symbol> [--limit L]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("-l,--limit", opts->limit, "Depth limit");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::order_book_request_t req;
                req.symbol = opts->symbol;
                req.limit = opts->limit;

                auto rest = co_await c.create_rest_client();
                if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
                auto r = co_await (*rest)->market_data.async_execute(req);
                if (!r) { sink.on_error(r.err); sink.on_done(1); co_return 1; }

                sink.on_info("order book: bids=" + std::to_string(r->bids.size())
                             + " asks=" + std::to_string(r->asks.size())
                             + " lastUpdateId=" + std::to_string(r->lastUpdateId));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(*r); j)
                    sink.on_response_json(*j);

                // Fallback short summary when the JSON channel is suppressed
                // (i.e. verbosity == 0 for the CLI). Routes through the
                // prefix-free `"out"` logger.
                if (verbosity < 1) {
                    int n = std::min(5, static_cast<int>(r->bids.size()));
                    for (int i = 0; i < n; ++i)
                        out("  bid {} x {}", r->bids[static_cast<std::size_t>(i)].price,
                            r->bids[static_cast<std::size_t>(i)].quantity);
                    n = std::min(5, static_cast<int>(r->asks.size()));
                    for (int i = 0; i < n; ++i)
                        out("  ask {} x {}", r->asks[static_cast<std::size_t>(i)].price,
                            r->asks[static_cast<std::size_t>(i)].quantity);
                }
                sink.on_done(0);
                co_return 0;
            };
        });
    }

    // ── symbol + optional limit ───────────────────────────────────────

    add_symbol_limit<types::recent_trades_request_t>    (app, "recent-trades",     "Recent trades", sel)->group(group);
    add_symbol_limit<types::aggregate_trades_request_t> (app, "aggregate-trades",  "Aggregate trades", sel)->group(group);
    add_symbol_limit<types::historical_trades_request_t>(app, "historical-trades", "Historical trades", sel)->group(group);
    add_symbol_limit<types::funding_rate_history_request_t>(app, "funding-rate",   "Funding rate history", sel)->group(group);
    add_symbol_limit<types::rpi_depth_request_t>        (app, "rpi-depth",         "RPI depth", sel)->group(group);

    // ── symbol only ───────────────────────────────────────────────────

    add_symbol<types::book_ticker_request_t>       (app, "book-ticker",        "Book ticker", sel)->group(group);
    add_symbol<types::price_ticker_request_t>      (app, "price-ticker",       "Price ticker", sel)->group(group);
    add_symbol<types::price_ticker_v2_request_t>   (app, "price-ticker-v2",    "Price ticker v2", sel)->group(group);
    add_symbol<types::ticker_24hr_request_t>       (app, "ticker-24hr",        "24hr ticker", sel)->group(group);
    add_symbol<types::mark_price_request_t>        (app, "mark-price",         "Mark price", sel)->group(group);
    add_symbol<types::open_interest_request_t>     (app, "open-interest",      "Open interest", sel)->group(group);
    add_symbol<types::index_constituents_request_t>(app, "index-constituents", "Index constituents", sel)->group(group);

    // ── zero-arg aggregates ───────────────────────────────────────────

    add_noarg<types::book_tickers_request_t>     (app, "book-tickers",       "All book tickers", sel)->group(group);
    add_noarg<types::price_tickers_request_t>    (app, "price-tickers",      "All price tickers", sel)->group(group);
    add_noarg<types::price_tickers_v2_request_t> (app, "price-tickers-v2",   "All price tickers v2", sel)->group(group);
    add_noarg<types::ticker_24hrs_request_t>     (app, "ticker-24hrs",       "All 24hr tickers", sel)->group(group);
    add_noarg<types::mark_prices_request_t>      (app, "mark-prices",        "All mark prices", sel)->group(group);
    add_noarg<types::funding_rate_info_request_t>(app, "funding-rate-info",  "Funding rate info (all)", sel)->group(group);
    add_noarg<types::trading_schedule_request_t> (app, "trading-schedule",   "Trading schedule", sel)->group(group);

    // ── klines family (shared builder in common.hpp → library) ────────

    add_kline_sub     <types::klines_request_t>              (app, "klines",               "Klines", sel)->group(group);
    add_pair_kline_sub<types::continuous_kline_request_t>    (app, "continuous-kline",     "Continuous kline", sel)->group(group);
    add_pair_kline_sub<types::index_price_kline_request_t>   (app, "index-price-kline",    "Index price kline", sel)->group(group);
    add_kline_sub     <types::mark_price_klines_request_t>   (app, "mark-price-klines",    "Mark price klines", sel)->group(group);
    add_kline_sub     <types::premium_index_klines_request_t>(app, "premium-index-klines", "Premium index klines", sel)->group(group);

    // ── futures analytics family ──────────────────────────────────────

    add_analytics_sub<types::open_interest_statistics_request_t>    (app, "open-interest-stats",  "Open interest stats", sel)->group(group);
    add_analytics_sub<types::top_long_short_account_ratio_request_t>(app, "top-ls-account-ratio", "Top L/S account ratio", sel)->group(group);
    add_analytics_sub<types::top_trader_long_short_ratio_request_t> (app, "top-ls-trader-ratio",  "Top L/S trader ratio", sel)->group(group);
    add_analytics_sub<types::long_short_ratio_request_t>            (app, "long-short-ratio",    "Global L/S ratio", sel)->group(group);
    add_analytics_sub<types::taker_buy_sell_volume_request_t>       (app, "taker-volume",        "Taker buy/sell volume", sel)->group(group);

    // basis: <pair> <period> [--limit N] — one-off shape, inline opts.
    {
        struct opts_t { std::string pair, period; std::optional<int> limit; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("basis", "Basis");
        sub->group(group);
        sub->add_option("pair",   opts->pair,   "Pair")->required();
        sub->add_option("period", opts->period, "Period (5m,15m,…)")->required();
        sub->add_option("-l,--limit", opts->limit, "Result limit");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::basis_request_t req;
                req.pair = opts->pair;
                req.period = lib::parse_enum<types::futures_data_period_t>(opts->period);
                req.limit = opts->limit;
                co_return co_await lib::exec_market_data(c, std::move(req), sink);
            };
        });
    }

    // ── miscellaneous ─────────────────────────────────────────────────

    // delivery-price: <pair>
    {
        struct opts_t { std::string pair; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("delivery-price", "Delivery price");
        sub->group(group);
        sub->add_option("pair", opts->pair, "Pair")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                types::delivery_price_request_t req;
                req.pair = opts->pair;
                co_return co_await lib::exec_market_data(c, std::move(req), sink);
            };
        });
    }

    add_optional_symbol<types::exchange_info_request_t>       (app, "exchange-info",       "Exchange info [symbol]", sel)->group(group);
    add_optional_symbol<types::composite_index_info_request_t>(app, "composite-index-info","Composite index info [symbol]", sel)->group(group);
    add_optional_symbol<types::asset_index_request_t>         (app, "asset-index",         "Asset index [symbol]", sel)->group(group);
    add_optional_symbol<types::adl_risk_request_t>            (app, "adl-risk",            "ADL risk [symbol]", sel)->group(group);

    // insurance-fund: [symbol] — omit → call the plural endpoint.
    {
        struct opts_t { std::string symbol; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("insurance-fund", "Insurance fund [symbol]");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol (optional)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c,
                                 lib::result_sink& sink) -> boost::cobalt::task<int> {
                if (!opts->symbol.empty()) {
                    types::insurance_fund_request_t req;
                    req.symbol = opts->symbol;
                    co_return co_await lib::exec_market_data(c, std::move(req), sink);
                }
                co_return co_await lib::exec_market_data(
                    c, types::insurance_funds_request_t{}, sink);
            };
        });
    }
}

} // namespace demo
