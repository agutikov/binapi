// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
struct websocket_api_error_t
{
    int code{};
    std::string msg{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
struct session_logon_request_t
{
    std::string apiKey{};
    std::uint64_t timestamp{};
    std::uint64_t recvWindow{};
    std::string signature{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
struct websocket_api_status_t
{
    int status{};
    std::string id{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
struct session_logon_result_t
{
    std::optional<std::string> apiKey{};
    std::optional<timestamp_ms_t> authorizedSince{};
    std::optional<timestamp_ms_t> connectedSince{};
    std::optional<bool> returnRateLimits{};
    std::optional<timestamp_ms_t> serverTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
struct websocket_api_signed_request_t
{
    std::string apiKey{};
    std::uint64_t timestamp{};
    std::optional<std::uint64_t> recvWindow{};
    std::string signature{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Order.md
struct websocket_api_order_place_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    order_type_t type{};
    std::optional<time_in_force_t> timeInForce{};
    decimal_t quantity{};
    std::optional<decimal_t> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<decimal_t> stopPrice{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Query-Order.md
struct websocket_api_order_query_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Order.md
struct websocket_api_order_cancel_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Order-Book-Ticker.md
struct websocket_api_book_ticker_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Price-Ticker.md
struct websocket_api_price_ticker_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Modify-Order.md
struct websocket_api_order_modify_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    order_side_t side{};
    decimal_t quantity{};
    decimal_t price{};
    std::optional<price_match_t> priceMatch{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Information.md
struct websocket_api_position_request_t : websocket_api_signed_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Algo-Order.md
struct websocket_api_algo_order_place_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    order_type_t type{};
    std::optional<time_in_force_t> timeInForce{};
    decimal_t quantity{};
    std::optional<decimal_t> price{};
    std::optional<decimal_t> triggerPrice{};
    algo_type_t algoType{};
    std::optional<std::string> workingType{};
    std::optional<std::string> priceMatch{};
    std::optional<std::string> closePosition{};
    std::optional<std::string> priceProtect{};
    std::optional<std::string> reduceOnly{};
    std::optional<decimal_t> activationPrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<std::string> clientAlgoId{};
    std::optional<std::string> newOrderRespType{};
    std::optional<std::string> selfTradePreventionMode{};
    std::optional<std::uint64_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Algo-Order.md
struct websocket_api_algo_order_cancel_request_t : websocket_api_signed_request_t
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream-Wsp.md
struct websocket_api_user_data_stream_request_t
{
    std::string apiKey{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream-Wsp.md
struct websocket_api_listen_key_result_t
{
    std::string listenKey{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
template<typename T>
struct websocket_api_response_t
{
    std::string id{};
    int status{};
    std::optional<T> result{};
    std::optional<std::vector<rate_limit_t>> rateLimits{};
    std::optional<websocket_api_error_t> error{};
};

// --- Parameterless WS API request types ---

struct ws_account_status_request_t { };
struct ws_account_status_v2_request_t { };
struct ws_account_balance_request_t { };
struct ws_user_data_stream_start_request_t { };
struct ws_user_data_stream_ping_request_t { };
struct ws_user_data_stream_stop_request_t { };

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_error_t>
{
    using T = binapi2::fapi::types::websocket_api_error_t;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg);
};

template<>
struct glz::meta<binapi2::fapi::types::session_logon_request_t>
{
    using T = binapi2::fapi::types::session_logon_request_t;
    static constexpr auto value =
        object("apiKey", &T::apiKey, "timestamp", &T::timestamp, "recvWindow", &T::recvWindow, "signature", &T::signature);
};

template<>
struct glz::meta<binapi2::fapi::types::session_logon_result_t>
{
    using T = binapi2::fapi::types::session_logon_result_t;
    static constexpr auto value = object("apiKey",
                                         &T::apiKey,
                                         "authorizedSince",
                                         &T::authorizedSince,
                                         "connectedSince",
                                         &T::connectedSince,
                                         "returnRateLimits",
                                         &T::returnRateLimits,
                                         "serverTime",
                                         &T::serverTime);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_signed_request_t>
{
    using T = binapi2::fapi::types::websocket_api_signed_request_t;
    static constexpr auto value =
        object("apiKey", &T::apiKey, "timestamp", &T::timestamp, "recvWindow", &T::recvWindow, "signature", &T::signature);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_place_request_t>
{
    using T = binapi2::fapi::types::websocket_api_order_place_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_order_query_request_t>
{
    using T = binapi2::fapi::types::websocket_api_order_query_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_order_cancel_request_t>
{
    using T = binapi2::fapi::types::websocket_api_order_cancel_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_book_ticker_request_t>
{
    using T = binapi2::fapi::types::websocket_api_book_ticker_request_t;
    static constexpr auto value = object("symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_price_ticker_request_t>
{
    using T = binapi2::fapi::types::websocket_api_price_ticker_request_t;
    static constexpr auto value = object("symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_order_modify_request_t>
{
    using T = binapi2::fapi::types::websocket_api_order_modify_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_position_request_t>
{
    using T = binapi2::fapi::types::websocket_api_position_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_algo_order_place_request_t>
{
    using T = binapi2::fapi::types::websocket_api_algo_order_place_request_t;
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
                                         &T::workingType,
                                         "priceMatch",
                                         &T::priceMatch,
                                         "closePosition",
                                         &T::closePosition,
                                         "priceProtect",
                                         &T::priceProtect,
                                         "reduceOnly",
                                         &T::reduceOnly,
                                         "activatePrice",
                                         &T::activationPrice,
                                         "callbackRate",
                                         &T::callbackRate,
                                         "clientAlgoId",
                                         &T::clientAlgoId,
                                         "newOrderRespType",
                                         &T::newOrderRespType,
                                         "selfTradePreventionMode",
                                         &T::selfTradePreventionMode,
                                         "goodTillDate",
                                         &T::goodTillDate);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_algo_order_cancel_request_t>
{
    using T = binapi2::fapi::types::websocket_api_algo_order_cancel_request_t;
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
struct glz::meta<binapi2::fapi::types::websocket_api_user_data_stream_request_t>
{
    using T = binapi2::fapi::types::websocket_api_user_data_stream_request_t;
    static constexpr auto value = object("apiKey", &T::apiKey);
};

template<>
struct glz::meta<binapi2::fapi::types::websocket_api_listen_key_result_t>
{
    using T = binapi2::fapi::types::websocket_api_listen_key_result_t;
    static constexpr auto value = object("listenKey", &T::listenKey);
};

template<typename T>
struct glz::meta<binapi2::fapi::types::websocket_api_response_t<T>>
{
    using U = binapi2::fapi::types::websocket_api_response_t<T>;
    static constexpr auto value =
        object("id", &U::id, "status", &U::status, "result", &U::result, "rateLimits", &U::rateLimits, "error", &U::error);
};
