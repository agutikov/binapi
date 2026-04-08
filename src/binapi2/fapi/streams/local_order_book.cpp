// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the async local order book synchronization algorithm.

#include <binapi2/fapi/streams/local_order_book.hpp>

#include <binapi2/fapi/detail/json_opts.hpp>

#include <glaze/glaze.hpp>

#include <algorithm>

namespace binapi2::fapi::streams {

local_order_book::local_order_book(market_streams& streams, rest::pipeline& rest)
    : streams_(streams), rest_(rest)
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
local_order_book::apply_levels(const std::vector<types::price_level_t>& levels,
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

template void local_order_book::apply_levels(const std::vector<types::price_level_t>&,
                                             std::map<types::decimal_t, types::decimal_t, std::greater<>>&);
template void local_order_book::apply_levels(const std::vector<types::price_level_t>&,
                                             std::map<types::decimal_t, types::decimal_t, std::less<>>&);

boost::cobalt::task<result<void>>
local_order_book::async_run(types::symbol_t symbol, int depth_limit)
{
    running_ = true;

    // Step 1: connect to diff depth stream
    std::string symbol_lower = symbol.str();
    std::ranges::transform(symbol_lower, symbol_lower.begin(), ::tolower);
    const auto target = streams_.configuration().stream_base_target
        + "/" + symbol_lower + "@depth@100ms";

    auto conn = co_await streams_.async_connect(target);
    if (!conn) co_return result<void>::failure(conn.err);

    std::vector<types::depth_stream_event_t> buffer;
    bool synced = false;
    std::uint64_t last_u = 0;

    while (running_) {
        auto msg = co_await streams_.async_read_text();
        if (!msg) co_return result<void>::failure(msg.err);

        // Parse depth event
        types::depth_stream_event_t event{};
        glz::context ctx{};
        if (glz::read<fapi::detail::json_read_opts>(event, *msg, ctx)) {
            continue;  // skip unparseable frames
        }

        if (!synced) {
            buffer.push_back(event);

            // Fetch REST snapshot on first buffered event
            if (buffer.size() == 1) {
                types::order_book_request_t req;
                req.symbol = symbol;
                req.limit = depth_limit;
                auto snap = co_await rest_.async_execute(req);
                if (!snap) continue;  // retry on next event

                const auto snapshot_id = snap->lastUpdateId;

                // Discard events before snapshot
                std::erase_if(buffer, [snapshot_id](const auto& ev) {
                    return ev.final_update_id < snapshot_id;
                });

                // Verify first event brackets snapshot
                if (buffer.empty()
                    || buffer.front().first_update_id > snapshot_id
                    || buffer.front().final_update_id < snapshot_id) {
                    buffer.clear();
                    continue;  // retry
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
                    for (const auto& ev : buffer) {
                        if (ev.final_update_id <= snapshot_id) {
                            last_u = ev.final_update_id;
                            continue;
                        }
                        apply_event(ev);
                        last_u = ev.final_update_id;
                    }

                    if (on_snapshot_) on_snapshot_(book_);
                }

                buffer.clear();
                synced = true;
            }
        } else {
            // Step 6: validate sequence continuity
            if (event.prev_final_update_id != last_u) {
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

    co_await streams_.async_close();
    co_return result<void>::success();
}

} // namespace binapi2::fapi::streams
