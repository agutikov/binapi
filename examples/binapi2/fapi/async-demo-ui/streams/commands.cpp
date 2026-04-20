// SPDX-License-Identifier: Apache-2.0
//
// Market-stream command registry. Each entry has a `start(ctx)` free
// function that spawns the subscription coroutine; stop is handled
// uniformly by flipping `ctx.cap->stop` which the stream loop checks.

#include "commands.hpp"

#include "stream_capture_sink.hpp"

#include <binapi2/demo_commands/enum_parse.hpp>
#include <binapi2/demo_commands/exec.hpp>
#include <binapi2/fapi/types/enums.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>

#include <spdlog/spdlog.h>

#include <charconv>

namespace demo_ui::streams {

namespace lib   = binapi2::demo;
namespace types = binapi2::fapi::types;

// ── Form-field visibility predicates ────────────────────────────────

bool needs_symbol(form_kind k)
{
    return k == form_kind::symbol || k == form_kind::kline
        || k == form_kind::levels || k == form_kind::speed;
}
bool needs_pair    (form_kind k) { return k == form_kind::pair_kline; }
bool needs_interval(form_kind k) { return k == form_kind::kline || k == form_kind::pair_kline; }
bool needs_levels  (form_kind k) { return k == form_kind::levels; }
bool needs_speed   (form_kind k) { return k == form_kind::speed; }

namespace {

// ── Shared start helper ─────────────────────────────────────────────
//
// Template free-function coroutine — one instantiation per Subscription
// type. Never wrapped in a local lambda so captures live in the frame,
// per `feedback_cobalt_lambda_lifetime.md`.

template<typename Subscription>
boost::cobalt::task<void>
run_stream_sub(worker& wrk,
               std::shared_ptr<stream_capture_sink> sink,
               std::shared_ptr<stream_capture> cap,
               Subscription sub)
{
    co_await lib::exec_stream(wrk.api(), std::move(sub), *sink, &cap->stop);
}

/// Reset the capture, install a fresh sink, spawn the stream coroutine.
template<typename Subscription>
void spawn_stream(const start_ctx& c, Subscription sub)
{
    c.cap->stop = false;
    c.cap->total_events = 0;
    c.cap->errors = 0;
    {
        std::lock_guard lk(c.cap->mtx);
        c.cap->ring.clear();
        c.cap->error.clear();
    }

    auto sink = std::make_shared<stream_capture_sink>(c.cap, c.wrk, c.state);
    boost::cobalt::spawn(
        c.wrk.io().get_executor(),
        run_stream_sub<Subscription>(c.wrk, std::move(sink), c.cap, std::move(sub)),
        boost::asio::use_future);
}

// ── No-args streams ─────────────────────────────────────────────────

template<typename Sub>
void start_no_args(const start_ctx& c) { spawn_stream(c, Sub{}); }

void start_all_book_tickers  (const start_ctx& c) { start_no_args<types::all_book_ticker_subscription>(c); }
void start_all_tickers       (const start_ctx& c) { start_no_args<types::all_market_ticker_subscription>(c); }
void start_all_mini_tickers  (const start_ctx& c) { start_no_args<types::all_market_mini_ticker_subscription>(c); }
void start_all_liquidations  (const start_ctx& c) { start_no_args<types::all_market_liquidation_order_subscription>(c); }
void start_all_mark_prices   (const start_ctx& c) { start_no_args<types::all_market_mark_price_subscription>(c); }
void start_all_asset_index   (const start_ctx& c) { start_no_args<types::all_asset_index_subscription>(c); }
void start_contract_info     (const start_ctx& c) { start_no_args<types::contract_info_subscription>(c); }
void start_trading_session   (const start_ctx& c) { start_no_args<types::trading_session_subscription>(c); }

// ── Symbol-only streams ─────────────────────────────────────────────

template<typename Sub>
void start_symbol(const start_ctx& c)
{
    Sub s;
    s.symbol = c.form.symbol;
    spawn_stream(c, std::move(s));
}

void start_book_ticker         (const start_ctx& c) { start_symbol<types::book_ticker_subscription>(c); }
void start_agg_trade           (const start_ctx& c) { start_symbol<types::aggregate_trade_subscription>(c); }
void start_mark_price          (const start_ctx& c) { start_symbol<types::mark_price_subscription>(c); }
void start_ticker              (const start_ctx& c) { start_symbol<types::ticker_subscription>(c); }
void start_mini_ticker         (const start_ctx& c) { start_symbol<types::mini_ticker_subscription>(c); }
void start_liquidation         (const start_ctx& c) { start_symbol<types::liquidation_order_subscription>(c); }
void start_composite_index     (const start_ctx& c) { start_symbol<types::composite_index_subscription>(c); }
void start_asset_index         (const start_ctx& c) { start_symbol<types::asset_index_subscription>(c); }
void start_rpi_diff_depth      (const start_ctx& c) { start_symbol<types::rpi_diff_book_depth_subscription>(c); }

// ── Kline / continuous-kline ────────────────────────────────────────

void start_kline(const start_ctx& c)
{
    types::kline_subscription s;
    s.symbol = c.form.symbol;
    s.interval = lib::parse_enum<types::kline_interval_t>(c.form.interval);
    spawn_stream(c, std::move(s));
}

void start_continuous_kline(const start_ctx& c)
{
    types::continuous_contract_kline_subscription s;
    s.pair = c.form.pair;
    s.interval = lib::parse_enum<types::kline_interval_t>(c.form.interval);
    spawn_stream(c, std::move(s));
}

// ── Partial book depth (symbol + levels) ────────────────────────────

void start_partial_depth(const start_ctx& c)
{
    int lv = 10;
    std::from_chars(c.form.levels.data(),
                    c.form.levels.data() + c.form.levels.size(),
                    lv);
    if (lv != 5 && lv != 10 && lv != 20) lv = 10;

    types::partial_book_depth_subscription s;
    s.symbol = c.form.symbol;
    s.levels = lv;
    spawn_stream(c, std::move(s));
}

// ── Diff book depth (symbol + speed) ────────────────────────────────

void start_diff_depth(const start_ctx& c)
{
    types::diff_book_depth_subscription s;
    s.symbol = c.form.symbol;
    if (!c.form.speed.empty()) s.speed = c.form.speed;
    spawn_stream(c, std::move(s));
}

// ── Registry table ──────────────────────────────────────────────────

constexpr stream_command entries[] = {
    // Per-symbol
    { "bookTicker",          "Individual book ticker",             form_kind::symbol,     &start_book_ticker },
    { "aggTrade",            "Aggregate trade",                    form_kind::symbol,     &start_agg_trade },
    { "markPrice",           "Mark price",                         form_kind::symbol,     &start_mark_price },
    { "ticker",              "24hr ticker",                        form_kind::symbol,     &start_ticker },
    { "miniTicker",          "Mini ticker",                        form_kind::symbol,     &start_mini_ticker },
    { "liquidation",         "Liquidation order",                  form_kind::symbol,     &start_liquidation },
    { "compositeIndex",      "Composite index",                    form_kind::symbol,     &start_composite_index },
    { "assetIndex",          "Asset index",                        form_kind::symbol,     &start_asset_index },
    { "rpiDiffDepth",        "RPI diff book depth",                form_kind::symbol,     &start_rpi_diff_depth },

    // Kline variants
    { "kline",               "Kline (symbol + interval)",          form_kind::kline,      &start_kline },
    { "continuousKline",     "Continuous contract kline",          form_kind::pair_kline, &start_continuous_kline },

    // Depth variants
    { "partialDepth",        "Partial book depth (5/10/20)",       form_kind::levels,     &start_partial_depth },
    { "diffDepth",           "Diff book depth (speed)",            form_kind::speed,      &start_diff_depth },

    // All-market
    { "!bookTicker",         "All book tickers",                   form_kind::no_args,    &start_all_book_tickers },
    { "!ticker",             "All 24hr tickers",                   form_kind::no_args,    &start_all_tickers },
    { "!miniTicker",         "All mini tickers",                   form_kind::no_args,    &start_all_mini_tickers },
    { "!liquidation",        "All liquidations",                   form_kind::no_args,    &start_all_liquidations },
    { "!markPrice",          "All mark prices",                    form_kind::no_args,    &start_all_mark_prices },
    { "!assetIndex",         "All asset index",                    form_kind::no_args,    &start_all_asset_index },
    { "contractInfo",        "Contract info",                      form_kind::no_args,    &start_contract_info },
    { "tradingSession",      "Trading session",                    form_kind::no_args,    &start_trading_session },
};

} // namespace

std::span<const stream_command> stream_commands()
{
    return { entries, sizeof(entries) / sizeof(entries[0]) };
}

} // namespace demo_ui::streams
