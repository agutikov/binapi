// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>

namespace binapi2::fapi::types {

struct new_order_request
{
    std::string symbol{};
    order_side side{ order_side::buy };
    order_type type{ order_type::limit };
    std::optional<time_in_force> timeInForce{};
    std::string quantity{};
    std::optional<std::string> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<std::string> stopPrice{};
};

struct order_response
{
    std::string clientOrderId{};
    std::string cumQty{};
    std::string cumQuote{};
    std::string executedQty{};
    std::optional<std::string> cumBase{};
    std::uint64_t orderId{};
    std::string avgPrice{};
    std::string origQty{};
    std::optional<std::string> pair{};
    std::string price{};
    bool reduceOnly{};
    std::string side{};
    std::string positionSide{};
    std::string status{};
    std::string stopPrice{};
    bool closePosition{};
    std::string symbol{};
    std::string timeInForce{};
    std::string type{};
    std::optional<std::string> workingType{};
    std::optional<bool> priceProtect{};
    std::string origType{};
    std::optional<std::string> priceMatch{};
    std::optional<std::string> selfTradePreventionMode{};
    std::optional<std::uint64_t> goodTillDate{};
    std::optional<std::uint64_t> updateTime{};
};

struct modify_order_request
{
    std::string symbol{};
    order_side side{ order_side::buy };
    std::string quantity{};
    std::string price{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<std::string> priceMatch{};
};

struct cancel_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct query_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::order_response>
{
    using T = binapi2::fapi::types::order_response;
    static constexpr auto value = object("clientOrderId",
                                         &T::clientOrderId,
                                         "cumQty",
                                         &T::cumQty,
                                         "cumQuote",
                                         &T::cumQuote,
                                         "executedQty",
                                         &T::executedQty,
                                         "cumBase",
                                         &T::cumBase,
                                         "orderId",
                                         &T::orderId,
                                         "avgPrice",
                                         &T::avgPrice,
                                         "origQty",
                                         &T::origQty,
                                         "pair",
                                         &T::pair,
                                         "price",
                                         &T::price,
                                         "reduceOnly",
                                         &T::reduceOnly,
                                         "side",
                                         &T::side,
                                         "positionSide",
                                         &T::positionSide,
                                         "status",
                                         &T::status,
                                         "stopPrice",
                                         &T::stopPrice,
                                         "closePosition",
                                         &T::closePosition,
                                         "symbol",
                                         &T::symbol,
                                         "timeInForce",
                                         &T::timeInForce,
                                         "type",
                                         &T::type,
                                         "workingType",
                                         &T::workingType,
                                         "priceProtect",
                                         &T::priceProtect,
                                         "origType",
                                         &T::origType,
                                         "priceMatch",
                                         &T::priceMatch,
                                         "selfTradePreventionMode",
                                         &T::selfTradePreventionMode,
                                         "goodTillDate",
                                         &T::goodTillDate,
                                         "updateTime",
                                         &T::updateTime);
};
