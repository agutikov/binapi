// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <string_view>

namespace binapi2::fapi::websocket_api {

inline constexpr std::string_view session_logon_method{ "session.logon" };
inline constexpr std::string_view account_status_method{ "account.status" };
inline constexpr std::string_view account_balance_method{ "account.balance" };
inline constexpr std::string_view order_place_method{ "order.place" };
inline constexpr std::string_view order_status_method{ "order.status" };
inline constexpr std::string_view order_cancel_method{ "order.cancel" };
inline constexpr std::string_view ticker_book_method{ "ticker.book" };
inline constexpr std::string_view ticker_price_method{ "ticker.price" };
inline constexpr std::string_view order_modify_method{ "order.modify" };
inline constexpr std::string_view account_position_method{ "account.position" };
inline constexpr std::string_view account_position_v2_method{ "v2/account.position" };
inline constexpr std::string_view account_status_v2_method{ "v2/account.status" };
inline constexpr std::string_view algo_order_place_method{ "algoOrder.place" };
inline constexpr std::string_view algo_order_cancel_method{ "algoOrder.cancel" };
inline constexpr std::string_view user_data_stream_start_method{ "userDataStream.start" };
inline constexpr std::string_view user_data_stream_ping_method{ "userDataStream.ping" };
inline constexpr std::string_view user_data_stream_stop_method{ "userDataStream.stop" };

} // namespace binapi2::fapi::websocket_api
