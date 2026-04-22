// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the async local order book synchronization algorithm.
/// Uses market_stream::subscribe() for typed depth events and
/// market_data_service for the REST snapshot.

#include <binapi2/fapi/order_book/local_order_book.hpp>

#include <algorithm>

namespace binapi2::fapi::order_book {

local_order_book::local_order_book(streams::market_stream& streams, rest::market_data_service& market_data)
    : streams_(streams), market_data_(market_data)
{
}

void local_order_book::stop() { running_ = false; }

order_book_snapshot
local_order_book::snapshot() const
{
    std::lock_guard lock(mutex_);
    return book_;
}

void
local_order_book::set_snapshot_callback(snapshot_callback cb)
{
    std::lock_guard lock(mutex_);
    on_snapshot_ = std::move(cb);
}

void
local_order_book::apply_event(const types::depth_stream_event_t& event)
{
    apply_levels(event.bids, book_.bids);
    apply_levels(event.asks, book_.asks);
    book_.last_update_id = event.final_update_id;
}

template<class Compare>
void
local_order_book::apply_levels(const types::depth_levels_t& levels,
                               std::map<types::decimal_t, types::decimal_t, Compare>& side)
{
    for (const auto& level : levels) {
        if (level.quantity.is_zero()) {
            side.erase(level.price);
        } else {
            side[level.price] = level.quantity;
        }
    }
}

template void local_order_book::apply_levels(const types::depth_levels_t&,
                                             std::map<types::decimal_t, types::decimal_t, std::greater<>>&);
template void local_order_book::apply_levels(const types::depth_levels_t&,
                                             std::map<types::decimal_t, types::decimal_t, std::less<>>&);

boost::cobalt::task<result<void>>
local_order_book::async_run(types::symbol_t symbol, int depth_limit)
{
    running_ = true;

    // Subscribe to diff depth stream — typed events, no raw parsing needed
    types::diff_book_depth_subscription sub;
    sub.symbol = symbol;
    sub.speed = "100ms";
    auto gen = streams_.subscribe(sub);

    std::vector<types::depth_stream_event_t> buffer;
    bool synced = false;
    std::uint64_t last_u = 0;

    // How many events to buffer before fetching a (re-)snapshot.
    // On the first sync attempt we fetch immediately (events_until_snap = 0).
    // After each failed bracket check we double the wait (up to a cap)
    // so that the buffer has time to grow and overlap the snapshot's
    // lastUpdateId window. This avoids the 82-snapshots-zero-sync
    // pathology on very liquid pairs (BTCUSDT) where the snapshot is
    // always stale by the time the REST call returns.
    constexpr std::size_t max_wait = 50;
    std::size_t events_until_snap = 0;

    while (running_ && gen) {
        auto ev_result = co_await gen;
        if (!ev_result) co_return result<void>::failure(ev_result.err);

        const auto& event = *ev_result;

        if (!synced) {
            buffer.push_back(event);

            // Wait for enough events before (re-)fetching the snapshot.
            if (events_until_snap > 0) {
                --events_until_snap;
                continue;
            }

            {
                types::order_book_request_t req;
                req.symbol = symbol;
                req.limit = depth_limit;
                auto snap = co_await market_data_.async_execute(req);
                if (!snap) {
                    events_until_snap = std::min(max_wait, std::max<std::size_t>(events_until_snap, 1) * 2);
                    continue;  // REST error — retry after more events
                }

                const auto snapshot_id = snap->lastUpdateId;

                // Discard events entirely before the snapshot.
                std::erase_if(buffer, [snapshot_id](const auto& ev) {
                    return ev.final_update_id < snapshot_id;
                });

                // Verify first remaining event brackets the snapshot:
                //   event.first_update_id <= snapshot_id <= event.final_update_id
                if (buffer.empty()
                    || buffer.front().first_update_id > snapshot_id
                    || buffer.front().final_update_id < snapshot_id) {
                    // Bracket check failed. Accumulate more events before
                    // retrying — exponential backoff capped at max_wait.
                    events_until_snap = std::min(max_wait, std::max<std::size_t>(events_until_snap, 1) * 2);
                    continue;
                }

                // Initialize book from snapshot
                {
                    std::lock_guard lock(mutex_);
                    book_ = {};
                    book_.last_update_id = snapshot_id;
                    for (const auto& level : snap->bids) {
                        if (!level.quantity.is_zero())
                            book_.bids[level.price] = level.quantity;
                    }
                    for (const auto& level : snap->asks) {
                        if (!level.quantity.is_zero())
                            book_.asks[level.price] = level.quantity;
                    }

                    // Apply buffered events
                    last_u = snapshot_id;
                    for (const auto& buffered : buffer) {
                        if (buffered.final_update_id <= snapshot_id) {
                            last_u = buffered.final_update_id;
                            continue;
                        }
                        apply_event(buffered);
                        last_u = buffered.final_update_id;
                    }

                    if (on_snapshot_) on_snapshot_(book_);
                }

                buffer.clear();
                synced = true;
            }
            // else: still accumulating events, waiting for next
            // need_snapshot cycle to try again.
        } else {
            // Validate sequence continuity
            if (event.prev_final_update_id != last_u) {
                // Gap detected — re-sync
                synced = false;
                buffer.clear();
                buffer.push_back(event);
                continue;
            }

            {
                std::lock_guard lock(mutex_);
                apply_event(event);
                last_u = event.final_update_id;
                if (on_snapshot_) on_snapshot_(book_);
            }
        }
    }

    co_return result<void>::success();
}

} // namespace binapi2::fapi::order_book
