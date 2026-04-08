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
struct account_asset
{
    std::string asset{};
    decimal walletBalance{};
    decimal unrealizedProfit{};
    decimal marginBalance{};
    decimal maintMargin{};
    decimal initialMargin{};
    decimal positionInitialMargin{};
    decimal openOrderInitialMargin{};
    decimal crossWalletBalance{};
    decimal crossUnPnl{};
    decimal availableBalance{};
    decimal maxWithdrawAmount{};
    std::optional<bool> marginAvailable{};
    timestamp_ms updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
struct account_position
{
    std::string symbol{};
    decimal initialMargin{};
    decimal maintMargin{};
    decimal unrealizedProfit{};
    decimal positionInitialMargin{};
    decimal openOrderInitialMargin{};
    std::optional<decimal> leverage{};
    std::optional<bool> isolated{};
    std::optional<decimal> entryPrice{};
    std::optional<decimal> breakEvenPrice{};
    std::optional<decimal> maxNotional{};
    std::optional<decimal> bidNotional{};
    std::optional<decimal> askNotional{};
    position_side positionSide{};
    decimal positionAmt{};
    std::optional<decimal> isolatedMargin{};
    std::optional<decimal> notional{};
    std::optional<decimal> isolatedWallet{};
    timestamp_ms updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
struct account_information
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    timestamp_ms updateTime{};
    std::optional<bool> multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
    decimal totalInitialMargin{};
    decimal totalMaintMargin{};
    decimal totalWalletBalance{};
    decimal totalUnrealizedProfit{};
    decimal totalMarginBalance{};
    decimal totalPositionInitialMargin{};
    decimal totalOpenOrderInitialMargin{};
    decimal totalCrossWalletBalance{};
    decimal totalCrossUnPnl{};
    decimal availableBalance{};
    decimal maxWithdrawAmount{};
    std::vector<account_asset> assets{};
    std::vector<account_position> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Account-Balance-V3.md
struct futures_account_balance
{
    std::string accountAlias{};
    std::string asset{};
    decimal balance{};
    decimal crossWalletBalance{};
    decimal crossUnPnl{};
    decimal availableBalance{};
    decimal maxWithdrawAmount{};
    bool marginAvailable{};
    timestamp_ms updateTime{};
};

// ---------------------------------------------------------------------------
// Position risk
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
struct position_risk_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
struct position_risk
{
    std::string symbol{};
    decimal positionAmt{};
    decimal entryPrice{};
    decimal breakEvenPrice{};
    decimal markPrice{};
    decimal unRealizedProfit{};
    decimal liquidationPrice{};
    decimal leverage{};
    decimal maxNotionalValue{};
    margin_type marginType{};
    decimal isolatedMargin{};
    bool isAutoAddMargin{};
    position_side positionSide{};
    decimal notional{};
    decimal isolatedWallet{};
    timestamp_ms updateTime{};
};

// ---------------------------------------------------------------------------
// Account and symbol configuration
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Config.md
struct account_config_response
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    bool dualSidePosition{};
    bool multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
    timestamp_ms updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
struct symbol_config_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
struct symbol_config_entry
{
    std::string symbol{};
    margin_type marginType{};
    bool isAutoAddMargin{};
    int leverage{};
    decimal maxNotionalValue{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Multi-Assets-Mode.md
struct multi_assets_mode_response
{
    bool multiAssetsMargin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Position-Mode.md
struct position_mode_response
{
    bool dualSidePosition{};
};

// ---------------------------------------------------------------------------
// Income history
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
struct income_history_request
{
    std::optional<std::string> symbol{};
    std::optional<std::string> incomeType{};
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
struct income_history_entry
{
    std::string symbol{};
    income_type incomeType{};
    decimal income{};
    std::string asset{};
    std::string info{};
    timestamp_ms time{};
    std::uint64_t tranId{};
    std::string tradeId{};
};

// ---------------------------------------------------------------------------
// Leverage brackets and commissions
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct leverage_bracket_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct leverage_bracket_entry
{
    int bracket{};
    int initialLeverage{};
    std::uint64_t notionalCap{};
    std::uint64_t notionalFloor{};
    decimal maintMarginRatio{};
    decimal cum{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
struct symbol_leverage_brackets
{
    std::string symbol{};
    std::optional<decimal> notionalCoef{};
    std::vector<leverage_bracket_entry> brackets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
struct commission_rate_request
{
    std::string symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
struct commission_rate_response
{
    std::string symbol{};
    decimal makerCommissionRate{};
    decimal takerCommissionRate{};
    std::optional<decimal> rpiCommissionRate{};
};

// ---------------------------------------------------------------------------
// Transaction history download
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
struct download_id_request
{
    timestamp_ms startTime{};
    timestamp_ms endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
struct download_id_response
{
    timestamp_ms avgCostTimestampOfLast30d{};
    std::string downloadId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
struct download_link_request
{
    std::string downloadId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
struct download_link_response
{
    std::string downloadId{};
    std::string status{};
    std::string url{};
    bool notified{};
    timestamp_ms expirationTimestamp{};
    bool isExpired{};
};

// ---------------------------------------------------------------------------
// BNB burn and trading rules
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-BNB-Burn-Status.md
struct bnb_burn_status_response
{
    bool feeBurn{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Toggle-BNB-Burn-On-Futures-Trade.md
struct toggle_bnb_burn_request
{
    std::string feeBurn{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct trading_status_indicator
{
    bool isLocked{};
    timestamp_ms plannedRecoverTime{};
    std::string indicator{};
    double value{};
    double triggerValue{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct quantitative_rules_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
struct quantitative_rules_response
{
    std::map<std::string, std::vector<trading_status_indicator>> indicators{};
    timestamp_ms updateTime{};
};

// ---------------------------------------------------------------------------
// Portfolio margin
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md
struct pm_account_info_request
{
    std::string asset{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md
struct pm_account_info_response
{
    decimal maxWithdrawAmountUSD{};
    std::string asset{};
    decimal maxWithdrawAmount{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::pm_account_info_response>
{
    using T = binapi2::fapi::types::pm_account_info_response;
    static constexpr auto value =
        object("maxWithdrawAmountUSD", &T::maxWithdrawAmountUSD, "asset", &T::asset, "maxWithdrawAmount", &T::maxWithdrawAmount);
};

template<>
struct glz::meta<binapi2::fapi::types::account_asset>
{
    using T = binapi2::fapi::types::account_asset;
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
struct glz::meta<binapi2::fapi::types::account_position>
{
    using T = binapi2::fapi::types::account_position;
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
struct glz::meta<binapi2::fapi::types::account_information>
{
    using T = binapi2::fapi::types::account_information;
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
struct glz::meta<binapi2::fapi::types::futures_account_balance>
{
    using T = binapi2::fapi::types::futures_account_balance;
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
struct glz::meta<binapi2::fapi::types::position_risk>
{
    using T = binapi2::fapi::types::position_risk;
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
struct glz::meta<binapi2::fapi::types::account_config_response>
{
    using T = binapi2::fapi::types::account_config_response;
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
struct glz::meta<binapi2::fapi::types::symbol_config_entry>
{
    using T = binapi2::fapi::types::symbol_config_entry;
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
struct glz::meta<binapi2::fapi::types::multi_assets_mode_response>
{
    using T = binapi2::fapi::types::multi_assets_mode_response;
    static constexpr auto value = object("multiAssetsMargin", &T::multiAssetsMargin);
};

template<>
struct glz::meta<binapi2::fapi::types::position_mode_response>
{
    using T = binapi2::fapi::types::position_mode_response;
    static constexpr auto value = object("dualSidePosition", &T::dualSidePosition);
};

template<>
struct glz::meta<binapi2::fapi::types::income_history_entry>
{
    using T = binapi2::fapi::types::income_history_entry;
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
struct glz::meta<binapi2::fapi::types::leverage_bracket_entry>
{
    using T = binapi2::fapi::types::leverage_bracket_entry;
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
struct glz::meta<binapi2::fapi::types::symbol_leverage_brackets>
{
    using T = binapi2::fapi::types::symbol_leverage_brackets;
    static constexpr auto value =
        object("symbol", &T::symbol, "notionalCoef", &T::notionalCoef, "brackets", &T::brackets);
};

template<>
struct glz::meta<binapi2::fapi::types::commission_rate_response>
{
    using T = binapi2::fapi::types::commission_rate_response;
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
struct glz::meta<binapi2::fapi::types::download_id_response>
{
    using T = binapi2::fapi::types::download_id_response;
    static constexpr auto value =
        object("avgCostTimestampOfLast30d", &T::avgCostTimestampOfLast30d, "downloadId", &T::downloadId);
};

template<>
struct glz::meta<binapi2::fapi::types::download_link_response>
{
    using T = binapi2::fapi::types::download_link_response;
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
struct glz::meta<binapi2::fapi::types::bnb_burn_status_response>
{
    using T = binapi2::fapi::types::bnb_burn_status_response;
    static constexpr auto value = object("feeBurn", &T::feeBurn);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_status_indicator>
{
    using T = binapi2::fapi::types::trading_status_indicator;
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
struct glz::meta<binapi2::fapi::types::quantitative_rules_response>
{
    using T = binapi2::fapi::types::quantitative_rules_response;
    static constexpr auto value = object("indicators", &T::indicators, "updateTime", &T::updateTime);
};
