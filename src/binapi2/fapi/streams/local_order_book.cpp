// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements a locally-maintained order book that stays synchronised
/// with the exchange via the diff depth WebSocket stream, following the
/// Binance "How to manage a local order book correctly" algorithm:
///
///   1. Open a @depth@100ms WebSocket stream and begin buffering events.
///   2. Fetch a REST depth snapshot (GET /fapi/v1/depth).
///   3. Discard buffered events where u < snapshot.lastUpdateId.
///   4. Verify the first remaining event brackets the snapshot
///      (U <= lastUpdateId <= u).
///   5. Apply remaining buffered events, then process live events.
///   6. On each live event, verify pu == previous u (sequence continuity).
///      If a gap is detected, re-sync from step 2.
///   7. Remove price levels with quantity == 0.
///
/// All book state is protected by a mutex so snapshot() can be called from
/// any thread while the read loop runs on the io_context thread.

#include <binapi2/fapi/streams/local_order_book.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/streams/subscriptions.hpp>

#include <algorithm>
#include <mutex>
#include <utility>

namespace binapi2::fapi::streams {

local_order_book::local_order_book(market_streams& streams, client& rest_client) noexcept :
    streams_(streams), rest_client_(rest_client)
{
}

result<void>
local_order_book::start(const std::string& symbol, int depth_limit)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        symbol_ = symbol;
        depth_limit_ = depth_limit;
        synced_ = false;
        running_ = true;
        buffer_.clear();
        book_ = {};
        last_u_ = 0;
    }

    // Step 1: open diff depth stream
    diff_book_depth_subscription sub;
    sub.symbol = symbol;
    sub.speed = "100ms";
    auto connect_result = streams_.connect_diff_book_depth(sub);
    if (!connect_result) {
        return connect_result;
    }

    // Step 2: buffer incoming diff events. On the first event (or after a
    // sequence gap), fetch a REST snapshot and apply the buffered events to
    // bring the book up to date. From then on, events are applied directly.
    auto loop_result = streams_.read_diff_book_depth_loop([this](const types::depth_stream_event& event) -> bool {
        bool need_snapshot = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                return false;
            }

            if (!synced_) {
                buffer_.push_back(event);
                // Trigger snapshot on the first buffered event
                need_snapshot = true;
            } else {
                // Step 6: validate pu == previous u
                if (event.prev_final_update_id != last_u_) {
                    synced_ = false;
                    buffer_.clear();
                    buffer_.push_back(event);
                    need_snapshot = true;
                } else {
                    apply_event(event);
                    last_u_ = event.final_update_id;

                    if (on_snapshot_) {
                        on_snapshot_(book_);
                    }
                    return true;
                }
            }
        }

        if (need_snapshot) {
            fetch_snapshot();
        }

        std::lock_guard<std::mutex> lock(mutex_);
        return running_;
    });

    return loop_result;
}

void
local_order_book::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
}

order_book_snapshot
local_order_book::snapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return book_;
}

void
local_order_book::set_snapshot_callback(snapshot_callback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    on_snapshot_ = std::move(callback);
}

// Fetches the REST depth snapshot and reconciles it with buffered diff events.
// The mutex is intentionally released during the network call to avoid blocking
// the WebSocket read loop; it is re-acquired before mutating book state.
void
local_order_book::fetch_snapshot()
{
    // Step 3: get REST depth snapshot (no lock held during network call)
    types::order_book_request request;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        request.symbol = symbol_;
        request.limit = depth_limit_;
    }

    auto snap = rest_client_.market_data.execute(request);
    if (!snap) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        return;
    }

    const auto last_update_id = snap->lastUpdateId;

    // Step 4: drop buffered events where u < lastUpdateId
    buffer_.erase(std::remove_if(buffer_.begin(),
                                 buffer_.end(),
                                 [last_update_id](const types::depth_stream_event& ev) { return ev.final_update_id < last_update_id; }),
                  buffer_.end());

    // Step 5: first event should have U <= lastUpdateId AND u >= lastUpdateId
    if (buffer_.empty() || buffer_.front().first_update_id > last_update_id || buffer_.front().final_update_id < last_update_id) {
        // Cannot sync yet - will retry on next event
        return;
    }

    // Initialize book from snapshot
    book_ = {};
    book_.last_update_id = last_update_id;
    for (const auto& level : snap->bids) {
        if (!level.quantity.is_zero()) {
            book_.bids[level.price] = level.quantity;
        }
    }
    for (const auto& level : snap->asks) {
        if (!level.quantity.is_zero()) {
            book_.asks[level.price] = level.quantity;
        }
    }

    // Apply buffered events in sequence
    last_u_ = last_update_id;
    for (const auto& event : buffer_) {
        if (event.final_update_id <= last_update_id) {
            last_u_ = event.final_update_id;
            continue;
        }
        apply_event(event);
        last_u_ = event.final_update_id;
    }
    buffer_.clear();
    synced_ = true;

    if (on_snapshot_) {
        on_snapshot_(book_);
    }
}

void
local_order_book::apply_event(const types::depth_stream_event& event)
{
    apply_levels(event.bids, book_.bids);
    apply_levels(event.asks, book_.asks);
    book_.last_update_id = event.final_update_id;
}

template<class Compare>
void
local_order_book::apply_levels(const std::vector<types::price_level>& levels,
                               std::map<types::decimal, types::decimal, Compare>& side)
{
    for (const auto& level : levels) {
        // Step 7: if quantity=0, remove price level
        if (level.quantity.is_zero()) {
            side.erase(level.price);
        } else {
            side[level.price] = level.quantity;
        }
    }
}

// Explicit instantiations for bid (descending) and ask (ascending) sides.
template void local_order_book::apply_levels(const std::vector<types::price_level>&,
                                             std::map<types::decimal, types::decimal, std::greater<>>&);
template void local_order_book::apply_levels(const std::vector<types::price_level>&,
                                             std::map<types::decimal, types::decimal, std::less<>>&);

} // namespace binapi2::fapi::streams
