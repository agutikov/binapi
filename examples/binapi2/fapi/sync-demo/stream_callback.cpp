// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: Stream with callback per event — spawn a task that reads events
// and posts each to a callback on the caller's side via a thread-safe queue.

#include "examples.hpp"

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace sync_demo {

namespace types = binapi2::fapi::types;
using binapi2::fapi::client;
using binapi2::fapi::result;

/// Thread-safe queue for passing events from the I/O thread to the caller.
template<typename T>
class event_queue
{
public:
    void push(T item)
    {
        {
            std::lock_guard<std::mutex> lk(mu_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    void close()
    {
        {
            std::lock_guard<std::mutex> lk(mu_);
            closed_ = true;
        }
        cv_.notify_one();
    }

    /// Blocks until an item is available or the queue is closed.
    /// Returns nullopt when closed and empty.
    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty())
            return std::nullopt;
        T item = std::move(q_.front());
        q_.pop();
        return item;
    }

private:
    std::mutex mu_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool closed_{false};
};

using event_t = result<types::book_ticker_stream_event_t>;

/// Coroutine that reads events and pushes each to the queue.
/// Creates its own market_streams so the websocket lifetime matches the coroutine.
static boost::cobalt::task<void>
read_and_post(binapi2::fapi::config cfg, int count, event_queue<event_t>& queue)
{
    binapi2::fapi::streams::market_streams streams(std::move(cfg));
    auto conn = co_await streams.async_connect(
        types::book_ticker_subscription{.symbol = "BTCUSDT"});
    if (!conn) {
        queue.push(result<types::book_ticker_stream_event_t>::failure(conn.err));
        queue.close();
        co_return;
    }

    for (int i = 0; i < count; ++i) {
        auto e = co_await streams.async_read_event<types::book_ticker_stream_event_t>();
        queue.push(std::move(e));
        if (!e)
            break;
    }
    co_await streams.async_close();
    queue.close();
}

static void consume_queue(event_queue<event_t>& queue)
{
    int n = 0;
    while (auto item = queue.pop()) {
        if (*item) {
            std::cout << "    [" << n << "] " << (*item)->symbol
                      << "  bid: " << (*item)->best_bid_price
                      << "  ask: " << (*item)->best_ask_price << "\n";
        } else {
            std::cout << "    [" << n << "] error: " << item->err.message << "\n";
        }
        ++n;
    }
    std::cout << "  total events: " << n << "\n";
}

void stream_callback(client& c)
{
    // --- 1. io_thread: spawn task, consume from queue on caller thread ---
    {
        std::cout << "=== Stream callback: io_thread ===\n";
        binapi2::fapi::detail::io_thread io;
        event_queue<event_t> queue;

        boost::cobalt::spawn(io.context(), read_and_post(c.configuration(), 3, queue),
            [](std::exception_ptr) {});

        consume_queue(queue);
    }

    // --- 2. std::async: spawn task, consume from queue on caller thread ---
    {
        std::cout << "=== Stream callback: std::async ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        event_queue<event_t> queue;

        boost::cobalt::spawn(io, read_and_post(c.configuration(), 3, queue),
            [](std::exception_ptr) {});

        consume_queue(queue);

        guard.reset();
        io_future.get();
    }

    // --- 3. Manual io_context: run io on a background thread for this example,
    //        since io.run() would block and we need the caller thread to consume. ---
    {
        std::cout << "=== Stream callback: manual io_context ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);

        event_queue<event_t> queue;

        boost::cobalt::spawn(io, read_and_post(c.configuration(), 3, queue),
            [](std::exception_ptr) {});

        // Run io_context on a separate thread so the caller can consume the queue.
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        consume_queue(queue);

        guard.reset();
        io_future.get();
    }
}

} // namespace sync_demo
