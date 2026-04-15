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

#include <binapi2/fapi/config.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include <ftxui/component/screen_interactive.hpp>

#include <memory>
#include <thread>

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

private:
    void load_credentials_async();

    app_state&                  state_;
    ftxui::ScreenInteractive&   screen_;
    boost::asio::io_context     ioc_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::unique_ptr<binapi2::futures_usdm_api> api_;
    std::thread                 thread_;
    bool                        started_ = false;
};

} // namespace demo_ui
