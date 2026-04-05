// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file trade.hpp
/// @brief Request and response types for Binance USD-M Futures trade endpoints.
///
/// Covers order placement/query/cancel, batch orders, position management
/// (leverage, margin type, margin adjustment), trade history, and algo orders.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/decimal.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// ---------------------------------------------------------------------------
// Order placement, query, and cancel
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
struct new_order_request
{
    std::string symbol{};
    order_side side{ order_side::buy };
    order_type type{ order_type::limit };
    std::optional<time_in_force> timeInForce{};
    decimal quantity{};
    std::optional<decimal> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<decimal> stopPrice{};
    std::optional<position_side> positionSide{};
    std::optional<std::string> reduceOnly{};
    std::optional<std::string> closePosition{};
    std::optional<decimal> activationPrice{};
    std::optional<decimal> callbackRate{};
    std::optional<std::string> workingType{};
    std::optional<std::string> priceProtect{};
    std::optional<std::string> newOrderRespType{};
    std::optional<std::string> priceMatch{};
    std::optional<std::string> selfTradePreventionMode{};
    std::optional<std::uint64_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
struct order_response
{
    std::string clientOrderId{};
    decimal cumQty{};
    decimal cumQuote{};
    decimal executedQty{};
    std::optional<decimal> cumBase{};
    std::uint64_t orderId{};
    decimal avgPrice{};
    decimal origQty{};
    std::optional<std::string> pair{};
    decimal price{};
    bool reduceOnly{};
    order_side side{};
    position_side positionSide{};
    order_status status{};
    decimal stopPrice{};
    bool closePosition{};
    std::string symbol{};
    time_in_force timeInForce{};
    order_type type{};
    std::optional<working_type> workingType{};
    std::optional<bool> priceProtect{};
    order_type origType{};
    std::optional<price_match> priceMatch{};
    std::optional<stp_mode> selfTradePreventionMode{};
    std::optional<std::uint64_t> goodTillDate{};
    std::optional<decimal> activatePrice{};
    std::optional<decimal> priceRate{};
    std::optional<std::uint64_t> time{};       ///< Order creation time (ms). Present in query/list responses.
    std::optional<std::uint64_t> updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Order.md
struct modify_order_request
{
    std::string symbol{};
    order_side side{ order_side::buy };
    decimal quantity{};
    decimal price{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<std::string> priceMatch{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Order.md
struct cancel_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Order.md
struct query_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

/// Alias: test order uses the same request shape as a real order.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
using test_new_order_request = new_order_request;

// ---------------------------------------------------------------------------
// Batch and cancel-all orders
// ---------------------------------------------------------------------------

/// Batch order placement. The batchOrders field is a JSON-encoded array
/// of order parameter objects (pre-serialized by the caller).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Place-Multiple-Orders.md
struct batch_orders_request
{
    std::string batchOrders{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Multiple-Orders.md
struct cancel_multiple_orders_request
{
    std::string symbol{};
    std::optional<std::vector<std::uint64_t>> orderIdList{};
    std::optional<std::vector<std::string>> origClientOrderIdList{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Open-Orders.md
struct cancel_all_open_orders_request
{
    std::string symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
struct code_msg_response
{
    int code{};
    std::string msg{};
};

/// Auto-cancel (dead man's switch): cancel all open orders if not refreshed
/// within countdownTime milliseconds.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
struct auto_cancel_request
{
    std::string symbol{};
    std::uint64_t countdownTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
struct auto_cancel_response
{
    std::string symbol{};
    std::uint64_t countdownTime{};
};

// ---------------------------------------------------------------------------
// Order queries and trade history
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Current-Open-Order.md
struct query_open_order_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Open-Orders.md
struct all_open_orders_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/All-Orders.md
struct all_orders_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

// ---------------------------------------------------------------------------
// Position information and ADL quantile
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
struct position_risk_v3
{
    std::string symbol{};
    position_side positionSide{};
    decimal positionAmt{};
    decimal entryPrice{};
    decimal breakEvenPrice{};
    decimal markPrice{};
    decimal unRealizedProfit{};
    decimal liquidationPrice{};
    decimal isolatedMargin{};
    decimal notional{};
    std::string marginAsset{};
    std::string isolatedWallet{};
    decimal initialMargin{};
    decimal maintMargin{};
    decimal positionInitialMargin{};
    decimal openOrderInitialMargin{};
    int adl{};
    decimal bidNotional{};
    decimal askNotional{};
    std::uint64_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
struct position_info_v3_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_values
{
    int LONG{};
    int SHORT{};
    std::optional<int> BOTH{};
    std::optional<int> HEDGE{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_entry
{
    std::string symbol{};
    adl_quantile_values adlQuantile{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Users-Force-Orders.md
struct force_orders_request
{
    std::optional<std::string> symbol{};
    std::optional<auto_close_type> autoCloseType{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
struct account_trade_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<std::uint64_t> fromId{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
struct account_trade_entry
{
    std::uint64_t id{};
    std::uint64_t orderId{};
    std::string symbol{};
    order_side side{};
    position_side positionSide{};
    decimal price{};
    decimal qty{};
    decimal quoteQty{};
    decimal commission{};
    std::string commissionAsset{};
    std::uint64_t time{};
    bool buyer{};
    bool maker{};
    decimal realizedPnl{};
};

// ---------------------------------------------------------------------------
// Position management (leverage, margin type, margin adjustment, mode)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Position-Mode.md
struct change_position_mode_request
{
    std::string dualSidePosition{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Multi-Assets-Mode.md
struct change_multi_assets_mode_request
{
    std::string multiAssetsMargin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
struct change_leverage_request
{
    std::string symbol{};
    int leverage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
struct change_leverage_response
{
    int leverage{};
    decimal maxNotionalValue{};
    std::string symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Margin-Type.md
struct change_margin_type_request
{
    std::string symbol{};
    std::string marginType{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
struct modify_isolated_margin_request
{
    std::string symbol{};
    std::optional<position_side> positionSide{};
    decimal amount{};
    delta_type type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
struct modify_isolated_margin_response
{
    int code{};
    std::string msg{};
    decimal amount{};
    int type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
struct position_margin_history_request
{
    std::string symbol{};
    std::optional<int> type{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
struct position_margin_history_entry
{
    std::string symbol{};
    int type{};
    std::string deltaType{};
    decimal amount{};
    std::string asset{};
    std::uint64_t time{};
    std::string positionSide{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Order-Modify-History.md
struct order_modify_history_request
{
    std::string symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
struct new_algo_order_request
{
    std::string symbol{};
    order_side side{};
    std::optional<position_side> positionSide{};
    order_type type{};
    std::optional<time_in_force> timeInForce{};
    decimal quantity{};
    std::optional<decimal> price{};
    std::optional<decimal> triggerPrice{};
    std::optional<std::string> workingType{};
    algo_type algoType{};
    std::optional<std::string> clientAlgoId{};
    std::optional<std::string> priceMatch{};
    std::optional<std::string> closePosition{};
    std::optional<std::string> priceProtect{};
    std::optional<std::string> reduceOnly{};
    std::optional<decimal> activatePrice{};
    std::optional<decimal> callbackRate{};
    std::optional<std::string> newOrderRespType{};
    std::optional<std::string> selfTradePreventionMode{};
    std::optional<std::uint64_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
struct algo_order_response
{
    std::uint64_t algoId{};
    std::optional<std::string> clientAlgoId{};
    algo_type algoType{};
    std::string symbol{};
    order_side side{};
    std::optional<position_side> positionSide{};
    algo_status algoStatus{};
    std::optional<order_type> orderType{};
    std::optional<time_in_force> timeInForce{};
    std::optional<decimal> quantity{};
    std::optional<decimal> triggerPrice{};
    std::optional<decimal> price{};
    std::optional<decimal> icebergQuantity{};
    std::optional<stp_mode> selfTradePreventionMode{};
    std::optional<working_type> workingType{};
    std::optional<price_match> priceMatch{};
    std::optional<bool> closePosition{};
    std::optional<bool> priceProtect{};
    std::optional<bool> reduceOnly{};
    std::optional<decimal> activatePrice{};
    std::optional<decimal> callbackRate{};
    std::optional<std::uint64_t> createTime{};
    std::optional<std::uint64_t> updateTime{};
    std::optional<std::uint64_t> triggerTime{};
    std::optional<std::uint64_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Algo-Order.md
struct cancel_algo_order_request
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Algo-Order.md
struct query_algo_order_request
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-All-Algo-Orders.md
struct all_algo_orders_request
{
    std::string symbol{};
    std::optional<std::uint64_t> algoId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/TradFi-Perps.md
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
                                         "activatePrice",
                                         &T::activatePrice,
                                         "priceRate",
                                         &T::priceRate,
                                         "time",
                                         &T::time,
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
    static constexpr auto value = object("LONG", &T::LONG, "SHORT", &T::SHORT, "BOTH", &T::BOTH, "HEDGE", &T::HEDGE);
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
                                         &T::algoStatus,
                                         "orderType",
                                         &T::orderType,
                                         "timeInForce",
                                         &T::timeInForce,
                                         "quantity",
                                         &T::quantity,
                                         "triggerPrice",
                                         &T::triggerPrice,
                                         "price",
                                         &T::price,
                                         "icebergQuantity",
                                         &T::icebergQuantity,
                                         "selfTradePreventionMode",
                                         &T::selfTradePreventionMode,
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
                                         &T::activatePrice,
                                         "callbackRate",
                                         &T::callbackRate,
                                         "createTime",
                                         &T::createTime,
                                         "updateTime",
                                         &T::updateTime,
                                         "triggerTime",
                                         &T::triggerTime,
                                         "goodTillDate",
                                         &T::goodTillDate);
};
