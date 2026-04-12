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
#include <binapi2/fapi/types/detail/decimal.hpp>
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
struct new_order_request_t
{
    symbol_t symbol{};
    order_side_t side{ order_side_t::buy };
    order_type_t type{ order_type_t::limit };
    std::optional<time_in_force_t> timeInForce{};
    decimal_t quantity{};
    std::optional<decimal_t> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<decimal_t> stopPrice{};
    std::optional<position_side_t> positionSide{};
    std::optional<std::string> reduceOnly{};
    std::optional<std::string> closePosition{};
    std::optional<decimal_t> activationPrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<working_type_t> workingType{};
    std::optional<std::string> priceProtect{};
    std::optional<response_type_t> newOrderRespType{};
    std::optional<price_match_t> priceMatch{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
struct order_response_t
{
    std::string clientOrderId{};
    std::optional<decimal_t> cumQty{};
    decimal_t cumQuote{};
    decimal_t executedQty{};
    std::optional<decimal_t> cumBase{};
    std::uint64_t orderId{};
    decimal_t avgPrice{};
    decimal_t origQty{};
    std::optional<pair_t> pair{};
    decimal_t price{};
    bool reduceOnly{};
    order_side_t side{};
    position_side_t positionSide{};
    order_status_t status{};
    decimal_t stopPrice{};
    bool closePosition{};
    symbol_t symbol{};
    time_in_force_t timeInForce{};
    order_type_t type{};
    std::optional<working_type_t> workingType{};
    std::optional<bool> priceProtect{};
    order_type_t origType{};
    std::optional<price_match_t> priceMatch{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
    std::optional<decimal_t> activatePrice{};
    std::optional<decimal_t> priceRate{};
    std::optional<timestamp_ms_t> time{};       ///< Order creation time (ms). Present in query/list responses.
    std::optional<timestamp_ms_t> updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Order.md
struct modify_order_request_t
{
    symbol_t symbol{};
    order_side_t side{ order_side_t::buy };
    decimal_t quantity{};
    decimal_t price{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<price_match_t> priceMatch{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Order.md
struct cancel_order_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Order.md
struct query_order_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

/// Alias: test order uses the same request shape as a real order.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
using test_new_order_request = new_order_request_t;

// ---------------------------------------------------------------------------
// Batch and cancel-all orders
// ---------------------------------------------------------------------------

/// Batch order placement. The batchOrders field is a JSON-encoded array
/// of order parameter objects (pre-serialized by the caller).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Place-Multiple-Orders.md
struct batch_orders_request_t
{
    std::string batchOrders{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Multiple-Orders.md
struct cancel_multiple_orders_request_t
{
    symbol_t symbol{};
    std::optional<std::vector<std::uint64_t>> orderIdList{};
    std::optional<std::vector<std::string>> origClientOrderIdList{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Open-Orders.md
struct cancel_all_open_orders_request_t
{
    symbol_t symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
struct code_msg_response_t
{
    int code{};
    std::string msg{};
};

/// Auto-cancel (dead man's switch): cancel all open orders if not refreshed
/// within countdownTime milliseconds.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
struct auto_cancel_request_t
{
    symbol_t symbol{};
    std::uint64_t countdownTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
struct auto_cancel_response_t
{
    symbol_t symbol{};
    std::uint64_t countdownTime{};
};

// ---------------------------------------------------------------------------
// Order queries and trade history
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Current-Open-Order.md
struct query_open_order_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Open-Orders.md
struct all_open_orders_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/All-Orders.md
struct all_orders_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// ---------------------------------------------------------------------------
// Position information and ADL quantile
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
struct position_risk_v3_t
{
    symbol_t symbol{};
    position_side_t positionSide{};
    decimal_t positionAmt{};
    decimal_t entryPrice{};
    decimal_t breakEvenPrice{};
    decimal_t markPrice{};
    decimal_t unRealizedProfit{};
    decimal_t liquidationPrice{};
    decimal_t isolatedMargin{};
    decimal_t notional{};
    std::string marginAsset{};
    std::string isolatedWallet{};
    decimal_t initialMargin{};
    decimal_t maintMargin{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    int adl{};
    decimal_t bidNotional{};
    decimal_t askNotional{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
struct position_info_v3_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_values_t
{
    int LONG{};
    int SHORT{};
    std::optional<int> BOTH{};
    std::optional<int> HEDGE{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_entry_t
{
    symbol_t symbol{};
    adl_quantile_values_t adlQuantile{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
struct adl_quantile_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Users-Force-Orders.md
struct force_orders_request_t
{
    std::optional<symbol_t> symbol{};
    std::optional<auto_close_type_t> autoCloseType{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
struct account_trade_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<std::uint64_t> fromId{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
struct account_trade_entry_t
{
    std::uint64_t id{};
    std::uint64_t orderId{};
    symbol_t symbol{};
    order_side_t side{};
    position_side_t positionSide{};
    decimal_t price{};
    decimal_t qty{};
    decimal_t quoteQty{};
    decimal_t commission{};
    std::string commissionAsset{};
    timestamp_ms_t time{};
    bool buyer{};
    bool maker{};
    decimal_t realizedPnl{};
};

// ---------------------------------------------------------------------------
// Position management (leverage, margin type, margin adjustment, mode)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Position-Mode.md
struct change_position_mode_request_t
{
    std::string dualSidePosition{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Multi-Assets-Mode.md
struct change_multi_assets_mode_request_t
{
    std::string multiAssetsMargin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
struct change_leverage_request_t
{
    symbol_t symbol{};
    int leverage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
struct change_leverage_response_t
{
    int leverage{};
    decimal_t maxNotionalValue{};
    symbol_t symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Margin-Type.md
struct change_margin_type_request_t
{
    symbol_t symbol{};
    std::string marginType{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
struct modify_isolated_margin_request_t
{
    symbol_t symbol{};
    std::optional<position_side_t> positionSide{};
    decimal_t amount{};
    delta_type_t type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
struct modify_isolated_margin_response_t
{
    int code{};
    std::string msg{};
    decimal_t amount{};
    int type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
struct position_margin_history_request_t
{
    symbol_t symbol{};
    std::optional<int> type{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
struct position_margin_history_entry_t
{
    symbol_t symbol{};
    int type{};
    delta_type_t deltaType{};
    decimal_t amount{};
    std::string asset{};
    timestamp_ms_t time{};
    position_side_t positionSide{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Order-Modify-History.md
struct order_modify_history_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
struct new_algo_order_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    order_type_t type{};
    std::optional<time_in_force_t> timeInForce{};
    decimal_t quantity{};
    std::optional<decimal_t> price{};
    std::optional<decimal_t> triggerPrice{};
    std::optional<working_type_t> workingType{};
    algo_type_t algoType{};
    std::optional<std::string> clientAlgoId{};
    std::optional<price_match_t> priceMatch{};
    std::optional<std::string> closePosition{};
    std::optional<std::string> priceProtect{};
    std::optional<std::string> reduceOnly{};
    std::optional<decimal_t> activatePrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<response_type_t> newOrderRespType{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
struct algo_order_response_t
{
    std::uint64_t algoId{};
    std::optional<std::string> clientAlgoId{};
    algo_type_t algoType{};
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    algo_status_t algoStatus{};
    std::optional<order_type_t> orderType{};
    std::optional<time_in_force_t> timeInForce{};
    std::optional<decimal_t> quantity{};
    std::optional<decimal_t> triggerPrice{};
    std::optional<decimal_t> price{};
    std::optional<decimal_t> icebergQuantity{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<working_type_t> workingType{};
    std::optional<price_match_t> priceMatch{};
    std::optional<bool> closePosition{};
    std::optional<bool> priceProtect{};
    std::optional<bool> reduceOnly{};
    std::optional<decimal_t> activatePrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<timestamp_ms_t> createTime{};
    std::optional<timestamp_ms_t> updateTime{};
    std::optional<timestamp_ms_t> triggerTime{};
    std::optional<timestamp_ms_t> goodTillDate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Algo-Order.md
struct cancel_algo_order_request_t
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Algo-Order.md
struct query_algo_order_request_t
{
    std::optional<std::uint64_t> algoId{};
    std::optional<std::string> clientAlgoId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-All-Algo-Orders.md
struct all_algo_orders_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> algoId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/TradFi-Perps.md
struct tradfi_perps_request_t
{
};

// --- Parameterless trade request types ---

struct open_algo_orders_request_t { };
struct cancel_all_algo_orders_request_t { };

// --- Test order: distinct type sharing new_order fields ---

struct test_order_request_t {
    symbol_t symbol{};
    order_side_t side{ order_side_t::buy };
    order_type_t type{ order_type_t::limit };
    std::optional<time_in_force_t> timeInForce{};
    decimal_t quantity{};
    std::optional<decimal_t> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<decimal_t> stopPrice{};
    std::optional<decimal_t> activationPrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<working_type_t> workingType{};
    std::optional<bool> reduceOnly{};
    std::optional<position_side_t> positionSide{};
    std::optional<bool> closePosition{};
    std::optional<response_type_t> newOrderRespType{};
    std::optional<bool> priceProtect{};
    std::optional<price_match_t> priceMatch{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::order_response_t>
{
    using T = binapi2::fapi::types::order_response_t;
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
struct glz::meta<binapi2::fapi::types::code_msg_response_t>
{
    using T = binapi2::fapi::types::code_msg_response_t;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg);
};

template<>
struct glz::meta<binapi2::fapi::types::auto_cancel_response_t>
{
    using T = binapi2::fapi::types::auto_cancel_response_t;
    static constexpr auto value = object("symbol", &T::symbol, "countdownTime", &T::countdownTime);
};

template<>
struct glz::meta<binapi2::fapi::types::position_risk_v3_t>
{
    using T = binapi2::fapi::types::position_risk_v3_t;
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
struct glz::meta<binapi2::fapi::types::adl_quantile_values_t>
{
    using T = binapi2::fapi::types::adl_quantile_values_t;
    static constexpr auto value = object("LONG", &T::LONG, "SHORT", &T::SHORT, "BOTH", &T::BOTH, "HEDGE", &T::HEDGE);
};

template<>
struct glz::meta<binapi2::fapi::types::adl_quantile_entry_t>
{
    using T = binapi2::fapi::types::adl_quantile_entry_t;
    static constexpr auto value = object("symbol", &T::symbol, "adlQuantile", &T::adlQuantile);
};

template<>
struct glz::meta<binapi2::fapi::types::account_trade_entry_t>
{
    using T = binapi2::fapi::types::account_trade_entry_t;
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
struct glz::meta<binapi2::fapi::types::change_leverage_response_t>
{
    using T = binapi2::fapi::types::change_leverage_response_t;
    static constexpr auto value =
        object("leverage", &T::leverage, "maxNotionalValue", &T::maxNotionalValue, "symbol", &T::symbol);
};

template<>
struct glz::meta<binapi2::fapi::types::modify_isolated_margin_response_t>
{
    using T = binapi2::fapi::types::modify_isolated_margin_response_t;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg, "amount", &T::amount, "type", &T::type);
};

template<>
struct glz::meta<binapi2::fapi::types::position_margin_history_entry_t>
{
    using T = binapi2::fapi::types::position_margin_history_entry_t;
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
struct glz::meta<binapi2::fapi::types::algo_order_response_t>
{
    using T = binapi2::fapi::types::algo_order_response_t;
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
