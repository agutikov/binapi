// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file local_order_book.hpp
/// @brief Thread-safe locally-maintained order book synchronized via WebSocket depth stream.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/decimal.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::streams {

class market_streams;

/// @brief Snapshot of the locally maintained order book at a point in time.
struct order_book_snapshot
{
    using decimal = types::decimal;
    std::uint64_t last_update_id{};                         ///< Last update ID reflected in this snapshot.
    std::map<decimal, decimal, std::greater<>> bids{};      ///< Bid levels sorted by price descending (best bid first).
    std::map<decimal, decimal, std::less<>> asks{};          ///< Ask levels sorted by price ascending (best ask first).
};

/// @brief Locally maintained order book synchronized from the diff depth stream.
///
/// Implements the Binance order book synchronization algorithm:
///  1. Buffer incoming diff depth events from @ref market_streams.
///  2. Fetch a REST depth snapshot via the @ref client.
///  3. Discard buffered events with @c finalUpdateId <= snapshot's @c lastUpdateId.
///  4. Apply remaining and subsequent diff events incrementally.
///
/// All public accessors are protected by an internal mutex, making it safe to
/// call @ref snapshot from any thread while the update loop runs on another.
class local_order_book
{
public:
    /// @brief Callback invoked after each snapshot update with the new state.
    using snapshot_callback = std::function<void(const order_book_snapshot&)>;

    /// @brief Construct a local order book.
    /// @param streams Reference to a market_streams instance (provides diff depth events).
    /// @param rest_client Reference to a REST client (used to fetch the initial depth snapshot).
    local_order_book(market_streams& streams, client& rest_client) noexcept;

    /// @brief Start synchronizing the order book for a symbol.
    ///
    /// Connects to the diff depth stream, fetches an initial snapshot, and begins
    /// applying incremental updates.
    ///
    /// @param symbol      Trading pair symbol (e.g. "BTCUSDT").
    /// @param depth_limit Maximum number of price levels to maintain (default: 1000).
    /// @return A result indicating success or an error during initial synchronization.
    [[nodiscard]] result<void> start(const std::string& symbol, int depth_limit = 1000);

    /// @brief Stop the synchronization loop and reset internal state.
    void stop();

    /// @brief Obtain a thread-safe copy of the current order book state.
    /// @return A snapshot of the current bids, asks, and last update ID.
    [[nodiscard]] order_book_snapshot snapshot() const;

    /// @brief Register a callback invoked after each incremental update.
    /// @param callback The callback to invoke with the updated snapshot.
    void set_snapshot_callback(snapshot_callback callback);

private:
    /// @brief Fetch the initial depth snapshot from the REST API and seed the book.
    void fetch_snapshot();

    /// @brief Apply a single diff depth event to the local book.
    /// @param event The depth stream event containing bid/ask deltas.
    void apply_event(const types::depth_stream_event& event);

    /// @brief Apply price level deltas to the bid side.
    /// @param levels Price level updates (quantity of "0" removes the level).
    /// @param side   The bid-side map to update.
    template<class Compare>
    void apply_levels(const std::vector<types::price_level>& levels, std::map<types::decimal, types::decimal, Compare>& side);

    market_streams& streams_;         ///< Source of diff depth stream events.
    client& rest_client_;             ///< REST client for fetching the initial snapshot.
    std::string symbol_{};            ///< The symbol being tracked.
    int depth_limit_{ 1000 };         ///< Maximum number of price levels to maintain.

    mutable std::mutex mutex_{};      ///< Mutex protecting all mutable book state below.
    order_book_snapshot book_{};      ///< The current order book state.
    std::uint64_t last_u_{};          ///< Last processed event's first update ID.
    bool synced_{ false };            ///< Whether the book is synchronized with the snapshot.
    bool running_{ false };           ///< Whether the update loop is active.
    std::vector<types::depth_stream_event> buffer_{}; ///< Buffer for events received before snapshot.
    snapshot_callback on_snapshot_{}; ///< User callback invoked after each update.
};

} // namespace binapi2::fapi::streams
