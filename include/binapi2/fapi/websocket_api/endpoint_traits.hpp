// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.
//
// Maps WS API request types to their RPC method names and response types.

#pragma once

#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/trade.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>
#include <binapi2/fapi/websocket_api/generated_methods.hpp>

#include <string_view>
#include <vector>

namespace binapi2::fapi::websocket_api {

template<class Request>
struct endpoint_traits;

// --- Market data ---

template<>
struct endpoint_traits<types::websocket_api_book_ticker_request>
{
    using response_type = types::book_ticker;
    static constexpr auto& method = ticker_book_method;
};

template<>
struct endpoint_traits<types::websocket_api_price_ticker_request>
{
    using response_type = types::price_ticker;
    static constexpr auto& method = ticker_price_method;
};

// --- Trade ---

template<>
struct endpoint_traits<types::websocket_api_order_place_request>
{
    using response_type = types::order_response;
    static constexpr auto& method = order_place_method;
};

template<>
struct endpoint_traits<types::websocket_api_order_query_request>
{
    using response_type = types::order_response;
    static constexpr auto& method = order_status_method;
};

template<>
struct endpoint_traits<types::websocket_api_order_cancel_request>
{
    using response_type = types::order_response;
    static constexpr auto& method = order_cancel_method;
};

template<>
struct endpoint_traits<types::websocket_api_order_modify_request>
{
    using response_type = types::order_response;
    static constexpr auto& method = order_modify_method;
};

template<>
struct endpoint_traits<types::websocket_api_position_request>
{
    using response_type = std::vector<types::position_risk>;
    static constexpr auto& method = account_position_method;
};

template<>
struct endpoint_traits<types::websocket_api_algo_order_place_request>
{
    using response_type = types::algo_order_response;
    static constexpr auto& method = algo_order_place_method;
};

template<>
struct endpoint_traits<types::websocket_api_algo_order_cancel_request>
{
    using response_type = types::code_msg_response;
    static constexpr auto& method = algo_order_cancel_method;
};

template<class T>
concept has_ws_endpoint_traits = requires {
    typename endpoint_traits<T>::response_type;
    { endpoint_traits<T>::method } -> std::convertible_to<std::string_view>;
};

} // namespace binapi2::fapi::websocket_api
