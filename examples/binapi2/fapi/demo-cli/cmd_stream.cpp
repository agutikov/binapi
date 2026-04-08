// SPDX-License-Identifier: Apache-2.0
//
// Market data stream commands — WebSocket streams with blocking read loops.
// Demonstrates: connect_*/read_*_loop pattern, subscription parameters,
// single-symbol and all-market streams.

#include "cmd_stream.hpp"

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

int cmd_stream_book_ticker(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-book-ticker <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to book_ticker stream for {}", args[0]);
    if (auto c = streams.connect_book_ticker({ .symbol = args[0] }); !c) {
        print_error(c.err); return 1;
    }

    spdlog::info("connected, reading events...");
    auto loop = streams.read_book_ticker_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol
                      << "  bid: " << e.best_bid_price << " x " << e.best_bid_qty
                      << "  ask: " << e.best_ask_price << " x " << e.best_ask_qty << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_mark_price(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-mark-price <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to mark_price stream for {}", args[0]);
    if (auto c = streams.connect_mark_price({ .symbol = args[0] }); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_mark_price_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol
                      << "  mark: " << e.mark_price
                      << "  index: " << e.index_price
                      << "  funding: " << e.funding_rate << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_kline(const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: stream-kline <symbol> <interval>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to kline stream for {} {}", args[0], args[1]);
    if (auto c = streams.connect_kline({
            .symbol = args[0],
            .interval = parse_enum<binapi2::fapi::types::kline_interval_t>(args[1]) }); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_kline_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol << "  O:" << e.kline.open_price
                      << " H:" << e.kline.high_price << " L:" << e.kline.low_price
                      << " C:" << e.kline.close_price << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_ticker(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-ticker <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to ticker stream for {}", args[0]);
    if (auto c = streams.connect_ticker({ .symbol = args[0] }); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_ticker_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol << "  last: " << e.last_price
                      << "  change: " << e.price_change_pct << "%\n";
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_depth(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-depth <symbol> [levels]\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    binapi2::fapi::streams::partial_book_depth_subscription sub;
    sub.symbol = args[0];
    if (args.size() > 1) sub.levels = std::stoi(args[1]);

    spdlog::info("connecting to partial_book_depth stream for {} levels={}", sub.symbol, sub.levels);
    if (auto c = streams.connect_partial_book_depth(sub); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_partial_book_depth_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol << "  bids: " << e.bids.size()
                      << "  asks: " << e.asks.size() << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_all_book_tickers(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to all_book_tickers stream");
    if (auto c = streams.connect_all_book_tickers(); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_all_book_tickers_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << e.symbol << "  " << e.best_bid_price << " / " << e.best_ask_price << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_all_tickers(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to all_market_tickers stream");
    if (auto c = streams.connect_all_market_tickers(); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_all_market_tickers_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << "tickers: " << e.size() << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_all_mini_tickers(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to all_market_mini_tickers stream");
    if (auto c = streams.connect_all_market_mini_tickers(); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_all_market_mini_tickers_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << "mini_tickers: " << e.size() << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

int cmd_stream_liquidation(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: stream-liquidation <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::streams::market_streams streams{ io, make_config() };

    spdlog::info("connecting to liquidation_order stream for {}", args[0]);
    if (auto c = streams.connect_liquidation_order({ .symbol = args[0] }); !c) {
        print_error(c.err); return 1;
    }

    auto loop = streams.read_liquidation_order_loop([](const auto& e) {
        if (verbosity >= 1) { print_json(e); }
        else {
            std::cout << "liquidation: " << e.order.symbol
                      << "  " << to_string(e.order.side) << "  " << e.order.price << '\n';
        }
        return false;
    });
    (void)streams.close();
    return loop ? 0 : (print_error(loop.err), 1);
}

} // namespace demo
