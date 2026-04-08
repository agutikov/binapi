// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.
//
// Maps WS API request types to their RPC method names and response types.

/// @file endpoint_traits.hpp
/// @brief Compile-time mapping from WebSocket API request types to RPC methods and response types.

#pragma once

#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/trade.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/generated_methods.hpp>

#include <string_view>
#include <vector>

namespace binapi2::fapi::websocket_api {

/// @brief Traits class mapping a WebSocket API request type to its RPC method
///        name and expected response type.
///
/// Each specialization must provide:
/// - @c response_type_t : the deserialized response payload type.
/// - @c method        : a @c constexpr reference to the RPC method name string.
///
/// @tparam Request The request parameter struct to look up.
template<class Request>
struct endpoint_traits;

// --- Market data ---

/// @brief Traits for book ticker requests.
template<>
struct endpoint_traits<types::websocket_api_book_ticker_request_t>
{
    using response_type_t = types::book_ticker_t;
    static constexpr auto& method = ticker_book_method;
};

/// @brief Traits for price ticker requests.
template<>
struct endpoint_traits<types::websocket_api_price_ticker_request_t>
{
    using response_type_t = types::price_ticker_t;
    static constexpr auto& method = ticker_price_method;
};

// --- Trade ---

/// @brief Traits for order placement requests.
template<>
struct endpoint_traits<types::websocket_api_order_place_request_t>
{
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_place_method;
};

/// @brief Traits for order status query requests.
template<>
struct endpoint_traits<types::websocket_api_order_query_request_t>
{
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_status_method;
};

/// @brief Traits for order cancellation requests.
template<>
struct endpoint_traits<types::websocket_api_order_cancel_request_t>
{
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_cancel_method;
};

/// @brief Traits for order modification requests.
template<>
struct endpoint_traits<types::websocket_api_order_modify_request_t>
{
    using response_type_t = types::order_response_t;
    static constexpr auto& method = order_modify_method;
};

/// @brief Traits for position risk query requests.
template<>
struct endpoint_traits<types::websocket_api_position_request_t>
{
    using response_type_t = std::vector<types::position_risk_t>;
    static constexpr auto& method = account_position_method;
};

/// @brief Traits for algo order placement requests.
template<>
struct endpoint_traits<types::websocket_api_algo_order_place_request_t>
{
    using response_type_t = types::algo_order_response_t;
    static constexpr auto& method = algo_order_place_method;
};

/// @brief Traits for algo order cancellation requests.
template<>
struct endpoint_traits<types::websocket_api_algo_order_cancel_request_t>
{
    using response_type_t = types::code_msg_response_t;
    static constexpr auto& method = algo_order_cancel_method;
};

/// @brief Concept constraining types that have a valid @ref endpoint_traits specialization.
///
/// A type satisfies this concept when @c endpoint_traits<T> provides both a
/// @c response_type_t alias and a @c method member convertible to @c std::string_view.
template<class T>
concept has_ws_endpoint_traits = requires {
    typename endpoint_traits<T>::response_type_t;
    { endpoint_traits<T>::method } -> std::convertible_to<std::string_view>;
};

} // namespace binapi2::fapi::websocket_api
