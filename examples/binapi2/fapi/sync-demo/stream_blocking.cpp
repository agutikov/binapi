// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: Stream with blocking read loop via all three execution environments.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <future>
#include <iostream>
#include <vector>

namespace sync_demo {

namespace types = binapi2::fapi::types;
using binapi2::futures_usdm_api;
using binapi2::fapi::result;

// Each invocation creates its own market_streams so the websocket_client's
// lifetime is tied to the coroutine, not to the shared client.
static boost::cobalt::task<result<std::vector<types::book_ticker_stream_event_t>>>
read_stream_events(binapi2::fapi::config cfg, int count)
{
    binapi2::fapi::streams::market_streams streams(std::move(cfg));
    auto conn = co_await streams.async_connect(
        types::book_ticker_subscription{.symbol = "BTCUSDT"});
    if (!conn)
        co_return result<std::vector<types::book_ticker_stream_event_t>>::failure(conn.err);

    std::vector<types::book_ticker_stream_event_t> events;
    for (int i = 0; i < count; ++i) {
        auto e = co_await streams.async_read_event<types::book_ticker_stream_event_t>();
        if (!e)
            break;
        events.push_back(std::move(*e));
    }
    co_await streams.async_close();
    co_return result<std::vector<types::book_ticker_stream_event_t>>::success(std::move(events));
}

static void print_events(const result<std::vector<types::book_ticker_stream_event_t>>& r)
{
    if (!r) {
        std::cout << "  error: " << r.err.message << "\n";
        return;
    }
    std::cout << "  received " << r->size() << " events:\n";
    for (const auto& e : *r) {
        std::cout << "    " << e.symbol << "  bid: " << e.best_bid_price
                  << "  ask: " << e.best_ask_price << "\n";
    }
}

void stream_blocking(futures_usdm_api& c)
{
    auto cfg = c.configuration();

    // --- 1. io_thread: run_sync blocks the caller ---
    {
        std::cout << "=== Stream blocking: io_thread ===\n";
        binapi2::fapi::detail::io_thread io;
        auto r = io.run_sync(read_stream_events(cfg, 3));
        print_events(r);
    }

    // --- 2. std::async: spawn on io_context, future.get() ---
    {
        std::cout << "=== Stream blocking: std::async ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        auto future = boost::cobalt::spawn(io, read_stream_events(cfg, 3),
                                           boost::asio::use_future);
        auto r = future.get();

        guard.reset();
        io_future.get();
        print_events(r);
    }

    // --- 3. Manual io_context: spawn, io.run() on caller thread ---
    {
        std::cout << "=== Stream blocking: manual io_context ===\n";
        boost::asio::io_context io;
        auto future = boost::cobalt::spawn(io, read_stream_events(cfg, 3),
                                           boost::asio::use_future);
        io.run();
        auto r = future.get();
        print_events(r);
    }
}

} // namespace sync_demo
