// SPDX-License-Identifier: Apache-2.0
//
// Market data commands — public REST endpoints, no authentication required.
// Demonstrates: async_execute() with endpoint_traits, distinct request types,
// and parameterless list endpoints.

#include "cmd_market_data.hpp"

#include <binapi2/fapi/client.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_ping(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::debug("executing ping request");
    auto r = co_await c.market_data.async_execute(types::ping_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("pong");
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_time(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::debug("executing server_time request");
    auto r = co_await c.market_data.async_execute(types::server_time_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("server time: {}", r->serverTime);
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_exchange_info(binapi2::fapi::client& c, const args_t& args)
{
    types::exchange_info_request_t req;
    if (!args.empty()) req.symbol = args[0];

    spdlog::debug("executing exchange_info request symbol={}",
                  req.symbol.value_or("(all)"));
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("symbols: {}  rate_limits: {}", r->symbols.size(), r->rateLimits.size());
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_order_book(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: order-book <symbol> [limit]"); co_return 1; }

    types::order_book_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing order_book request symbol={} limit={}",
                  req.symbol, req.limit.value_or(0));
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("order book: bids={} asks={} lastUpdateId={}",
                 r->bids.size(), r->asks.size(), r->lastUpdateId);

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(5, static_cast<int>(r->bids.size()));
        for (int i = 0; i < n; ++i)
            out("  bid {} x {}", r->bids[static_cast<std::size_t>(i)].price,
                r->bids[static_cast<std::size_t>(i)].quantity);
        n = std::min(5, static_cast<int>(r->asks.size()));
        for (int i = 0; i < n; ++i)
            out("  ask {} x {}", r->asks[static_cast<std::size_t>(i)].price,
                r->asks[static_cast<std::size_t>(i)].quantity);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_recent_trades(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: recent-trades <symbol> [limit]"); co_return 1; }

    types::recent_trades_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing recent_trades request symbol={}", req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("trades: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& t : *r)
            out("  {} x {}{}", t.price, t.qty,
                t.isBuyerMaker ? " sell" : " buy");
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_book_ticker(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: book-ticker <symbol>"); co_return 1; }

    types::book_ticker_request_t req;
    req.symbol = args[0];

    spdlog::debug("executing book_ticker_t request symbol={}", *req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        out("{}  bid: {} x {}  ask: {} x {}", r->symbol,
            r->bidPrice, r->bidQty, r->askPrice, r->askQty);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_book_tickers(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::debug("executing book_tickers (all symbols)");
    auto r = co_await c.market_data.async_execute(types::book_tickers_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("book_tickers: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            out("  {}  {} / {}", (*r)[static_cast<std::size_t>(i)].symbol,
                (*r)[static_cast<std::size_t>(i)].bidPrice,
                (*r)[static_cast<std::size_t>(i)].askPrice);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_price_ticker(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: price-ticker <symbol>"); co_return 1; }

    types::price_ticker_request_t req;
    req.symbol = args[0];

    spdlog::debug("executing price_ticker_t request symbol={}", *req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        out("{}  price: {}", r->symbol, r->price);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_price_tickers(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::debug("executing price_tickers (all symbols)");
    auto r = co_await c.market_data.async_execute(types::price_tickers_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("price_tickers: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            out("  {}  {}", (*r)[static_cast<std::size_t>(i)].symbol,
                (*r)[static_cast<std::size_t>(i)].price);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_ticker_24hr(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: ticker-24hr <symbol>"); co_return 1; }

    types::ticker_24hr_request_t req;
    req.symbol = args[0];

    spdlog::debug("executing ticker_24hr_t request symbol={}", *req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_mark_price(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: mark-price <symbol>"); co_return 1; }

    types::mark_price_request_t req;
    req.symbol = args[0];

    spdlog::debug("executing mark_price_t request symbol={}", *req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        out("{}  mark: {}  index: {}  funding: {}", r->symbol,
            r->markPrice, r->indexPrice, r->lastFundingRate);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_mark_prices(binapi2::fapi::client& c, const args_t& /*args*/)
{
    spdlog::debug("executing mark_prices (all symbols)");
    auto r = co_await c.market_data.async_execute(types::mark_prices_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("mark_prices: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(10, static_cast<int>(r->size()));
        for (int i = 0; i < n; ++i)
            out("  {}  {}", (*r)[static_cast<std::size_t>(i)].symbol,
                (*r)[static_cast<std::size_t>(i)].markPrice);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_klines(binapi2::fapi::client& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: klines <symbol> <interval> [limit]"); co_return 1; }

    types::klines_request_t req;
    req.symbol = args[0];
    req.interval = parse_enum<types::kline_interval_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);

    spdlog::debug("executing klines request symbol={} interval={}",
                  req.symbol, to_string(req.interval));

    auto r = co_await c.market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("klines: {}", r->size());
    if (verbosity >= 1) {
        print_json(*r);
    } else {
        for (auto& k : *r)
            out("  {}  O:{} H:{} L:{} C:{}", k.openTime,
                k.open, k.high, k.low, k.close);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_funding_rate(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: funding-rate <symbol> [limit]"); co_return 1; }

    types::funding_rate_history_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    spdlog::debug("executing funding_rate_history request symbol={}", *req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    co_return handle_result(r);
}

boost::cobalt::task<int> cmd_open_interest(binapi2::fapi::client& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: open-interest <symbol>"); co_return 1; }

    types::open_interest_request_t req;
    req.symbol = args[0];

    spdlog::debug("executing open_interest_t request symbol={}", req.symbol);
    auto r = co_await c.market_data.async_execute(req);
    co_return handle_result(r);
}

} // namespace demo
