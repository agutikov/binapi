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
#include <vector>

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

using test_new_order_request = new_order_request;

struct batch_orders_request
{
    std::string batchOrders{};
};

struct cancel_multiple_orders_request
{
    std::string symbol{};
    std::optional<std::vector<std::uint64_t>> orderIdList{};
    std::optional<std::vector<std::string>> origClientOrderIdList{};
};

struct cancel_all_open_orders_request
{
    std::string symbol{};
};

struct code_msg_response
{
    int code{};
    std::string msg{};
};

struct auto_cancel_request
{
    std::string symbol{};
    std::uint64_t countdownTime{};
};

struct auto_cancel_response
{
    std::string symbol{};
    std::uint64_t countdownTime{};
};

struct query_open_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct all_open_orders_request
{
    std::optional<std::string> symbol{};
};

struct all_orders_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct position_risk_v3
{
    std::string symbol{};
    std::string positionSide{};
    std::string positionAmt{};
    std::string entryPrice{};
    std::string breakEvenPrice{};
    std::string markPrice{};
    std::string unRealizedProfit{};
    std::string liquidationPrice{};
    std::string isolatedMargin{};
    std::string notional{};
    std::string marginAsset{};
    std::string isolatedWallet{};
    std::string initialMargin{};
    std::string maintMargin{};
    std::string positionInitialMargin{};
    std::string openOrderInitialMargin{};
    int adl{};
    std::string bidNotional{};
    std::string askNotional{};
    std::uint64_t updateTime{};
};

struct position_info_v3_request
{
    std::optional<std::string> symbol{};
};

struct adl_quantile_values
{
    int LONG{};
    int SHORT{};
    std::optional<int> BOTH{};
};

struct adl_quantile_entry
{
    std::string symbol{};
    adl_quantile_values adlQuantile{};
};

struct adl_quantile_request
{
    std::optional<std::string> symbol{};
};

