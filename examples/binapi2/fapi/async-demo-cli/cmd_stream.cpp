// SPDX-License-Identifier: Apache-2.0
//
// Market data stream commands — async WebSocket streams using cobalt generators.

#include "cmd_stream.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

namespace {

struct symbol_opts { std::string symbol; };

template<typename Subscription>
CLI::App* add_noarg_stream(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
            co_return co_await exec_stream(c, Subscription{});
        };
    });
    return sub;
}

template<typename Subscription>
CLI::App* add_symbol_stream(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto opts = std::make_shared<symbol_opts>();
    auto* sub = parent.add_subcommand(name, desc);
    sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
    sub->callback([&sel, opts] {
        sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
            Subscription s;
            s.symbol = opts->symbol;
            co_return co_await exec_stream(c, s);
        };
    });
    return sub;
}

} // namespace

void register_cmd_stream(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "Market Streams";

    // ── single-symbol streams ─────────────────────────────────────────

    add_symbol_stream<types::book_ticker_subscription>        (app, "stream-book-ticker",     "Book ticker stream",       sel)->group(group);
    add_symbol_stream<types::mark_price_subscription>         (app, "stream-mark-price",      "Mark price stream",        sel)->group(group);
    add_symbol_stream<types::ticker_subscription>             (app, "stream-ticker",          "24hr ticker stream",       sel)->group(group);
    add_symbol_stream<types::liquidation_order_subscription>  (app, "stream-liquidation",     "Liquidation stream",       sel)->group(group);
    add_symbol_stream<types::aggregate_trade_subscription>    (app, "stream-aggregate-trade", "Aggregate trade stream",   sel)->group(group);
    add_symbol_stream<types::mini_ticker_subscription>        (app, "stream-mini-ticker",     "Mini ticker stream",       sel)->group(group);
    add_symbol_stream<types::composite_index_subscription>    (app, "stream-composite-index", "Composite index stream",   sel)->group(group);
    add_symbol_stream<types::asset_index_subscription>        (app, "stream-asset-index",     "Asset index stream",       sel)->group(group);
    add_symbol_stream<types::rpi_diff_book_depth_subscription>(app, "stream-rpi-diff-depth",  "RPI diff depth stream",    sel)->group(group);

    // ── klines (symbol + interval) ────────────────────────────────────

    {
        struct opts_t { std::string symbol, interval; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("stream-kline", "Kline stream");
        sub->group(group);
        sub->add_option("symbol",   opts->symbol,   "Trading symbol")->required();
        sub->add_option("interval", opts->interval, "Kline interval")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
                types::kline_subscription s;
                s.symbol = opts->symbol;
                s.interval = parse_enum<types::kline_interval_t>(opts->interval);
                co_return co_await exec_stream(c, s);
            };
        });
    }

    {
        struct opts_t { std::string pair, interval; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("stream-continuous-kline", "Continuous kline stream");
        sub->group(group);
        sub->add_option("pair",     opts->pair,     "Pair")->required();
        sub->add_option("interval", opts->interval, "Kline interval")->required();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
                types::continuous_contract_kline_subscription s;
                s.pair = opts->pair;
                s.interval = parse_enum<types::kline_interval_t>(opts->interval);
                co_return co_await exec_stream(c, s);
            };
        });
    }

    // ── depth ─────────────────────────────────────────────────────────

    {
        struct opts_t { std::string symbol; int levels = 10; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("stream-depth", "Partial book depth stream");
        sub->group(group);
        sub->add_option("symbol", opts->symbol, "Trading symbol")->required();
        sub->add_option("-l,--levels", opts->levels, "Depth levels (5|10|20)")
            ->check(CLI::IsMember({ 5, 10, 20 }))->capture_default_str();
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
                types::partial_book_depth_subscription s;
                s.symbol = opts->symbol;
                s.levels = opts->levels;
                co_return co_await exec_stream(c, s);
            };
        });
    }

    {
        struct opts_t { std::string symbol, speed; };
        auto opts = std::make_shared<opts_t>();
        auto* sub = app.add_subcommand("stream-diff-depth", "Diff depth stream");
        sub->group(group);
        sub->add_option("symbol",    opts->symbol, "Trading symbol")->required();
        sub->add_option("-s,--speed", opts->speed,  "Update speed (100ms|250ms|500ms)");
        sub->callback([&sel, opts] {
            sel.factory = [opts](binapi2::futures_usdm_api& c) -> boost::cobalt::task<int> {
                types::diff_book_depth_subscription s;
                s.symbol = opts->symbol;
                s.speed = opts->speed;
                co_return co_await exec_stream(c, s);
            };
        });
    }

    // ── all-market streams (no args) ──────────────────────────────────

    add_noarg_stream<types::all_book_ticker_subscription>            (app, "stream-all-book-tickers", "All book tickers stream",  sel)->group(group);
    add_noarg_stream<types::all_market_ticker_subscription>          (app, "stream-all-tickers",      "All 24hr tickers stream",  sel)->group(group);
    add_noarg_stream<types::all_market_mini_ticker_subscription>     (app, "stream-all-mini-tickers", "All mini tickers stream",  sel)->group(group);
    add_noarg_stream<types::all_market_liquidation_order_subscription>(app,"stream-all-liquidations","All liquidations stream",  sel)->group(group);
    add_noarg_stream<types::all_market_mark_price_subscription>      (app, "stream-all-mark-prices",  "All mark prices stream",   sel)->group(group);
    add_noarg_stream<types::all_asset_index_subscription>            (app, "stream-all-asset-index",  "All asset index stream",   sel)->group(group);
    add_noarg_stream<types::contract_info_subscription>              (app, "stream-contract-info",    "Contract info stream",     sel)->group(group);
    add_noarg_stream<types::trading_session_subscription>            (app, "stream-trading-session",  "Trading session stream",   sel)->group(group);
}

} // namespace demo
