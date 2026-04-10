// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: WS API with blocking call via all three execution environments.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <cstdlib>
#include <future>
#include <iostream>

namespace sync_demo {

namespace types = binapi2::fapi::types;
using binapi2::futures_usdm_api;
using binapi2::fapi::result;

static boost::cobalt::task<result<types::book_ticker_t>>
ws_book_ticker(futures_usdm_api& c)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws)
        co_return result<types::book_ticker_t>::failure(ws.err);

    auto conn = co_await (*ws)->async_connect();
    if (!conn)
        co_return result<types::book_ticker_t>::failure(conn.err);

    if (!c.configuration().api_key.empty()) {
        auto logon = co_await (*ws)->async_session_logon();
        if (!logon)
            co_return result<types::book_ticker_t>::failure(logon.err);
    }

    auto r = co_await (*ws)->async_execute(
        types::websocket_api_book_ticker_request_t{.symbol = "BTCUSDT"});
    co_await (*ws)->async_close();

    if (!r || !r->result)
        co_return result<types::book_ticker_t>::failure(r.err);

    co_return result<types::book_ticker_t>::success(std::move(*r->result));
}

static void print_result(const result<types::book_ticker_t>& r)
{
    if (r)
        std::cout << "  symbol: " << r->symbol << "  bid: " << r->bidPrice
                  << "  ask: " << r->askPrice << "\n";
    else
        std::cout << "  error: " << r.err.message << "\n";
}

void ws_api_blocking(futures_usdm_api& c)
{
    const char* key = std::getenv("BINANCE_API_KEY");
    if (!key || key[0] == '\0') {
        std::cout << "=== WS API blocking: SKIPPED (BINANCE_API_KEY not set) ===\n";
        return;
    }

    // --- 1. io_thread: run_sync blocks the caller ---
    {
        std::cout << "=== WS API blocking: io_thread ===\n";
        binapi2::fapi::detail::io_thread io;
        auto r = io.run_sync(ws_book_ticker(c));
        print_result(r);
    }

    // --- 2. std::async: spawn on io_context, future.get() ---
    {
        std::cout << "=== WS API blocking: std::async ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        auto future = boost::cobalt::spawn(io, ws_book_ticker(c),
                                           boost::asio::use_future);
        auto r = future.get();

        guard.reset();
        io_future.get();
        print_result(r);
    }

    // --- 3. Manual io_context: spawn, io.run() on caller thread ---
    {
        std::cout << "=== WS API blocking: manual io_context ===\n";
        boost::asio::io_context io;
        auto future = boost::cobalt::spawn(io, ws_book_ticker(c),
                                           boost::asio::use_future);
        io.run();
        auto r = future.get();
        print_result(r);
    }
}

} // namespace sync_demo
