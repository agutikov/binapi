// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file subscriptions.hpp
/// @brief Subscription parameter types for Binance USD-M Futures WebSocket market streams.

#pragma once

#include <binapi2/fapi/types/enums.hpp>

#include <string>

namespace binapi2::fapi::streams {

/// @brief Parameters for subscribing to the aggregate trade stream.
struct aggregate_trade_subscription
{
    std::string symbol{}; ///< Trading pair symbol (e.g. "BTCUSDT").
};

/// @brief Parameters for subscribing to the book ticker stream.
struct book_ticker_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

/// @brief Parameters for subscribing to the mark price stream.
struct mark_price_subscription
{
    std::string symbol{};   ///< Trading pair symbol.
    bool every_1s{ false }; ///< If @c true, receive updates every 1 second instead of every 3 seconds.
};

/// @brief Parameters for subscribing to the diff book depth stream.
struct diff_book_depth_subscription
{
    std::string symbol{};          ///< Trading pair symbol.
    std::string speed{ "100ms" };  ///< Update speed: "100ms" or "250ms".
};

/// @brief Parameters for subscribing to the mini ticker stream.
struct mini_ticker_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

/// @brief Parameters for subscribing to the all-market mini ticker stream.
/// @note No fields required; subscribes to all symbols.
struct all_market_mini_ticker_subscription
{
};

/// @brief Parameters for subscribing to the 24hr ticker stream.
struct ticker_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

/// @brief Parameters for subscribing to the all-market 24hr ticker stream.
/// @note No fields required; subscribes to all symbols.
struct all_market_ticker_subscription
{
};

/// @brief Parameters for subscribing to the all-market book ticker stream.
/// @note No fields required; subscribes to all symbols.
struct all_book_ticker_subscription
{
};

/// @brief Parameters for subscribing to the liquidation order stream.
struct liquidation_order_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

/// @brief Parameters for subscribing to the all-market liquidation order stream.
/// @note No fields required; subscribes to all symbols.
struct all_market_liquidation_order_subscription
{
};

/// @brief Parameters for subscribing to the partial book depth stream.
struct partial_book_depth_subscription
{
    std::string symbol{};          ///< Trading pair symbol.
    int levels{ 5 };               ///< Number of price levels: 5, 10, or 20.
    std::string speed{ "250ms" };  ///< Update speed: "100ms" or "250ms".
};

/// @brief Parameters for subscribing to the kline/candlestick stream.
struct kline_subscription
{
    std::string symbol{};                                        ///< Trading pair symbol.
    types::kline_interval_t interval{ types::kline_interval_t::m1 }; ///< Candlestick interval (e.g. 1m, 5m, 1h).
};

/// @brief Parameters for subscribing to the continuous contract kline stream.
struct continuous_contract_kline_subscription
{
    std::string pair{};                                                    ///< Trading pair (e.g. "BTCUSDT").
    types::contract_type_t contract_type_t{ types::contract_type_t::perpetual }; ///< Contract type (perpetual, current quarter, etc.).
    types::kline_interval_t interval{ types::kline_interval_t::m1 };           ///< Candlestick interval.
};

/// @brief Parameters for subscribing to the all-market mark price stream.
struct all_market_mark_price_subscription
{
    bool every_1s{ false }; ///< If @c true, receive updates every 1 second instead of every 3 seconds.
};

/// @brief Parameters for subscribing to the composite index stream.
struct composite_index_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

/// @brief Parameters for subscribing to the contract info stream.
/// @note No fields required; subscribes to all contract info events.
struct contract_info_subscription
{
};

/// @brief Parameters for subscribing to the asset index stream.
struct asset_index_subscription
{
    std::string symbol{}; ///< Asset symbol.
};

/// @brief Parameters for subscribing to the all asset index stream.
/// @note No fields required; subscribes to all assets.
struct all_asset_index_subscription
{
};

/// @brief Parameters for subscribing to the trading session stream.
/// @note No fields required; subscribes to trading session status events.
struct trading_session_subscription
{
};

/// @brief Parameters for subscribing to the RPI diff book depth stream.
struct rpi_diff_book_depth_subscription
{
    std::string symbol{}; ///< Trading pair symbol.
};

} // namespace binapi2::fapi::streams
