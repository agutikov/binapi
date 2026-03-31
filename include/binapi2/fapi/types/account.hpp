// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <glaze/glaze.hpp>

#include <cstdint>
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
