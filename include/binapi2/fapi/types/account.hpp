// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file account.hpp
/// @brief Request and response types for Binance USD-M Futures account endpoints.
///
/// Covers account information, balances, position risk, configuration,
/// income history, leverage brackets, commissions, and related queries.

#pragma once

#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/symbol.hpp>
#include <binapi2/fapi/types/enums.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// ---------------------------------------------------------------------------
// Account information and balances
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
struct account_asset_t
{
    std::string asset{};
    decimal_t walletBalance{};
    decimal_t unrealizedProfit{};
    decimal_t marginBalance{};
    decimal_t maintMargin{};
    decimal_t initialMargin{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    decimal_t crossWalletBalance{};
    decimal_t crossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    std::optional<bool> marginAvailable{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
struct account_position_t
{
    symbol_t symbol{};
    decimal_t initialMargin{};
    decimal_t maintMargin{};
    decimal_t unrealizedProfit{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    std::optional<decimal_t> leverage{};
    std::optional<bool> isolated{};
    std::optional<decimal_t> entryPrice{};
    std::optional<decimal_t> breakEvenPrice{};
    std::optional<decimal_t> maxNotional{};
    std::optional<decimal_t> bidNotional{};
    std::optional<decimal_t> askNotional{};
    position_side_t positionSide{};
    decimal_t positionAmt{};
    std::optional<decimal_t> isolatedMargin{};
    std::optional<decimal_t> notional{};
    std::optional<decimal_t> isolatedWallet{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
struct account_information_t
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    timestamp_ms_t updateTime{};
    std::optional<bool> multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
    decimal_t totalInitialMargin{};
    decimal_t totalMaintMargin{};
    decimal_t totalWalletBalance{};
    decimal_t totalUnrealizedProfit{};
    decimal_t totalMarginBalance{};
    decimal_t totalPositionInitialMargin{};
    decimal_t totalOpenOrderInitialMargin{};
    decimal_t totalCrossWalletBalance{};
    decimal_t totalCrossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    std::vector<account_asset_t> assets{};
    std::vector<account_position_t> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Account-Balance-V3.md
struct futures_account_balance_t
{
    std::string accountAlias{};
    std::string asset{};
    decimal_t balance{};
    decimal_t crossWalletBalance{};
    decimal_t crossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    bool marginAvailable{};
    timestamp_ms_t updateTime{};
};

// ---------------------------------------------------------------------------
// Position risk
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
struct position_risk_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
struct position_risk_t
{
    symbol_t symbol{};
    decimal_t positionAmt{};
    decimal_t entryPrice{};
    decimal_t breakEvenPrice{};
    decimal_t markPrice{};
    decimal_t unRealizedProfit{};
    decimal_t liquidationPrice{};
    decimal_t leverage{};
    decimal_t maxNotionalValue{};
    margin_type_t marginType{};
    decimal_t isolatedMargin{};
    bool isAutoAddMargin{};
    position_side_t positionSide{};
    decimal_t notional{};
    decimal_t isolatedWallet{};
    timestamp_ms_t updateTime{};
};

// ---------------------------------------------------------------------------
// Account and symbol configuration
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Config.md
struct account_config_response_t
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    bool dualSidePosition{};
    bool multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
struct symbol_config_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
struct symbol_config_entry_t
{
    symbol_t symbol{};
    margin_type_t marginType{};
    bool isAutoAddMargin{};
    int leverage{};
    decimal_t maxNotionalValue{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Multi-Assets-Mode.md
struct multi_assets_mode_response_t
{
    bool multiAssetsMargin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Position-Mode.md
struct position_mode_response_t
{
    bool dualSidePosition{};
};

// ---------------------------------------------------------------------------
// Income history
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
struct income_history_request_t
{
    std::optional<symbol_t> symbol{};
    std::optional<std::string> incomeType{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
struct income_history_entry_t
{
    symbol_t symbol{};
    income_type_t incomeType{};
    decimal_t income{};
    std::string asset{};
    std::string info{};
    timestamp_ms_t time{};
    std::uint64_t tranId{};
    std::string tradeId{};
};

// ---------------------------------------------------------------------------
// Leverage brackets and commissions
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct leverage_bracket_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct leverage_bracket_entry_t
{
    int bracket{};
    int initialLeverage{};
    std::uint64_t notionalCap{};
    std::uint64_t notionalFloor{};
    decimal_t maintMarginRatio{};
    decimal_t cum{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct symbol_leverage_brackets_t
{
    symbol_t symbol{};
    std::optional<decimal_t> notionalCoef{};
    std::vector<leverage_bracket_entry_t> brackets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
struct commission_rate_request_t
{
    symbol_t symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
struct commission_rate_response_t
{
    symbol_t symbol{};
    decimal_t makerCommissionRate{};
    decimal_t takerCommissionRate{};
    std::optional<decimal_t> rpiCommissionRate{};
};

// ---------------------------------------------------------------------------
// Transaction history download
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
struct download_id_request_t
{
    timestamp_ms_t startTime{};
    timestamp_ms_t endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
struct download_id_response_t
{
    timestamp_ms_t avgCostTimestampOfLast30d{};
    std::string downloadId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
struct download_link_request_t
{
    std::string downloadId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
struct download_link_response_t
{
    std::string downloadId{};
    std::string status{};
    std::string url{};
    bool notified{};
    timestamp_ms_t expirationTimestamp{};
    bool isExpired{};
};

// ---------------------------------------------------------------------------
// BNB burn and trading rules
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-BNB-Burn-Status.md
struct bnb_burn_status_response_t
{
    bool feeBurn{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Toggle-BNB-Burn-On-Futures-Trade.md
struct toggle_bnb_burn_request_t
{
    std::string feeBurn{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct trading_status_indicator_t
{
    bool isLocked{};
    timestamp_ms_t plannedRecoverTime{};
    std::string indicator{};
    double value{};
    double triggerValue{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct quantitative_rules_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct quantitative_rules_response_t
{
    std::map<std::string, std::vector<trading_status_indicator_t>> indicators{};
    timestamp_ms_t updateTime{};
};

// ---------------------------------------------------------------------------
// Portfolio margin
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md
struct pm_account_info_request_t
{
    std::string asset{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md
struct pm_account_info_response_t
{
    decimal_t maxWithdrawAmountUSD{};
    std::string asset{};
    decimal_t maxWithdrawAmount{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::pm_account_info_response_t>
{
    using T = binapi2::fapi::types::pm_account_info_response_t;
    static constexpr auto value =
        object("maxWithdrawAmountUSD", &T::maxWithdrawAmountUSD, "asset", &T::asset, "maxWithdrawAmount", &T::maxWithdrawAmount);
};

template<>
struct glz::meta<binapi2::fapi::types::account_asset_t>
{
    using T = binapi2::fapi::types::account_asset_t;
    static constexpr auto value = object("asset",
                                         &T::asset,
                                         "walletBalance",
                                         &T::walletBalance,
                                         "unrealizedProfit",
                                         &T::unrealizedProfit,
                                         "marginBalance",
                                         &T::marginBalance,
                                         "maintMargin",
                                         &T::maintMargin,
                                         "initialMargin",
                                         &T::initialMargin,
                                         "positionInitialMargin",
                                         &T::positionInitialMargin,
                                         "openOrderInitialMargin",
                                         &T::openOrderInitialMargin,
                                         "crossWalletBalance",
                                         &T::crossWalletBalance,
                                         "crossUnPnl",
                                         &T::crossUnPnl,
                                         "availableBalance",
                                         &T::availableBalance,
                                         "maxWithdrawAmount",
                                         &T::maxWithdrawAmount,
                                         "marginAvailable",
                                         &T::marginAvailable,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::account_position_t>
{
    using T = binapi2::fapi::types::account_position_t;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "initialMargin",
                                         &T::initialMargin,
                                         "maintMargin",
                                         &T::maintMargin,
                                         "unrealizedProfit",
                                         &T::unrealizedProfit,
                                         "positionInitialMargin",
                                         &T::positionInitialMargin,
                                         "openOrderInitialMargin",
                                         &T::openOrderInitialMargin,
                                         "leverage",
                                         &T::leverage,
                                         "isolated",
                                         &T::isolated,
                                         "entryPrice",
                                         &T::entryPrice,
                                         "breakEvenPrice",
                                         &T::breakEvenPrice,
                                         "maxNotional",
                                         &T::maxNotional,
                                         "bidNotional",
                                         &T::bidNotional,
                                         "askNotional",
                                         &T::askNotional,
                                         "positionSide",
                                         &T::positionSide,
                                         "positionAmt",
                                         &T::positionAmt,
                                         "isolatedMargin",
                                         &T::isolatedMargin,
                                         "notional",
                                         &T::notional,
                                         "isolatedWallet",
                                         &T::isolatedWallet,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::account_information_t>
{
    using T = binapi2::fapi::types::account_information_t;
    static constexpr auto value = object("feeTier",
                                         &T::feeTier,
                                         "canTrade",
                                         &T::canTrade,
                                         "canDeposit",
                                         &T::canDeposit,
                                         "canWithdraw",
                                         &T::canWithdraw,
                                         "updateTime",
                                         &T::updateTime,
                                         "multiAssetsMargin",
                                         &T::multiAssetsMargin,
                                         "tradeGroupId",
                                         &T::tradeGroupId,
                                         "totalInitialMargin",
                                         &T::totalInitialMargin,
                                         "totalMaintMargin",
                                         &T::totalMaintMargin,
                                         "totalWalletBalance",
                                         &T::totalWalletBalance,
                                         "totalUnrealizedProfit",
                                         &T::totalUnrealizedProfit,
                                         "totalMarginBalance",
                                         &T::totalMarginBalance,
                                         "totalPositionInitialMargin",
                                         &T::totalPositionInitialMargin,
                                         "totalOpenOrderInitialMargin",
                                         &T::totalOpenOrderInitialMargin,
                                         "totalCrossWalletBalance",
                                         &T::totalCrossWalletBalance,
                                         "totalCrossUnPnl",
                                         &T::totalCrossUnPnl,
                                         "availableBalance",
                                         &T::availableBalance,
                                         "maxWithdrawAmount",
                                         &T::maxWithdrawAmount,
                                         "assets",
                                         &T::assets,
                                         "positions",
                                         &T::positions);
};

template<>
struct glz::meta<binapi2::fapi::types::futures_account_balance_t>
{
    using T = binapi2::fapi::types::futures_account_balance_t;
    static constexpr auto value = object("accountAlias",
                                         &T::accountAlias,
                                         "asset",
                                         &T::asset,
                                         "balance",
                                         &T::balance,
                                         "crossWalletBalance",
                                         &T::crossWalletBalance,
                                         "crossUnPnl",
                                         &T::crossUnPnl,
                                         "availableBalance",
                                         &T::availableBalance,
                                         "maxWithdrawAmount",
                                         &T::maxWithdrawAmount,
                                         "marginAvailable",
                                         &T::marginAvailable,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::position_risk_t>
{
    using T = binapi2::fapi::types::position_risk_t;
    // Binance sends isAutoAddMargin as string "true"/"false".
    static constexpr auto read_auto_add = [](T& s, const std::string& v) { s.isAutoAddMargin = (v == "true"); };
    static constexpr auto write_auto_add = [](const T& s) -> std::string { return s.isAutoAddMargin ? "true" : "false"; };
    static constexpr auto value = object("symbol",
                                         &T::symbol,
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
                                         "leverage",
                                         &T::leverage,
                                         "maxNotionalValue",
                                         &T::maxNotionalValue,
                                         "marginType",
                                         &T::marginType,
                                         "isolatedMargin",
                                         &T::isolatedMargin,
                                         "isAutoAddMargin",
                                         glz::custom<read_auto_add, write_auto_add>,
                                         "positionSide",
                                         &T::positionSide,
                                         "notional",
                                         &T::notional,
                                         "isolatedWallet",
                                         &T::isolatedWallet,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_response_t>
{
    using T = binapi2::fapi::types::account_config_response_t;
    static constexpr auto value = object("feeTier",
                                         &T::feeTier,
                                         "canTrade",
                                         &T::canTrade,
                                         "canDeposit",
                                         &T::canDeposit,
                                         "canWithdraw",
                                         &T::canWithdraw,
                                         "dualSidePosition",
                                         &T::dualSidePosition,
                                         "multiAssetsMargin",
                                         &T::multiAssetsMargin,
                                         "tradeGroupId",
                                         &T::tradeGroupId,
                                         "updateTime",
                                         &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::symbol_config_entry_t>
{
    using T = binapi2::fapi::types::symbol_config_entry_t;
    // Binance sends isAutoAddMargin as string "true"/"false".
    static constexpr auto read_auto_add = [](T& s, const std::string& v) { s.isAutoAddMargin = (v == "true"); };
    static constexpr auto write_auto_add = [](const T& s) -> std::string { return s.isAutoAddMargin ? "true" : "false"; };
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "marginType",
                                         &T::marginType,
                                         "isAutoAddMargin",
                                         glz::custom<read_auto_add, write_auto_add>,
                                         "leverage",
                                         &T::leverage,
                                         "maxNotionalValue",
                                         &T::maxNotionalValue);
};

template<>
struct glz::meta<binapi2::fapi::types::multi_assets_mode_response_t>
{
    using T = binapi2::fapi::types::multi_assets_mode_response_t;
    static constexpr auto value = object("multiAssetsMargin", &T::multiAssetsMargin);
};

template<>
struct glz::meta<binapi2::fapi::types::position_mode_response_t>
{
    using T = binapi2::fapi::types::position_mode_response_t;
    static constexpr auto value = object("dualSidePosition", &T::dualSidePosition);
};

template<>
struct glz::meta<binapi2::fapi::types::income_history_entry_t>
{
    using T = binapi2::fapi::types::income_history_entry_t;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "incomeType",
                                         &T::incomeType,
                                         "income",
                                         &T::income,
                                         "asset",
                                         &T::asset,
                                         "info",
                                         &T::info,
                                         "time",
                                         &T::time,
                                         "tranId",
                                         &T::tranId,
                                         "tradeId",
                                         &T::tradeId);
};

template<>
struct glz::meta<binapi2::fapi::types::leverage_bracket_entry_t>
{
    using T = binapi2::fapi::types::leverage_bracket_entry_t;
    static constexpr auto value = object("bracket",
                                         &T::bracket,
                                         "initialLeverage",
                                         &T::initialLeverage,
                                         "notionalCap",
                                         &T::notionalCap,
                                         "notionalFloor",
                                         &T::notionalFloor,
                                         "maintMarginRatio",
                                         &T::maintMarginRatio,
                                         "cum",
                                         &T::cum);
};

template<>
struct glz::meta<binapi2::fapi::types::symbol_leverage_brackets_t>
{
    using T = binapi2::fapi::types::symbol_leverage_brackets_t;
    static constexpr auto value =
        object("symbol", &T::symbol, "notionalCoef", &T::notionalCoef, "brackets", &T::brackets);
};

template<>
struct glz::meta<binapi2::fapi::types::commission_rate_response_t>
{
    using T = binapi2::fapi::types::commission_rate_response_t;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "makerCommissionRate",
                                         &T::makerCommissionRate,
                                         "takerCommissionRate",
                                         &T::takerCommissionRate,
                                         "rpiCommissionRate",
                                         &T::rpiCommissionRate);
};

template<>
struct glz::meta<binapi2::fapi::types::download_id_response_t>
{
    using T = binapi2::fapi::types::download_id_response_t;
    static constexpr auto value =
        object("avgCostTimestampOfLast30d", &T::avgCostTimestampOfLast30d, "downloadId", &T::downloadId);
};

template<>
struct glz::meta<binapi2::fapi::types::download_link_response_t>
{
    using T = binapi2::fapi::types::download_link_response_t;
    static constexpr auto value = object("downloadId",
                                         &T::downloadId,
                                         "status",
                                         &T::status,
                                         "url",
                                         &T::url,
                                         "notified",
                                         &T::notified,
                                         "expirationTimestamp",
                                         &T::expirationTimestamp,
                                         "isExpired",
                                         &T::isExpired);
};

template<>
struct glz::meta<binapi2::fapi::types::bnb_burn_status_response_t>
{
    using T = binapi2::fapi::types::bnb_burn_status_response_t;
    static constexpr auto value = object("feeBurn", &T::feeBurn);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_status_indicator_t>
{
    using T = binapi2::fapi::types::trading_status_indicator_t;
    static constexpr auto value = object("isLocked",
                                         &T::isLocked,
                                         "plannedRecoverTime",
                                         &T::plannedRecoverTime,
                                         "indicator",
                                         &T::indicator,
                                         "value",
                                         &T::value,
                                         "triggerValue",
                                         &T::triggerValue);
};

template<>
struct glz::meta<binapi2::fapi::types::quantitative_rules_response_t>
{
    using T = binapi2::fapi::types::quantitative_rules_response_t;
    static constexpr auto value = object("indicators", &T::indicators, "updateTime", &T::updateTime);
};
