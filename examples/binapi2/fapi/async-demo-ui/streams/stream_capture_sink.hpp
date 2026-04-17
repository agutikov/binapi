// SPDX-License-Identifier: Apache-2.0
//
// `stream_capture_sink` implements `binapi2::demo::result_sink` for
// market data streams. Each `on_response_json` call pushes the pretty
// JSON into the capture's bounded ring buffer. UI notification is
// throttled to ≤30 Hz to avoid drowning FTXUI's event queue.

#pragma once

#include "../app_state.hpp"
#include "../worker.hpp"
#include "stream_capture.hpp"

#include <binapi2/demo_commands/result_sink.hpp>

#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <string_view>

namespace demo_ui {

class stream_capture_sink final : public binapi2::demo::result_sink
{
public:
    stream_capture_sink(std::shared_ptr<stream_capture> cap,
                        worker& wrk,
                        app_state& state)
        : cap_(std::move(cap))
        , worker_(wrk)
        , state_(state)
    {
        cap_->running = true;
        ++state_.active_jobs;
        worker_.notify_ui();
    }

    ~stream_capture_sink() override
    {
        if (cap_->running.exchange(false)) {
            --state_.active_jobs;
            worker_.notify_ui();
        }
    }

    void on_info(std::string_view /*message*/) override {}

    void on_response_json(std::string_view pretty) override
    {
        {
            std::lock_guard lk(cap_->mtx);
            cap_->ring.emplace_back(pretty);
            if (cap_->ring.size() > stream_capture::max_ring)
                cap_->ring.pop_front();
            cap_->last_event_time = std::chrono::steady_clock::now();
        }
        ++cap_->total_events;
        throttled_notify();
    }

    void on_error(const binapi2::fapi::error& err) override
    {
        spdlog::warn("stream_capture_sink::on_error: {}", err.message);
        {
            std::lock_guard lk(cap_->mtx);
            cap_->error = err.message;
        }
        ++cap_->errors;
        worker_.notify_ui();
    }

    void on_done(int /*rc*/) override
    {
        if (cap_->running.exchange(false)) {
            --state_.active_jobs;
            worker_.notify_ui();
        }
    }

private:
    void throttled_notify()
    {
        auto now = std::chrono::steady_clock::now();
        if (now - last_notify_ < std::chrono::milliseconds(33)) return;
        last_notify_ = now;
        worker_.notify_ui();
    }

    std::shared_ptr<stream_capture> cap_;
    worker&    worker_;
    app_state& state_;
    std::chrono::steady_clock::time_point last_notify_{};
};

} // namespace demo_ui
