#pragma once

#include <string_view>

namespace binapi2::umf::websocket_api {

inline constexpr std::string_view session_logon_method{"session.logon"};
inline constexpr std::string_view account_status_method{"account.status"};
inline constexpr std::string_view account_balance_method{"account.balance"};
inline constexpr std::string_view order_place_method{"order.place"};
inline constexpr std::string_view order_status_method{"order.status"};
inline constexpr std::string_view order_cancel_method{"order.cancel"};
inline constexpr std::string_view ticker_book_method{"ticker.book"};

} // namespace binapi2::umf::websocket_api
