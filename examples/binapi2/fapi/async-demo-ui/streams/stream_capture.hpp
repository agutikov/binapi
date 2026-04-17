// SPDX-License-Identifier: Apache-2.0
//
// Shared mutable state for a live market data stream. The worker
// thread's stream sink pushes events here; the FTXUI render thread
// reads the ring buffer and counters.

#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>
#include <string>

namespace demo_ui {

struct stream_capture
{
    /// Set by the Stop button; the stream coroutine checks this between
    /// events and exits when true.
    std::atomic<bool> stop{ false };
    std::atomic<bool> running{ false };

    std::atomic<std::uint64_t> total_events{ 0 };
    std::atomic<std::uint64_t> errors{ 0 };

    mutable std::mutex mtx;

    /// Bounded ring of the last N pretty-printed JSON events (newest
    /// at the back). The render path copies a snapshot under the lock.
    std::deque<std::string> ring;
    static constexpr std::size_t max_ring = 200;

    /// Last event timestamp (for display).
    std::chrono::steady_clock::time_point last_event_time{};

    /// Error message from the stream (if it errored out).
    std::string error;
};

} // namespace demo_ui