struct force_orders_request
{
    std::optional<std::string> symbol{};
    std::optional<std::string> autoCloseType{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct account_trade_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<std::uint64_t> fromId{};
    std::optional<int> limit{};
};

struct account_trade_entry
{
    std::uint64_t id{};
    std::uint64_t orderId{};
    std::string symbol{};
    std::string side{};
    std::string positionSide{};
    std::string price{};
    std::string qty{};
    std::string quoteQty{};
    std::string commission{};
    std::string commissionAsset{};
    std::uint64_t time{};
    bool buyer{};
    bool maker{};
    std::string realizedPnl{};
};

struct change_position_mode_request
{
    std::string dualSidePosition{};
};

struct change_multi_assets_mode_request
{
    std::string multiAssetsMargin{};
};

struct change_leverage_request
{
    std::string symbol{};
    int leverage{};
};

struct change_leverage_response
{
    int leverage{};
    std::string maxNotionalValue{};
    std::string symbol{};
};

struct change_margin_type_request
{
    std::string symbol{};
    std::string marginType{};
};

struct modify_isolated_margin_request
{
    std::string symbol{};
    std::optional<std::string> positionSide{};
    std::string amount{};
    int type{};
};

struct modify_isolated_margin_response
{
    int code{};
    std::string msg{};
    std::string amount{};
    int type{};
};

struct position_margin_history_request
{
    std::string symbol{};
    std::optional<int> type{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct position_margin_history_entry
{
    std::string symbol{};
    int type{};
    std::string deltaType{};
    std::string amount{};
    std::string asset{};
    std::uint64_t time{};
    std::string positionSide{};
};

struct order_modify_history_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct new_algo_order_request
{
    std::string symbol{};
    std::string side{};
    std::optional<std::string> positionSide{};
    std::string type{};
    std::optional<std::string> timeInForce{};
    std::string quantity{};
    std::optional<std::string> price{};
    std::optional<std::string> triggerPrice{};
    std::optional<std::string> workingType{};
    std::string algoType{};
    std::optional<std::string> clientAlgoId{};
};

struct algo_order_response
{
    std::uint64_t algoId{};
    std::optional<std::string> clientAlgoId{};
    std::string algoType{};
    std::string symbol{};
    std::string side{};
    std::optional<std::string> positionSide{};
    std::string algoStatus{};
};

struct cancel_algo_order_request
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

struct query_algo_order_request
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

struct all_algo_orders_request
{
    std::string symbol{};
    std::optional<std::uint64_t> algoId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

struct tradfi_perps_request
{};

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

template<>
struct glz::meta<binapi2::fapi::types::code_msg_response>
{
    using T = binapi2::fapi::types::code_msg_response;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg);
};

template<>
struct glz::meta<binapi2::fapi::types::auto_cancel_response>
{
    using T = binapi2::fapi::types::auto_cancel_response;
    static constexpr auto value = object("symbol", &T::symbol, "countdownTime", &T::countdownTime);
};

template<>
struct glz::meta<binapi2::fapi::types::position_risk_v3>
{
    using T = binapi2::fapi::types::position_risk_v3;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "positionSide",
                                         &T::positionSide,
                                         "positionAmt",
                                         &T::positionAmt,
                                         "entryPrice",
                                         &T::entryPrice,
                                         "breakEvenPrice",
                                         &T::breakEvenPrice,
                                         "markPrice",
                                         &T::markPrice,
                                         "unRealizedProfit",
                                         &T::unRealizedProfit,
                                         "liquidationPrice",
                                         &T::liquidationPrice,
                                         "isolatedMargin",
                                         &T::isolatedMargin,
                                         "notional",
                                         &T::notional,
                                         "marginAsset",
                                         &T::marginAsset,
                                         "isolatedWallet",
                                         &T::isolatedWallet,
                                         "initialMargin",
                                         &T::initialMargin,
                                         "maintMargin",
                                         &T::maintMargin,
                                         "positionInitialMargin",
                                         &T::positionInitialMargin,
                                         "openOrderInitialMargin",
                                         &T::openOrderInitialMargin,
                                         "adl",
                                         &T::adl,
                                         "bidNotional",
                                         &T::bidNotional,
                                         "askNotional",
                                         &T::askNotional,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::adl_quantile_values>
{
    using T = binapi2::fapi::types::adl_quantile_values;
    static constexpr auto value = object("LONG", &T::LONG, "SHORT", &T::SHORT, "BOTH", &T::BOTH);
};

template<>
struct glz::meta<binapi2::fapi::types::adl_quantile_entry>
{
    using T = binapi2::fapi::types::adl_quantile_entry;
    static constexpr auto value = object("symbol", &T::symbol, "adlQuantile", &T::adlQuantile);
};

template<>
struct glz::meta<binapi2::fapi::types::account_trade_entry>
{
    using T = binapi2::fapi::types::account_trade_entry;
    static constexpr auto value = object("id",
                                         &T::id,
                                         "orderId",
                                         &T::orderId,
                                         "symbol",
                                         &T::symbol,
                                         "side",
                                         &T::side,
                                         "positionSide",
                                         &T::positionSide,
                                         "price",
                                         &T::price,
                                         "qty",
                                         &T::qty,
                                         "quoteQty",
                                         &T::quoteQty,
                                         "commission",
                                         &T::commission,
                                         "commissionAsset",
                                         &T::commissionAsset,
                                         "time",
                                         &T::time,
                                         "buyer",
                                         &T::buyer,
                                         "maker",
                                         &T::maker,
                                         "realizedPnl",
                                         &T::realizedPnl);
};

template<>
struct glz::meta<binapi2::fapi::types::change_leverage_response>
{
    using T = binapi2::fapi::types::change_leverage_response;
    static constexpr auto value =
        object("leverage", &T::leverage, "maxNotionalValue", &T::maxNotionalValue, "symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::modify_isolated_margin_response>
{
    using T = binapi2::fapi::types::modify_isolated_margin_response;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg, "amount", &T::amount, "type", &T::type);
};

template<>
struct glz::meta<binapi2::fapi::types::position_margin_history_entry>
{
    using T = binapi2::fapi::types::position_margin_history_entry;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "type",
                                         &T::type,
                                         "deltaType",
                                         &T::deltaType,
                                         "amount",
                                         &T::amount,
                                         "asset",
                                         &T::asset,
                                         "time",
                                         &T::time,
                                         "positionSide",
                                         &T::positionSide);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_order_response>
{
    using T = binapi2::fapi::types::algo_order_response;
    static constexpr auto value = object("algoId",
                                         &T::algoId,
                                         "clientAlgoId",
                                         &T::clientAlgoId,
                                         "algoType",
                                         &T::algoType,
                                         "symbol",
                                         &T::symbol,
                                         "side",
                                         &T::side,
                                         "positionSide",
                                         &T::positionSide,
                                         "algoStatus",
                                         &T::algoStatus);
};
