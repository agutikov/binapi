// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — periodic app-wide status logger.

/// @file status_reporter.hpp
/// @brief Single periodic logger that collects state from every stage
///        (screener, selector, detail, rest_sync, ...) and emits one
///        spdlog line per tick. Keeps stages decoupled: each stage
///        registers a `source_fn` at startup and exposes whatever state
///        it wants; the reporter does the formatting and the timing.

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/cobalt/task.hpp>

#include <chrono>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace binapi2::examples::async_recorder {

class status_reporter
{
public:
    /// @brief Source callback — returns the stage's current state as a
    ///        short string (e.g. "bt=1500 mp=12 tk=10"). Called on the
    ///        reporter's own coroutine so no synchronisation is needed
    ///        when everything shares one executor.
    using source_fn = std::function<std::string()>;

    status_reporter(boost::asio::io_context& ioc, std::chrono::seconds interval);

    status_reporter(const status_reporter&) = delete;
    status_reporter& operator=(const status_reporter&) = delete;

    /// @brief Register a named source. Must be called before `run()`
    ///        starts ticking. Sources are emitted in registration order.
    void add_source(std::string name, source_fn fn);

    /// @brief Periodic loop: every `interval`, gather from all sources
    ///        and emit one `spdlog::info` line with the full state.
    ///        Returns when `close()` is called.
    boost::cobalt::task<void> run();

    /// @brief Signal the loop to exit at its next wakeup and cancel the
    ///        currently-pending timer wait.
    void close();

private:
    struct source
    {
        std::string name;
        source_fn fn;
    };

    boost::asio::steady_timer timer_;
    std::chrono::seconds interval_;
    std::vector<source> sources_{};
    bool closed_{ false };
};

} // namespace binapi2::examples::async_recorder
