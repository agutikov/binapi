// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file endpoint_traits.hpp
/// @brief Compile-time mapping from WebSocket API request types to RPC methods, response types, and auth mode.

#pragma once

#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/trade.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/generated_methods.hpp>

#include <string_view>
#include <vector>

namespace binapi2::fapi::websocket_api {

/// @brief Authentication mode for WS API requests.
enum class ws_auth_mode {
    inject,         ///< inject_auth(request) — for signed request types with user params.
    none,           ///< No auth — request sent as-is (unsigned market data queries).
    signed_base,    ///< make_signed_request_base() — parameterless signed endpoints.
    api_key_only,   ///< {apiKey: ...} — user data stream management.
};

/// @brief Primary template; intentionally undefined.
template<class Request>
struct endpoint_traits;

// Helper: detect if a trait has an explicit auth member. Default is inject for
// types inheriting websocket_api_signed_request_t, none otherwise.
namespace detail {
template<class T, class = void>
struct has_auth_member : std::false_type {};
template<class T>
struct has_auth_member<T, std::void_t<decltype(T::auth)>> : std::true_type {};

template<class Request>
constexpr ws_auth_mode resolve_auth()
{
    using traits = endpoint_traits<Request>;
    if constexpr (has_auth_member<traits>::value) {
        return traits::auth;
    } else if constexpr (std::is_base_of_v<types::websocket_api_signed_request_t, Request>) {
        return ws_auth_mode::inject;
    } else {
        return ws_auth_mode::none;
    }
}
} // namespace detail

// --- Market data (unsigned) ---

template<> struct endpoint_traits<types::websocket_api_book_ticker_request_t> {
    using response_type_t = types::book_ticker_t;
    static constexpr auto& method = ticker_book_method;
};
template<> struct endpoint_traits<types::websocket_api_price_ticker_request_t> {
    using response_type_t = types::price_ticker_t;
    static constexpr auto& method = ticker_price_method;
};

// --- Trade (inject_auth — inherits websocket_api_signed_request_t) ---

template<> struct endpoint_traits<types::websocket_api_order_place_request_t> {
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_place_method;
};
template<> struct endpoint_traits<types::websocket_api_order_query_request_t> {
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_status_method;
};
template<> struct endpoint_traits<types::websocket_api_order_cancel_request_t> {
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_cancel_method;
};
template<> struct endpoint_traits<types::websocket_api_order_modify_request_t> {
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_modify_method;
};
template<> struct endpoint_traits<types::websocket_api_position_request_t> {
    using response_type_t = std::vector<types::position_risk_t>;
    static constexpr auto& method = account_position_method;
};
template<> struct endpoint_traits<types::websocket_api_algo_order_place_request_t> {
    using response_type_t = types::algo_order_response_t;
    static constexpr auto& method = algo_order_place_method;
};
template<> struct endpoint_traits<types::websocket_api_algo_order_cancel_request_t> {
    using response_type_t = types::code_msg_response_t;
    static constexpr auto& method = algo_order_cancel_method;
};

// --- Parameterless signed (make_signed_request_base) ---

template<> struct endpoint_traits<types::ws_account_status_request_t> {
    using response_type_t = types::account_information_t;
    static constexpr auto& method = account_status_method;
    static constexpr auto auth = ws_auth_mode::signed_base;
};
template<> struct endpoint_traits<types::ws_account_status_v2_request_t> {
    using response_type_t = types::account_information_t;
    static constexpr auto& method = account_status_v2_method;
    static constexpr auto auth = ws_auth_mode::signed_base;
};
template<> struct endpoint_traits<types::ws_account_balance_request_t> {
    using response_type_t = std::vector<types::futures_account_balance_t>;
    static constexpr auto& method = account_balance_method;
    static constexpr auto auth = ws_auth_mode::signed_base;
};

// --- API key only (user data stream management) ---

template<> struct endpoint_traits<types::ws_user_data_stream_start_request_t> {
    using response_type_t = types::websocket_api_listen_key_result_t;
    static constexpr auto& method = user_data_stream_start_method;
    static constexpr auto auth = ws_auth_mode::api_key_only;
};
template<> struct endpoint_traits<types::ws_user_data_stream_ping_request_t> {
    using response_type_t = types::websocket_api_listen_key_result_t;
    static constexpr auto& method = user_data_stream_ping_method;
    static constexpr auto auth = ws_auth_mode::api_key_only;
};
template<> struct endpoint_traits<types::ws_user_data_stream_stop_request_t> {
    using response_type_t = types::empty_response_t;
    static constexpr auto& method = user_data_stream_stop_method;
    static constexpr auto auth = ws_auth_mode::api_key_only;
};

// --- Concept ---

template<class T>
concept has_ws_endpoint_traits = requires {
    typename endpoint_traits<T>::response_type_t;
    { endpoint_traits<T>::method } -> std::convertible_to<std::string_view>;
};

} // namespace binapi2::fapi::websocket_api
