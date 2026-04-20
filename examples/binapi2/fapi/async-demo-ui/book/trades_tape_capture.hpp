// SPDX-License-Identifier: Apache-2.0
//
// Bounded ring of aggregate-trade prints for the Order Book tab's
// trades tape pane. Newest at the back (the view reverses at render
// time so the freshest entry sits near the book on the right).

#pragma once

#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>

namespace demo_ui {

struct tape_trade
{
    binapi2::fapi::types::decimal_t      price{};
    binapi2::fapi::types::decimal_t      quantity{};
    binapi2::fapi::types::timestamp_ms_t time{};
    bool is_buyer_maker{ false }; ///< true = aggressor was the seller
};

struct trades_tape_capture
{
    /// Set by the Stop button; tape coroutine checks between events
    /// and exits when true. Shared with book_capture's running flag.
    std::atomic<bool> stop{ false };

    /// Monotonic counters for the UI.
    std::atomic<std::uint64_t> total{ 0 };
    std::atomic<std::uint64_t> errors{ 0 };

    mutable std::mutex mtx;

    /// Bounded ring, newest at the back. Render path reads a snapshot
    /// under the lock and reverses for display.
    std::deque<tape_trade> ring;
    static constexpr std::size_t max_ring = 100;

    /// Error message from the stream (if it errored out).
    std::string error;
};

} // namespace demo_ui
