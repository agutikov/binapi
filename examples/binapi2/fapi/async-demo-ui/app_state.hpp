// SPDX-License-Identifier: Apache-2.0
//
// Shared mutable state between the FTXUI thread (main, render+input) and the
// asio io_context worker thread that runs the actual binapi coroutines.
//
// Step 0 only defines the bare minimum needed for the layout to render and
// the status bar to reflect connection/credential state. Later steps grow
// `request_capture`, `stream_capture`, and `book_capture`.

#pragma once

#include <atomic>
#include <mutex>
#include <string>

namespace demo_ui {

/// Snapshot of the worker thread's connection state, polled by the status bar.
/// Atomic flags so the render path never blocks; strings are guarded by `mtx`.
struct app_state
{
    // ── set once at startup, never mutated again ──────────────────────
    bool use_testnet = true;

    // ── populated by the worker as startup progresses ─────────────────
    std::atomic<bool> credentials_loaded{ false };
    std::atomic<bool> credentials_failed{ false };
    std::atomic<int>  active_jobs{ 0 };  ///< number of in-flight coroutines

    // Last status / error message; small enough that mutex contention is
    // a non-issue. Render path takes the mutex only to copy.
    mutable std::mutex mtx;
    std::string status_message;
    std::string credentials_error;
};

} // namespace demo_ui
