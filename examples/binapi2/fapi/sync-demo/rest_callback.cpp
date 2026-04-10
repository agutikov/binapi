// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: REST with callback — spawn with lambda, wait on promise.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <exception>
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

static void print_result(const char* label, const result<types::server_time_response_t>& r)
{
    if (r)
        std::cout << "  server_time: " << r->serverTime << "\n";
    else
        std::cout << "  error: " << r.err.message << "\n";
    (void)label;
}

void rest_callback(futures_usdm_api& c)
{
    // --- 1. io_thread: spawn with callback ---
    {
        std::cout << "=== REST callback: io_thread ===\n";
        binapi2::fapi::detail::io_thread io;
        std::promise<result<types::server_time_response_t>> promise;
        auto future = promise.get_future();

        boost::cobalt::spawn(io.context(), get_server_time(c),
            [&promise](std::exception_ptr ep, result<types::server_time_response_t> r) {
                if (ep) {
                    promise.set_exception(ep);
                } else {
                    promise.set_value(std::move(r));
                }
            });

        auto r = future.get();
        print_result("io_thread", r);
    }

    // --- 2. std::async: spawn with callback ---
    {
        std::cout << "=== REST callback: std::async ===\n";
        boost::asio::io_context io;
        auto guard = boost::asio::make_work_guard(io);
        auto io_future = std::async(std::launch::async, [&io] { io.run(); });

        std::promise<result<types::server_time_response_t>> promise;
        auto future = promise.get_future();

        boost::cobalt::spawn(io, get_server_time(c),
            [&promise](std::exception_ptr ep, result<types::server_time_response_t> r) {
                if (ep) {
                    promise.set_exception(ep);
                } else {
                    promise.set_value(std::move(r));
                }
            });

        auto r = future.get();
        guard.reset();
        io_future.get();
        print_result("std::async", r);
    }

    // --- 3. Manual io_context: spawn with callback ---
    {
        std::cout << "=== REST callback: manual io_context ===\n";
        boost::asio::io_context io;

        std::promise<result<types::server_time_response_t>> promise;
        auto future = promise.get_future();

        boost::cobalt::spawn(io, get_server_time(c),
            [&promise](std::exception_ptr ep, result<types::server_time_response_t> r) {
                if (ep) {
                    promise.set_exception(ep);
                } else {
                    promise.set_value(std::move(r));
                }
            });

        io.run();
        auto r = future.get();
        print_result("manual io_context", r);
    }
}

} // namespace sync_demo
