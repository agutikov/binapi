// SPDX-License-Identifier: Apache-2.0
//
// Market data commands — public REST endpoints, no authentication required.
// Demonstrates: execute() with endpoint_traits, named methods for shared
// request types, and parameterless list endpoints.

#include "cmd_market_data.hpp"

#include <binapi2/fapi/client.hpp>

#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace demo {

int cmd_ping(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing ping request");
    auto r = client.market_data.execute(binapi2::fapi::types::ping_request{});
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("pong");
    if (verbosity >= 1) print_json(*r);
    return 0;
}

int cmd_time(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing server_time request");
    auto r = client.market_data.execute(binapi2::fapi::types::server_time_request{});
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("server time: {}", r->serverTime);
    if (verbosity >= 1) print_json(*r);
    return 0;
}

int cmd_exchange_info(const args_t& args)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::exchange_info_request req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("executing exchange_info request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("symbols: {}  rate_limits: {}", r->symbols.size(), r->rateLimits.size());
    if (verbosity >= 1) print_json(*r);
    return 0;
}

int cmd_order_book(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: order-book <symbol> [limit]\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::order_book_request req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing order_book request symbol={} limit={}",
                  req.symbol, req.limit.value_or(0));
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("order book: bids={} asks={} lastUpdateId={}",
                 r->bids.size(), r->asks.size(), r->lastUpdateId);

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(5, static_cast<int>(r->bids.size()));
        for (int i = 0; i < n; ++i)
            std::cout << "  bid " << r->bids[static_cast<std::size_t>(i)].price
                      << " x " << r->bids[static_cast<std::size_t>(i)].quantity << '\n';
        n = std::min(5, static_cast<int>(r->asks.size()));
        for (int i = 0; i < n; ++i)
            std::cout << "  ask " << r->asks[static_cast<std::size_t>(i)].price
                      << " x " << r->asks[static_cast<std::size_t>(i)].quantity << '\n';
    }
    return 0;
}

int cmd_recent_trades(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: recent-trades <symbol> [limit]\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::recent_trades_request req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing recent_trades request symbol={}", req.symbol);
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("trades: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& t : *r)
            std::cout << "  " << t.price << " x " << t.qty
                      << (t.isBuyerMaker ? " sell" : " buy") << '\n';
    }
    return 0;
}

int cmd_book_ticker(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: book-ticker <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::book_ticker_request req;
    req.symbol = args[0];

    spdlog::debug("executing book_ticker request symbol={}", *req.symbol);
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        std::cout << r->symbol << "  bid: " << r->bidPrice << " x " << r->bidQty
                  << "  ask: " << r->askPrice << " x " << r->askQty << '\n';
    }
    return 0;
}

int cmd_book_tickers(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing book_tickers (all symbols)");
    auto r = client.market_data.book_tickers();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("book_tickers: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            std::cout << "  " << (*r)[static_cast<std::size_t>(i)].symbol
                      << "  " << (*r)[static_cast<std::size_t>(i)].bidPrice
                      << " / " << (*r)[static_cast<std::size_t>(i)].askPrice << '\n';
    }
    return 0;
}

int cmd_price_ticker(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: price-ticker <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::price_ticker_request req;
    req.symbol = args[0];

    spdlog::debug("executing price_ticker request symbol={}", *req.symbol);
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        std::cout << r->symbol << "  price: " << r->price << '\n';
    }
    return 0;
}

int cmd_price_tickers(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing price_tickers (all symbols)");
    auto r = client.market_data.price_tickers();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("price_tickers: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            std::cout << "  " << (*r)[static_cast<std::size_t>(i)].symbol
                      << "  " << (*r)[static_cast<std::size_t>(i)].price << '\n';
    }
    return 0;
}

int cmd_ticker_24hr(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: ticker-24hr <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::ticker_24hr_request req;
    req.symbol = args[0];

    spdlog::debug("executing ticker_24hr request symbol={}", *req.symbol);
    auto r = client.market_data.execute(req);
    return handle_result(r);
}

int cmd_mark_price(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: mark-price <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::mark_price_request req;
    req.symbol = args[0];

    spdlog::debug("executing mark_price request symbol={}", *req.symbol);
    auto r = client.market_data.execute(req);
    if (!r) { print_error(r.err); return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        std::cout << r->symbol << "  mark: " << r->markPrice
                  << "  index: " << r->indexPrice
                  << "  funding: " << r->lastFundingRate << '\n';
    }
    return 0;
}

int cmd_mark_prices(const args_t& /*args*/)
{
    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    spdlog::debug("executing mark_prices (all symbols)");
    auto r = client.market_data.mark_prices();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("mark_prices: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            std::cout << "  " << (*r)[static_cast<std::size_t>(i)].symbol
                      << "  " << (*r)[static_cast<std::size_t>(i)].markPrice << '\n';
    }
    return 0;
}

int cmd_klines(const args_t& args)
{
    if (args.size() < 2) { std::cerr << "usage: klines <symbol> <interval> [limit]\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::kline_request req;
    req.symbol = args[0];
    req.interval = parse_enum<binapi2::fapi::types::kline_interval_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);

    spdlog::debug("executing klines request symbol={} interval={}",
                  req.symbol, to_string(req.interval));

    auto r = client.market_data.klines(req);
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("klines: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& k : *r)
            std::cout << "  " << k.openTime
                      << "  O:" << k.open << " H:" << k.high
                      << " L:" << k.low << " C:" << k.close << '\n';
    }
    return 0;
}

int cmd_funding_rate(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: funding-rate <symbol> [limit]\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::funding_rate_history_request req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing funding_rate_history request symbol={}", *req.symbol);
    auto r = client.market_data.execute(req);
    return handle_result(r);
}

int cmd_open_interest(const args_t& args)
{
    if (args.empty()) { std::cerr << "usage: open-interest <symbol>\n"; return 1; }

    boost::asio::io_context io;
    binapi2::fapi::client client{ io, make_config() };

    binapi2::fapi::types::open_interest_request req;
    req.symbol = args[0];

    spdlog::debug("executing open_interest request symbol={}", req.symbol);
    auto r = client.market_data.execute(req);
    return handle_result(r);
}

} // namespace demo
