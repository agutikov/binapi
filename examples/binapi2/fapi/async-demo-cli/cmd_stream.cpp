// SPDX-License-Identifier: Apache-2.0
//
// Market data stream commands — async WebSocket streams using cobalt generators.
// Demonstrates: subscribe() returning typed async generators, subscription parameters,
// single-symbol and all-market streams.

#include "cmd_stream.hpp"

#include <binapi2/fapi/client.hpp>

#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-book-ticker <symbol>\n"; co_return 1; }

    spdlog::info("subscribing to book_ticker_t stream for {}", args[0]);
    types::book_ticker_subscription sub;
    sub.symbol = args[0];
    auto gen = c.streams().subscribe(sub);

    spdlog::info("connected, reading events...");
    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol
                      << "  bid: " << event->best_bid_price << " x " << event->best_bid_qty
                      << "  ask: " << event->best_ask_price << " x " << event->best_ask_qty << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_mark_price(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-mark-price <symbol>\n"; co_return 1; }

    spdlog::info("subscribing to mark_price_t stream for {}", args[0]);
    types::mark_price_subscription sub;
    sub.symbol = args[0];
    auto gen = c.streams().subscribe(sub);

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol
                      << "  mark: " << event->mark_price_t
                      << "  index: " << event->index_price
                      << "  funding: " << event->funding_rate << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_kline(binapi2::fapi::client& c, const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: stream-kline <symbol> <interval>\n"; co_return 1; }

    spdlog::info("subscribing to kline_t stream for {} {}", args[0], args[1]);
    types::kline_subscription sub;
    sub.symbol = args[0];
    sub.interval = parse_enum<types::kline_interval_t>(args[1]);
    auto gen = c.streams().subscribe(sub);

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol << "  O:" << event->kline_t.open_price
                      << " H:" << event->kline_t.high_price << " L:" << event->kline_t.low_price
                      << " C:" << event->kline_t.close_price << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_ticker(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-ticker <symbol>\n"; co_return 1; }

    spdlog::info("subscribing to ticker stream for {}", args[0]);
    types::ticker_subscription sub;
    sub.symbol = args[0];
    auto gen = c.streams().subscribe(sub);

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol << "  last: " << event->last_price
                      << "  change: " << event->price_change_pct << "%\n";
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_depth(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-depth <symbol> [levels]\n"; co_return 1; }

    types::partial_book_depth_subscription sub;
    sub.symbol = args[0];
    if (args.size() > 1) sub.levels = std::stoi(args[1]);

    spdlog::info("subscribing to partial_book_depth stream for {} levels={}", sub.symbol, sub.levels);
    auto gen = c.streams().subscribe(sub);

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol << "  bids: " << event->bids.size()
                      << "  asks: " << event->asks.size() << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_all_book_tickers(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::info("subscribing to all_book_tickers stream");
    auto gen = c.streams().subscribe(types::all_book_ticker_subscription{});

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << event->symbol << "  " << event->best_bid_price << " / " << event->best_ask_price << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_all_tickers(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::info("subscribing to all_market_tickers stream");
    auto gen = c.streams().subscribe(types::all_market_ticker_subscription{});

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << "tickers: " << event->size() << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_all_mini_tickers(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::info("subscribing to all_market_mini_tickers stream");
    auto gen = c.streams().subscribe(types::all_market_mini_ticker_subscription{});

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << "mini_tickers: " << event->size() << '\n';
        }
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_stream_liquidation(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-liquidation <symbol>\n"; co_return 1; }

    spdlog::info("subscribing to liquidation_order stream for {}", args[0]);
    types::liquidation_order_subscription sub;
    sub.symbol = args[0];
    auto gen = c.streams().subscribe(sub);

    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else {
            std::cout << "liquidation: " << event->order.symbol
                      << "  " << to_string(event->order.side) << "  " << event->order.price << '\n';
        }
    }
    co_return 0;
}

} // namespace demo
