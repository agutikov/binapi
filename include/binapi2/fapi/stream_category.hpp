// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_category.hpp
/// @brief Binance WebSocket URL category split (2026 migration).
///
/// Binance split `wss://fstream.binance.com` into category-routed paths:
///
///   - `/public` — high-frequency public market data
///       (bookTicker, partial/diff depth, RPI diff depth)
///   - `/market` — regular market data
///       (aggTrade, markPrice, kline, ticker, miniTicker, forceOrder,
///        compositeIndex, contractInfo, assetIndex, tradingSession, ...)
///   - `/private` — user data (listenKey-based)
///
/// A single combined-stream connection can only carry streams in one
/// category; mixing causes the server to silently drop frames for the
/// "wrong" category. Each `stream_traits<T>` carries a constexpr
/// `category` tag; the streaming primitives consume it at construction
/// time to pick the right URL prefix and at compile time to refuse
/// `subscribe` calls that mix categories.

#pragma once

#include <string_view>

namespace binapi2::fapi {

/// Binance WebSocket URL category. See file-level docs for routing rules.
enum class stream_category_e
{
    public_,  ///< `/public/...` — bookTicker, depth.
    market,   ///< `/market/...` — aggTrade, markPrice, kline, ticker, ...
    private_, ///< `/private/...` — user-data streams (listenKey).
};

/// Single-stream URL prefix for the category. The `<sym>@<topic>` suffix
/// is appended by the caller.
[[nodiscard]] constexpr std::string_view
category_single_target(stream_category_e c) noexcept
{
    switch (c) {
        case stream_category_e::public_: return "/public/ws";
        case stream_category_e::market:  return "/market/ws";
        case stream_category_e::private_: return "/private/ws";
    }
    return "/ws";
}

/// Combined-stream URL for the category. The `?streams=...` query is
/// appended by the caller (or, for SUBSCRIBE-driven dynamic streams,
/// the path itself is enough — the topic list is sent over the WS).
[[nodiscard]] constexpr std::string_view
category_combined_target(stream_category_e c) noexcept
{
    switch (c) {
        case stream_category_e::public_: return "/public/stream";
        case stream_category_e::market:  return "/market/stream";
        case stream_category_e::private_: return "/private/stream";
    }
    return "/stream";
}

/// Human-readable name of the category. Useful for logging.
[[nodiscard]] constexpr std::string_view
to_string(stream_category_e c) noexcept
{
    switch (c) {
        case stream_category_e::public_: return "public";
        case stream_category_e::market:  return "market";
        case stream_category_e::private_: return "private";
    }
    return "unknown";
}

} // namespace binapi2::fapi
