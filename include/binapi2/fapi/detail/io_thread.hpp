// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file io_thread.hpp
/// @brief Background I/O thread that drives a persistent io_context for
///        async operations, with full cobalt coroutine and generator support.

#pragma once

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_thread.hpp>

#include <future>
#include <thread>

namespace binapi2::fapi::detail {

/// @brief Owns an io_context and a background thread running it.
///
/// Provides `run_sync(task<T>)` which spawns a cobalt task on the persistent
/// io_context and blocks the calling thread until it completes. Unlike
/// `cobalt::run()`, the io_context is long-lived — so WebSocket connections
/// and other stateful I/O objects survive across multiple sync calls.
///
/// The background thread initializes the cobalt thread-local executor and
/// PMR resource so that cobalt generators work correctly (not just tasks).
class io_thread
{
public:
    io_thread()
        : work_(boost::asio::make_work_guard(io_))
        , thread_([this] {
            // Set up cobalt thread-local state so generators work on this thread.
            boost::cobalt::this_thread::set_executor(io_.get_executor());
#if !defined(BOOST_COBALT_NO_PMR)
            pmr_resource_.emplace();
            boost::cobalt::this_thread::set_default_resource(&*pmr_resource_);
#endif
            io_.run();
        })
    {
    }

    ~io_thread()
    {
        work_.reset();
        io_.stop();
        if (thread_.joinable())
            thread_.join();
    }

    io_thread(const io_thread&) = delete;
    io_thread& operator=(const io_thread&) = delete;

    [[nodiscard]] boost::asio::io_context& context() noexcept { return io_; }

    /// @brief Spawn a cobalt task on the io_context and block until it completes.
    ///
    /// The task runs on the background thread's io_context. The calling thread
    /// blocks on a future until the result is available. Supports both cobalt
    /// tasks and generators within the spawned task.
    template<typename T>
    T run_sync(boost::cobalt::task<T> task)
    {
        auto future = boost::cobalt::spawn(io_, std::move(task),
                                           boost::asio::use_future);
        return future.get();
    }

private:
    boost::asio::io_context io_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;
#if !defined(BOOST_COBALT_NO_PMR)
    std::optional<boost::cobalt::pmr::unsynchronized_pool_resource> pmr_resource_;
#endif
    std::thread thread_;
};

/// Specialization for void tasks.
template<>
inline void io_thread::run_sync(boost::cobalt::task<void> task)
{
    auto future = boost::cobalt::spawn(io_, std::move(task),
                                       boost::asio::use_future);
    future.get();
}

} // namespace binapi2::fapi::detail
