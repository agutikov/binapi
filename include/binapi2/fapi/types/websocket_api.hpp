// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

struct websocket_api_error
{
    int code{};
    std::string msg{};
};

struct session_logon_request
{
    std::string apiKey{};
    std::uint64_t timestamp{};
    std::uint64_t recvWindow{};
    std::string signature{};
};

struct websocket_api_status
{
    int status{};
    std::string id{};
};

struct session_logon_result
{
    std::optional<std::string> apiKey{};
    std::optional<bool> authorizedSinceConnect{};
    std::optional<bool> returnRateLimits{};
    std::optional<std::string> serverTime{};
};

struct websocket_api_signed_request
{
    std::string apiKey{};
    std::uint64_t timestamp{};
    std::optional<std::uint64_t> recvWindow{};
    std::string signature{};
};

struct websocket_api_order_place_request : websocket_api_signed_request
{
    std::string symbol{};
    std::string side{};
    std::string type{};
    std::optional<std::string> timeInForce{};
    std::string quantity{};
    std::optional<std::string> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<std::string> stopPrice{};
};

struct websocket_api_order_query_request : websocket_api_signed_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct websocket_api_order_cancel_request : websocket_api_signed_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct websocket_api_book_ticker_request
{
    std::optional<std::string> symbol{};
};

struct websocket_api_price_ticker_request
{
    std::optional<std::string> symbol{};
};

struct websocket_api_order_modify_request : websocket_api_signed_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::string side{};
    std::string quantity{};
    std::string price{};
    std::optional<std::string> priceMatch{};
};

struct websocket_api_position_request : websocket_api_signed_request
{
    std::optional<std::string> symbol{};
};

struct websocket_api_algo_order_place_request : websocket_api_signed_request
{
    std::string symbol{};
    std::string side{};
    std::optional<std::string> positionSide{};
    std::optional<std::string> type{};
    std::optional<std::string> timeInForce{};
    std::string quantity{};
    std::optional<std::string> price{};
    std::optional<std::string> triggerPrice{};
    std::string algoType{};
    std::optional<std::string> workingType{};
};

struct websocket_api_algo_order_cancel_request : websocket_api_signed_request
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

struct websocket_api_user_data_stream_request
{
    std::string apiKey{};
};

struct websocket_api_listen_key_result
{
    std::string listenKey{};
};

template<typename T>
struct websocket_api_response
{
    std::string id{};
    int status{};
    std::optional<T> result{};
    std::optional<std::vector<rate_limit>> rateLimits{};
    std::optional<websocket_api_error> error{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_error>
{
    using T = binapi2::fapi::types::websocket_api_error;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg);
};

template<>
struct glz::meta<binapi2::fapi::types::session_logon_request>
{
    using T = binapi2::fapi::types::session_logon_request;
    static constexpr auto value =
        object("apiKey", &T::apiKey, "timestamp", &T::timestamp, "recvWindow", &T::recvWindow, "signature", &T::signature);
};

template<>
struct glz::meta<binapi2::fapi::types::session_logon_result>
{
    using T = binapi2::fapi::types::session_logon_result;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "authorizedSinceConnect",
                                         &T::authorizedSinceConnect,
                                         "returnRateLimits",
                                         &T::returnRateLimits,
                                         "serverTime",
                                         &T::serverTime);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_signed_request>
{
    using T = binapi2::fapi::types::websocket_api_signed_request;
    static constexpr auto value =
        object("apiKey", &T::apiKey, "timestamp", &T::timestamp, "recvWindow", &T::recvWindow, "signature", &T::signature);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_place_request>
{
    using T = binapi2::fapi::types::websocket_api_order_place_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol,
                                         "side",
                                         &T::side,
                                         "type",
                                         &T::type,
                                         "timeInForce",
                                         &T::timeInForce,
                                         "quantity",
                                         &T::quantity,
                                         "price",
                                         &T::price,
                                         "newClientOrderId",
                                         &T::newClientOrderId,
                                         "stopPrice",
                                         &T::stopPrice);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_query_request>
{
    using T = binapi2::fapi::types::websocket_api_order_query_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol,
                                         "orderId",
                                         &T::orderId,
                                         "origClientOrderId",
                                         &T::origClientOrderId);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_cancel_request>
{
    using T = binapi2::fapi::types::websocket_api_order_cancel_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol,
                                         "orderId",
                                         &T::orderId,
                                         "origClientOrderId",
                                         &T::origClientOrderId);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_book_ticker_request>
{
    using T = binapi2::fapi::types::websocket_api_book_ticker_request;
    static constexpr auto value = object("symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_price_ticker_request>
{
    using T = binapi2::fapi::types::websocket_api_price_ticker_request;
    static constexpr auto value = object("symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_modify_request>
{
    using T = binapi2::fapi::types::websocket_api_order_modify_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol,
                                         "orderId",
                                         &T::orderId,
                                         "origClientOrderId",
                                         &T::origClientOrderId,
                                         "side",
                                         &T::side,
                                         "quantity",
                                         &T::quantity,
                                         "price",
                                         &T::price,
                                         "priceMatch",
                                         &T::priceMatch);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_position_request>
{
    using T = binapi2::fapi::types::websocket_api_position_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_algo_order_place_request>
{
    using T = binapi2::fapi::types::websocket_api_algo_order_place_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "symbol",
                                         &T::symbol,
                                         "side",
                                         &T::side,
                                         "positionSide",
                                         &T::positionSide,
                                         "type",
                                         &T::type,
                                         "timeInForce",
                                         &T::timeInForce,
                                         "quantity",
                                         &T::quantity,
                                         "price",
                                         &T::price,
                                         "triggerPrice",
                                         &T::triggerPrice,
                                         "algoType",
                                         &T::algoType,
                                         "workingType",
                                         &T::workingType);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_algo_order_cancel_request>
{
    using T = binapi2::fapi::types::websocket_api_algo_order_cancel_request;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "timestamp",
                                         &T::timestamp,
                                         "recvWindow",
                                         &T::recvWindow,
                                         "signature",
                                         &T::signature,
                                         "algoId",
                                         &T::algoId,
                                         "clientAlgoId",
                                         &T::clientAlgoId);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_user_data_stream_request>
{
    using T = binapi2::fapi::types::websocket_api_user_data_stream_request;
    static constexpr auto value = object("apiKey", &T::apiKey);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_listen_key_result>
{
    using T = binapi2::fapi::types::websocket_api_listen_key_result;
    static constexpr auto value = object("listenKey", &T::listenKey);
};

template<typename T>
struct glz::meta<binapi2::fapi::types::websocket_api_response<T>>
{
    using U = binapi2::fapi::types::websocket_api_response<T>;
    static constexpr auto value =
        object("id", &U::id, "status", &U::status, "result", &U::result, "rateLimits", &U::rateLimits, "error", &U::error);
};
