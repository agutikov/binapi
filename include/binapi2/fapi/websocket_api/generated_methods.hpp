// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file generated_methods.hpp
/// @brief Compile-time registry of Binance WebSocket API RPC method name strings.
///
/// Each constant corresponds to a Binance Futures WebSocket API method and is
/// referenced by @ref endpoint_traits specializations and named client methods.

#pragma once

#include <string_view>

namespace binapi2::fapi::websocket_api {

inline constexpr std::string_view session_logon_method{ "session.logon" };              ///< Authenticate a WebSocket session.
inline constexpr std::string_view account_status_method{ "account.status" };            ///< Query account information (v1).
inline constexpr std::string_view account_balance_method{ "account.balance" };          ///< Query futures account balances.
inline constexpr std::string_view order_place_method{ "order.place" };                  ///< Place a new order.
inline constexpr std::string_view order_status_method{ "order.status" };                ///< Query order status.
inline constexpr std::string_view order_cancel_method{ "order.cancel" };                ///< Cancel an existing order.
inline constexpr std::string_view ticker_book_method{ "ticker.book" };                  ///< Best bid/ask book ticker.
inline constexpr std::string_view ticker_price_method{ "ticker.price" };                ///< Latest price ticker.
inline constexpr std::string_view order_modify_method{ "order.modify" };                ///< Modify an existing order.
inline constexpr std::string_view account_position_method{ "account.position" };        ///< Query position risk (v1).
inline constexpr std::string_view account_position_v2_method{ "v2/account.position" };  ///< Query position risk (v2).
inline constexpr std::string_view account_status_v2_method{ "v2/account.status" };      ///< Query account information (v2).
inline constexpr std::string_view algo_order_place_method{ "algoOrder.place" };         ///< Place an algorithmic order (VP, TWAP).
inline constexpr std::string_view algo_order_cancel_method{ "algoOrder.cancel" };       ///< Cancel an algorithmic order.
inline constexpr std::string_view user_data_stream_start_method{ "userDataStream.start" }; ///< Start a user data stream (obtain listen key).
inline constexpr std::string_view user_data_stream_ping_method{ "userDataStream.ping" };   ///< Keep-alive ping for a user data stream.
inline constexpr std::string_view user_data_stream_stop_method{ "userDataStream.stop" };   ///< Stop a user data stream.

} // namespace binapi2::fapi::websocket_api
