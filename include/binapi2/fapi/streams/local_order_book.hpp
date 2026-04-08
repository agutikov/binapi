// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file local_order_book.hpp
/// @brief Async locally-maintained order book synchronized via WebSocket depth stream.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/symbol.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <boost/cobalt/task.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Snapshot of the locally maintained order book.
struct order_book_snapshot
{
    using decimal = types::decimal_t;
    std::uint64_t last_update_id{};
    std::map<decimal, decimal, std::greater<>> bids{};
    std::map<decimal, decimal, std::less<>> asks{};
};

/// @brief Async locally maintained order book.
///
/// Implements the Binance synchronization algorithm as a coroutine:
///  1. Connect to @depth@100ms stream
///  2. Buffer events, fetch REST snapshot
///  3. Reconcile buffered events with snapshot
///  4. Apply live events, detect gaps, re-sync
class local_order_book
{
public:
    using snapshot_callback = std::function<void(const order_book_snapshot&)>;

    local_order_book(market_streams& streams, rest::pipeline& rest);

    /// @brief Run the order book sync loop as a coroutine.
    ///
    /// Connects to the stream, fetches snapshot, applies events.
    /// Runs until stop() is called or an error occurs.
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_run(types::symbol_t symbol, int depth_limit);

    /// @brief Signal the run loop to stop.
    void stop();

    /// @brief Thread-safe snapshot of current book state.
    [[nodiscard]] order_book_snapshot snapshot() const;

    void set_snapshot_callback(snapshot_callback cb);

private:
    void apply_event(const types::depth_stream_event_t& event);

    template<class Compare>
    void apply_levels(const std::vector<types::price_level_t>& levels,
                      std::map<types::decimal_t, types::decimal_t, Compare>& side);

    market_streams& streams_;
    rest::pipeline& rest_;

    mutable std::mutex mutex_;
    order_book_snapshot book_;
    snapshot_callback on_snapshot_;
    std::atomic<bool> running_{false};
};

} // namespace binapi2::fapi::streams
