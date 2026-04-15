// SPDX-License-Identifier: Apache-2.0
//
// `capture_sink` implements `binapi2::demo::result_sink` by mutating a
// `request_capture` under its mutex and waking the FTXUI screen on each
// transition (so the render path picks up the new state).
//
// The sink is owned by the spawned coroutine via `std::shared_ptr` —
// the view's Run callback creates one, hands it to `lib::exec_*` (which
// holds it for the duration of the call), and releases its own copy
// once the spawn returns. When the coroutine completes, the
// shared_ptr destructs, the sink with it, and the request_capture is
// left in `done` / `failed` state for the render path to read.

#pragma once

#include "../app_state.hpp"
#include "../worker.hpp"
#include "request_capture.hpp"

#include <binapi2/demo_commands/result_sink.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <string_view>

namespace demo_ui {

class capture_sink final : public binapi2::demo::result_sink
{
public:
    capture_sink(std::shared_ptr<request_capture> cap,
                 worker& wrk,
                 app_state& state)
        : cap_(std::move(cap))
        , worker_(wrk)
        , state_(state)
    {
        spdlog::debug("capture_sink: constructed (cap={})", static_cast<void*>(cap_.get()));
        cap_->state.store(request_capture::running);
        ++state_.active_jobs;
        worker_.notify_ui();
    }

    ~capture_sink() override
    {
        spdlog::debug("capture_sink: destructing (cap={}, state={})",
                      static_cast<void*>(cap_.get()),
                      static_cast<int>(cap_->state.load()));
        // If the executor never called on_done (e.g. cancelled), still
        // release the active-jobs counter so the status bar doesn't lie.
        if (cap_->state.load() == request_capture::running) {
            spdlog::warn("capture_sink: destructing while running — flipping to failed");
            cap_->state.store(request_capture::failed);
            --state_.active_jobs;
            worker_.notify_ui();
        }
    }

    void on_info(std::string_view message) override
    {
        spdlog::debug("capture_sink::on_info: {}", message);
        {
            std::lock_guard lk(cap_->mtx);
            if (!cap_->info_lines.empty()) cap_->info_lines += '\n';
            cap_->info_lines.append(message);
        }
        worker_.notify_ui();
    }

    void on_response_json(std::string_view pretty) override
    {
        spdlog::debug("capture_sink::on_response_json: {} bytes", pretty.size());
        {
            std::lock_guard lk(cap_->mtx);
            fill_pretty_and_parsed(cap_->response, pretty);
        }
        worker_.notify_ui();
    }

    void on_error(const binapi2::fapi::error& err) override
    {
        spdlog::warn("capture_sink::on_error: {} (http={}, binance={})",
                     err.message, err.http_status, err.binance_code);
        {
            std::lock_guard lk(cap_->mtx);
            cap_->error_message = err.message;
            cap_->http_status = err.http_status;
            cap_->binance_code = err.binance_code;
        }
        worker_.notify_ui();
    }

    void on_done(int rc) override
    {
        spdlog::debug("capture_sink::on_done: rc={}", rc);
        cap_->state.store(rc == 0 ? request_capture::done : request_capture::failed);
        --state_.active_jobs;
        worker_.notify_ui();
    }

private:
    std::shared_ptr<request_capture> cap_;
    worker&    worker_;
    app_state& state_;
};

} // namespace demo_ui
