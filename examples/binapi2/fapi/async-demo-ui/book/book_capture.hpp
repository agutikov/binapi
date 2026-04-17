// SPDX-License-Identifier: Apache-2.0
//
// Shared mutable state for the live order book. The worker thread's
// snapshot callback writes here; the FTXUI render thread reads it.

#pragma once

#include <binapi2/fapi/order_book/local_order_book.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

namespace demo_ui {

struct book_capture
{
    std::atomic<bool>          running{ false };
    std::atomic<std::uint64_t> updates{ 0 };

    mutable std::mutex mtx;
    binapi2::fapi::order_book::order_book_snapshot latest;
    std::string error;

    /// Set by the UI thread before spawning the coroutine; read by
    /// the coroutine to configure the subscription.
    std::string symbol = "BTCUSDT";
    int depth = 1000;

    /// Pointer to the running book — set by the coroutine, used by
    /// the Stop button to call `book->stop()`. Guarded by `mtx`.
    binapi2::fapi::order_book::local_order_book* book_ptr = nullptr;
};

} // namespace demo_ui
