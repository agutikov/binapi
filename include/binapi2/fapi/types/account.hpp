// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <glaze/glaze.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

struct account_asset
{
    std::string asset{};
    std::string walletBalance{};
    std::string unrealizedProfit{};
    std::string marginBalance{};
    std::string maintMargin{};
    std::string initialMargin{};
    std::string positionInitialMargin{};
    std::string openOrderInitialMargin{};
    std::string crossWalletBalance{};
    std::string crossUnPnl{};
    std::string availableBalance{};
    std::string maxWithdrawAmount{};
    std::optional<bool> marginAvailable{};
    std::uint64_t updateTime{};
};

struct account_position
{
    std::string symbol{};
    std::string initialMargin{};
    std::string maintMargin{};
    std::string unrealizedProfit{};
    std::string positionInitialMargin{};
    std::string openOrderInitialMargin{};
    std::optional<std::string> leverage{};
    std::optional<bool> isolated{};
    std::optional<std::string> entryPrice{};
    std::optional<std::string> breakEvenPrice{};
    std::optional<std::string> maxNotional{};
    std::optional<std::string> bidNotional{};
    std::optional<std::string> askNotional{};
    std::string positionSide{};
    std::string positionAmt{};
    std::optional<std::string> isolatedMargin{};
    std::optional<std::string> notional{};
    std::optional<std::string> isolatedWallet{};
    std::uint64_t updateTime{};
};

struct account_information
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    std::uint64_t updateTime{};
    std::optional<bool> multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
    std::string totalInitialMargin{};
    std::string totalMaintMargin{};
    std::string totalWalletBalance{};
    std::string totalUnrealizedProfit{};
    std::string totalMarginBalance{};
    std::string totalPositionInitialMargin{};
    std::string totalOpenOrderInitialMargin{};
    std::string totalCrossWalletBalance{};
    std::string totalCrossUnPnl{};
    std::string availableBalance{};
    std::string maxWithdrawAmount{};
    std::vector<account_asset> assets{};
    std::vector<account_position> positions{};
};

struct futures_account_balance
{
    std::string accountAlias{};
    std::string asset{};
    std::string balance{};
    std::string crossWalletBalance{};
    std::string crossUnPnl{};
    std::string availableBalance{};
    std::string maxWithdrawAmount{};
    bool marginAvailable{};
    std::uint64_t updateTime{};
};

struct position_risk_request
{
    std::optional<std::string> symbol{};
};

struct position_risk
{
    std::string symbol{};
    std::string positionAmt{};
    std::string entryPrice{};
    std::string breakEvenPrice{};
    std::string markPrice{};
    std::string unRealizedProfit{};
    std::string liquidationPrice{};
    std::string leverage{};
    std::string maxNotionalValue{};
    std::string marginType{};
    std::string isolatedMargin{};
    bool isAutoAddMargin{};
    std::string positionSide{};
    std::string notional{};
    std::string isolatedWallet{};
    std::uint64_t updateTime{};
};

struct account_config_response
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    bool dualSidePosition{};
    bool multiAssetsMargin{};
    std::optional<int> tradeGroupId{};
};

struct symbol_config_request
{
    std::optional<std::string> symbol{};
};

struct symbol_config_entry
{
    std::string symbol{};
    std::string marginType{};
    std::string isAutoAddMargin{};
    int leverage{};
    std::string maxNotionalValue{};
};

struct multi_assets_mode_response
{
    bool multiAssetsMargin{};
};

struct position_mode_response
{
    bool dualSidePosition{};
};

struct income_history_request
{
    std::optional<std::string> symbol{};
    std::optional<std::string> incomeType{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> page{};
    std::optional<int> limit{};
};

struct income_history_entry
{
    std::string symbol{};
    std::string incomeType{};
    std::string income{};
    std::string asset{};
    std::string info{};
    std::uint64_t time{};
    std::uint64_t tranId{};
    std::string tradeId{};
};

struct leverage_bracket_request
{
    std::optional<std::string> symbol{};
};

struct leverage_bracket_entry
{
    int bracket{};
    int initialLeverage{};
    std::uint64_t notionalCap{};
    std::uint64_t notionalFloor{};
    std::string maintMarginRatio{};
    std::string cum{};
};

struct symbol_leverage_brackets
{
    std::string symbol{};
    std::optional<std::string> notionalCoef{};
    std::vector<leverage_bracket_entry> brackets{};
};

struct commission_rate_request
{
    std::string symbol{};
};

struct commission_rate_response
{
    std::string symbol{};
    std::string makerCommissionRate{};
    std::string takerCommissionRate{};
    std::optional<std::string> rpiCommissionRate{};
};

struct download_id_request
{
    std::uint64_t startTime{};
    std::uint64_t endTime{};
};

struct download_id_response
{
    std::uint64_t avgCostTimestampOfLast30d{};
    std::string downloadId{};
};

struct download_link_request
{
    std::string downloadId{};
};

struct download_link_response
{
    std::string downloadId{};
    std::string status{};
    std::string url{};
    bool notified{};
    std::uint64_t expirationTimestamp{};
    bool isExpired{};
};

struct bnb_burn_status_response
{
    bool feeBurn{};
};

struct toggle_bnb_burn_request
{
    std::string feeBurn{};
};

struct trading_status_indicator
{
    bool isLocked{};
    std::uint64_t plannedRecoverTime{};
    std::string indicator{};
    double value{};
    double triggerValue{};
};

struct quantitative_rules_request
{
    std::optional<std::string> symbol{};
};

struct quantitative_rules_response
{
    std::map<std::string, std::vector<trading_status_indicator>> indicators{};
};

} // namespace binapi2::fapi::types

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
                                         &T::isAutoAddMargin,
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
                                         &T::tradeGroupId);
};

template<>
struct glz::meta<binapi2::fapi::types::symbol_config_entry>
{
    using T = binapi2::fapi::types::symbol_config_entry;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "marginType",
                                         &T::marginType,
                                         "isAutoAddMargin",
                                         &T::isAutoAddMargin,
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
    static constexpr auto value = object("indicators", &T::indicators);
};
