// SPDX-License-Identifier: Apache-2.0
//
// `worker` owns the asio::io_context and the thread that drives it.
// The FTXUI screen runs on the main thread; this background thread runs all
// binapi coroutines via `co_spawn`. The two threads communicate through:
//
//   * the `app_state` struct (atomics + a mutex for strings)
//   * `screen.PostEvent(Event::Custom)` — the worker pokes the screen so it
//     re-renders after a state mutation.
//
// Step 0 only supports startup + clean shutdown. Step 1 adds `co_spawn`.

#pragma once

#include "app_state.hpp"
#include "util/request_capture.hpp"

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/client.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/cobalt/task.hpp>

#include <ftxui/component/screen_interactive.hpp>

#include <atomic>
#include <memory>
#include <thread>

namespace binapi2::demo { class result_sink; }

namespace demo_ui {

class worker
{
public:
    worker(app_state& state, ftxui::ScreenInteractive& screen, binapi2::fapi::config cfg);
    ~worker();

    worker(const worker&) = delete;
    worker& operator=(const worker&) = delete;

    /// Start the io_context thread and kick off async credential loading.
    void start();

    /// Stop the io_context, join the thread. Idempotent.
    void stop();

    /// Wake the screen so it re-renders (called from worker-thread tasks).
    void notify_ui();

    binapi2::futures_usdm_api& api() { return *api_; }
    boost::asio::io_context&   io()  { return ioc_; }

    /// Set / clear the request_capture that `cfg.logger` should fill in
    /// for raw HTTP capture. Set inside a cobalt task body before
    /// dispatching the request, clear after it returns. Single-threaded
    /// io_context means there's at most one active capture at a time.
    void set_active_capture(request_capture* cap) { active_capture_.store(cap, std::memory_order_release); }
    request_capture* active_capture() { return active_capture_.load(std::memory_order_acquire); }

    /// Acquire the long-lived REST client. Lazy-connects on first call;
    /// subsequent calls return the cached client immediately so every
    /// REST request after the first reuses the same TCP+TLS connection.
    ///
    /// On connect failure: emits the error through `sink` (so the
    /// caller doesn't have to) and returns nullptr.
    ///
    /// Must be co_awaited from the worker thread (i.e. from inside a
    /// task spawned on `io().get_executor()`).
    [[nodiscard]] boost::cobalt::task<binapi2::fapi::rest::client*>
    acquire_rest_client(binapi2::demo::result_sink& sink);

private:
    void load_credentials_async();
    void install_transport_logger();

    app_state&                  state_;
    ftxui::ScreenInteractive&   screen_;
    boost::asio::io_context     ioc_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::unique_ptr<binapi2::futures_usdm_api> api_;
    std::unique_ptr<binapi2::fapi::rest::client> rest_client_;  ///< lazy, persistent
    std::thread                 thread_;
    bool                        started_ = false;
    std::atomic<request_capture*> active_capture_{ nullptr };
};

/// RAII guard for `worker::set_active_capture` — sets in ctor, clears
/// in dtor. Use inside a cobalt task body to scope cfg.logger output
/// to the current request.
class active_capture_guard
{
public:
    active_capture_guard(worker& wrk, request_capture* cap)
        : wrk_(wrk) { wrk_.set_active_capture(cap); }
    ~active_capture_guard() { wrk_.set_active_capture(nullptr); }

    active_capture_guard(const active_capture_guard&) = delete;
    active_capture_guard& operator=(const active_capture_guard&) = delete;

private:
    worker& wrk_;
};

} // namespace demo_ui
