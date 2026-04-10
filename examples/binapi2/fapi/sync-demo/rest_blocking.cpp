// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: REST blocking calls via all three execution environments.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <future>
#include <iostream>

namespace sync_demo {

namespace types = binapi2::fapi::types;
using binapi2::futures_usdm_api;
using binapi2::fapi::result;

static boost::cobalt::task<result<types::server_time_response_t>>
get_server_time(futures_usdm_api& c)
{
    auto rest = co_await c.create_rest_client();
    if (!rest)
        co_return result<types::server_time_response_t>::failure(rest.err);
    co_return co_await (*rest)->market_data.async_execute(types::server_time_request_t{});
}

void rest_blocking(futures_usdm_api& c)
{
    // --- 1. io_thread: run_sync blocks the caller ---
    {
        std::cout << "=== REST blocking: io_thread ===\n";
        binapi2::fapi::detail::io_thread io;
        auto r = io.run_sync(get_server_time(c));
        if (r)
            std::cout << "  server_time: " << r->serverTime << "\n";
        else
            std::cout << "  error: " << r.err.message << "\n";
    }

    // --- 2. std::async: spawn on io_context, future.get() ---
    {
        std::cout << "=== REST blocking: std::async ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        auto future = boost::cobalt::spawn(io, get_server_time(c),
                                           boost::asio::use_future);
        auto r = future.get();

        guard.reset();
        io_future.get();

        if (r)
            std::cout << "  server_time: " << r->serverTime << "\n";
        else
            std::cout << "  error: " << r.err.message << "\n";
    }

    // --- 3. Manual io_context: spawn, io.run() on caller thread ---
    {
        std::cout << "=== REST blocking: manual io_context ===\n";
        boost::asio::io_context io;
        auto future = boost::cobalt::spawn(io, get_server_time(c),
                                           boost::asio::use_future);
        io.run();
        auto r = future.get();

        if (r)
            std::cout << "  server_time: " << r->serverTime << "\n";
        else
            std::cout << "  error: " << r.err.message << "\n";
    }
}

} // namespace sync_demo
