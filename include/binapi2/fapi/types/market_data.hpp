// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

struct ping_request
{};

struct server_time_request
{};

struct exchange_info_request
{
    std::optional<std::string> symbol{};
};

struct order_book_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

struct order_book_response
{
    std::uint64_t lastUpdateId{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::vector<price_level> bids{};
    std::vector<price_level> asks{};
};

struct recent_trades_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

struct recent_trade
{
    std::uint64_t id{};
    std::string price{};
    std::string qty{};
    std::string quoteQty{};
    std::uint64_t time{};
    bool isBuyerMaker{};
    std::optional<bool> isRPITrade{};
};

struct aggregate_trades_request
{
    std::string symbol{};
    std::optional<std::uint64_t> fromId{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct aggregate_trade
{
    std::uint64_t a{};
    std::string p{};
    std::string q{};
    std::uint64_t f{};
    std::uint64_t l{};
    std::uint64_t T{};
    bool m{};
};

struct historical_trades_request
{
    std::string symbol{};
    std::optional<int> limit{};
    std::optional<std::uint64_t> fromId{};
};

struct kline_request
{
    std::string symbol{};
    kline_interval interval{ kline_interval::m1 };
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct continuous_kline_request
{
    std::string pair{};
    contract_type contractType{ contract_type::perpetual };
    kline_interval interval{ kline_interval::m1 };
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct index_price_kline_request
{
    std::string pair{};
    kline_interval interval{ kline_interval::m1 };
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct kline
{
    std::uint64_t openTime{};
    std::string open{};
    std::string high{};
    std::string low{};
    std::string close{};
    std::string volume{};
    std::uint64_t closeTime{};
    std::string quoteAssetVolume{};
    std::uint64_t numberOfTrades{};
    std::string takerBuyBaseAssetVolume{};
    std::string takerBuyQuoteAssetVolume{};
    std::string ignore{};
};

struct book_ticker_request
{
    std::optional<std::string> symbol{};
};

struct book_ticker
{
    std::string symbol{};
    std::string bidPrice{};
    std::string bidQty{};
    std::string askPrice{};
    std::string askQty{};
    std::uint64_t time{};
    std::uint64_t lastUpdateId{};
};

struct price_ticker_request
{
    std::optional<std::string> symbol{};
};

struct price_ticker
{
    std::string symbol{};
    std::string price{};
    std::uint64_t time{};
};

struct ticker_24hr_request
{
    std::optional<std::string> symbol{};
};

struct ticker_24hr
{
    std::string symbol{};
    std::string priceChange{};
    std::string priceChangePercent{};
    std::string weightedAvgPrice{};
    std::string lastPrice{};
    std::string lastQty{};
    std::string openPrice{};
    std::string highPrice{};
    std::string lowPrice{};
    std::string volume{};
    std::string quoteVolume{};
    std::uint64_t openTime{};
    std::uint64_t closeTime{};
    std::uint64_t firstId{};
    std::uint64_t lastId{};
    std::uint64_t count{};
};

struct mark_price_request
{
    std::optional<std::string> symbol{};
};

struct mark_price
{
    std::string symbol{};
    std::string markPrice{};
    std::string indexPrice{};
    std::string estimatedSettlePrice{};
    std::string lastFundingRate{};
    std::uint64_t nextFundingTime{};
    std::uint64_t time{};
};

struct funding_rate_history_request
{
    std::optional<std::string> symbol{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
    std::optional<int> limit{};
};

struct funding_rate_history_entry
{
    std::string symbol{};
    std::string fundingRate{};
    std::uint64_t fundingTime{};
    std::string markPrice{};
};

struct funding_rate_info
{
    std::string symbol{};
    std::string adjustedFundingRateCap{};
    std::string adjustedFundingRateFloor{};
    int fundingIntervalHours{};
    bool disclaimer{};
};

struct open_interest_request
{
    std::string symbol{};
};

struct open_interest
{
    std::string openInterest{};
    std::string symbol{};
    std::uint64_t time{};
};

struct futures_data_request
{
    std::string symbol{};
    futures_data_period period{ futures_data_period::m5 };
    std::optional<int> limit{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
};

struct open_interest_statistics_entry
{
    std::string symbol{};
    std::string sumOpenInterest{};
    std::string sumOpenInterestValue{};
    std::string CMCCirculatingSupply{};
    std::uint64_t timestamp{};
};

struct long_short_ratio_entry
{
    std::string symbol{};
    std::string longShortRatio{};
    std::string longAccount{};
    std::string shortAccount{};
    std::uint64_t timestamp{};
};

struct taker_buy_sell_volume_entry
{
    std::string buySellRatio{};
    std::string buyVol{};
    std::string sellVol{};
    std::uint64_t timestamp{};
};

struct basis_request
{
    std::string pair{};
    contract_type contractType{ contract_type::perpetual };
    futures_data_period period{ futures_data_period::m5 };
    std::optional<int> limit{};
    std::optional<std::uint64_t> startTime{};
    std::optional<std::uint64_t> endTime{};
};

struct basis_entry
{
    std::string pair{};
    std::string contractType{};
    std::string basis{};
    std::string basisRate{};
    std::string futuresPrice{};
    std::string indexPrice{};
    std::string annualizedBasisRate{};
    std::uint64_t timestamp{};
};

struct price_ticker_v2_request
{
    std::optional<std::string> symbol{};
};

struct delivery_price_request
{
    std::string pair{};
};

struct delivery_price_entry
{
    std::uint64_t deliveryTime{};
    std::string deliveryPrice{};
};

struct composite_index_info_request
{
    std::optional<std::string> symbol{};
};

struct composite_index_base_asset
{
    std::string baseAsset{};
    std::string quoteAsset{};
    std::string weightInQuantity{};
    std::string weightInPercentage{};
};

struct composite_index_info
{
    std::string symbol{};
    std::uint64_t time{};
    std::string component{};
    std::vector<composite_index_base_asset> baseAssetList{};
};

struct index_constituents_request
{
    std::string symbol{};
};

struct index_constituent
{
    std::string exchange{};
    std::string symbol{};
    std::string price{};
    std::string weight{};
};

struct index_constituents_response
{
    std::string symbol{};
    std::uint64_t time{};
    std::vector<index_constituent> constituents{};
};

struct asset_index_request
{
    std::optional<std::string> symbol{};
};

struct asset_index
{
    std::string symbol{};
    std::uint64_t time{};
    std::string index{};
    std::string bidBuffer{};
    std::string askBuffer{};
    std::string bidRate{};
    std::string askRate{};
    std::string autoExchangeBidBuffer{};
    std::string autoExchangeAskBuffer{};
    std::string autoExchangeBidRate{};
    std::string autoExchangeAskRate{};
};

struct insurance_fund_request
{
    std::optional<std::string> symbol{};
};

struct insurance_fund_asset
{
    std::string asset{};
    std::string marginBalance{};
    std::uint64_t updateTime{};
};

struct insurance_fund_response
{
    std::vector<std::string> symbols{};
    std::vector<insurance_fund_asset> assets{};
};

struct adl_risk_request
{
    std::optional<std::string> symbol{};
};

struct adl_risk_entry
{
    std::string symbol{};
    std::string adlRisk{};
    std::uint64_t updateTime{};
};

struct rpi_depth_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

struct trading_schedule_request
{};

struct trading_schedule_response
{
    std::uint64_t updateTime{};
    std::map<std::string, std::vector<std::string>> marketSchedules{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::order_book_response>
{
    using T = binapi2::fapi::types::order_book_response;
    static constexpr auto value =
        object("lastUpdateId", &T::lastUpdateId, "E", &T::E, "T", &T::T, "bids", &T::bids, "asks", &T::asks);
};

template<>
struct glz::meta<binapi2::fapi::types::recent_trade>
{
    using T = binapi2::fapi::types::recent_trade;
    static constexpr auto value = object("id",
                                         &T::id,
                                         "price",
                                         &T::price,
                                         "qty",
                                         &T::qty,
                                         "quoteQty",
                                         &T::quoteQty,
                                         "time",
                                         &T::time,
                                         "isBuyerMaker",
                                         &T::isBuyerMaker,
                                         "isRPITrade",
                                         &T::isRPITrade);
};

template<>
struct glz::meta<binapi2::fapi::types::aggregate_trade>
{
    using T = binapi2::fapi::types::aggregate_trade;
    static constexpr auto value = object("a", &T::a, "p", &T::p, "q", &T::q, "f", &T::f, "l", &T::l, "T", &T::T, "m", &T::m);
};

template<>
struct glz::meta<binapi2::fapi::types::kline>
{
    using T = binapi2::fapi::types::kline;
    static constexpr auto value = array(&T::openTime,
                                        &T::open,
                                        &T::high,
                                        &T::low,
                                        &T::close,
                                        &T::volume,
                                        &T::closeTime,
                                        &T::quoteAssetVolume,
                                        &T::numberOfTrades,
                                        &T::takerBuyBaseAssetVolume,
                                        &T::takerBuyQuoteAssetVolume,
                                        &T::ignore);
};

template<>
struct glz::meta<binapi2::fapi::types::book_ticker>
{
    using T = binapi2::fapi::types::book_ticker;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "bidPrice",
                                         &T::bidPrice,
                                         "bidQty",
                                         &T::bidQty,
                                         "askPrice",
                                         &T::askPrice,
                                         "askQty",
                                         &T::askQty,
                                         "time",
                                         &T::time,
                                         "lastUpdateId",
                                         &T::lastUpdateId);
};

template<>
struct glz::meta<binapi2::fapi::types::price_ticker>
{
    using T = binapi2::fapi::types::price_ticker;
    static constexpr auto value = object("symbol", &T::symbol, "price", &T::price, "time", &T::time);
};

template<>
struct glz::meta<binapi2::fapi::types::ticker_24hr>
{
    using T = binapi2::fapi::types::ticker_24hr;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "priceChange",
                                         &T::priceChange,
                                         "priceChangePercent",
                                         &T::priceChangePercent,
                                         "weightedAvgPrice",
                                         &T::weightedAvgPrice,
                                         "lastPrice",
                                         &T::lastPrice,
                                         "lastQty",
                                         &T::lastQty,
                                         "openPrice",
                                         &T::openPrice,
                                         "highPrice",
                                         &T::highPrice,
                                         "lowPrice",
                                         &T::lowPrice,
                                         "volume",
                                         &T::volume,
                                         "quoteVolume",
                                         &T::quoteVolume,
                                         "openTime",
                                         &T::openTime,
                                         "closeTime",
                                         &T::closeTime,
                                         "firstId",
                                         &T::firstId,
                                         "lastId",
                                         &T::lastId,
                                         "count",
                                         &T::count);
};

template<>
struct glz::meta<binapi2::fapi::types::mark_price>
{
    using T = binapi2::fapi::types::mark_price;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "markPrice",
                                         &T::markPrice,
                                         "indexPrice",
                                         &T::indexPrice,
                                         "estimatedSettlePrice",
                                         &T::estimatedSettlePrice,
                                         "lastFundingRate",
                                         &T::lastFundingRate,
                                         "nextFundingTime",
                                         &T::nextFundingTime,
                                         "time",
                                         &T::time);
};

template<>
struct glz::meta<binapi2::fapi::types::funding_rate_history_entry>
{
    using T = binapi2::fapi::types::funding_rate_history_entry;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "fundingRate",
                                         &T::fundingRate,
                                         "fundingTime",
                                         &T::fundingTime,
                                         "markPrice",
                                         &T::markPrice);
};

template<>
struct glz::meta<binapi2::fapi::types::funding_rate_info>
{
    using T = binapi2::fapi::types::funding_rate_info;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "adjustedFundingRateCap",
                                         &T::adjustedFundingRateCap,
                                         "adjustedFundingRateFloor",
                                         &T::adjustedFundingRateFloor,
                                         "fundingIntervalHours",
                                         &T::fundingIntervalHours,
                                         "disclaimer",
                                         &T::disclaimer);
};

template<>
struct glz::meta<binapi2::fapi::types::open_interest>
{
    using T = binapi2::fapi::types::open_interest;
    static constexpr auto value = object("openInterest", &T::openInterest, "symbol", &T::symbol, "time", &T::time);
};

template<>
struct glz::meta<binapi2::fapi::types::open_interest_statistics_entry>
{
    using T = binapi2::fapi::types::open_interest_statistics_entry;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "sumOpenInterest",
                                         &T::sumOpenInterest,
                                         "sumOpenInterestValue",
                                         &T::sumOpenInterestValue,
                                         "CMCCirculatingSupply",
                                         &T::CMCCirculatingSupply,
                                         "timestamp",
                                         &T::timestamp);
};

template<>
struct glz::meta<binapi2::fapi::types::long_short_ratio_entry>
{
    using T = binapi2::fapi::types::long_short_ratio_entry;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "longShortRatio",
                                         &T::longShortRatio,
                                         "longAccount",
                                         &T::longAccount,
                                         "shortAccount",
                                         &T::shortAccount,
                                         "timestamp",
                                         &T::timestamp);
};

template<>
struct glz::meta<binapi2::fapi::types::taker_buy_sell_volume_entry>
{
    using T = binapi2::fapi::types::taker_buy_sell_volume_entry;
    static constexpr auto value =
        object("buySellRatio", &T::buySellRatio, "buyVol", &T::buyVol, "sellVol", &T::sellVol, "timestamp", &T::timestamp);
};

template<>
struct glz::meta<binapi2::fapi::types::basis_entry>
{
    using T = binapi2::fapi::types::basis_entry;
    static constexpr auto value = object("pair",
                                         &T::pair,
                                         "contractType",
                                         &T::contractType,
                                         "basis",
                                         &T::basis,
                                         "basisRate",
                                         &T::basisRate,
                                         "futuresPrice",
                                         &T::futuresPrice,
                                         "indexPrice",
                                         &T::indexPrice,
                                         "annualizedBasisRate",
                                         &T::annualizedBasisRate,
                                         "timestamp",
                                         &T::timestamp);
};

template<>
struct glz::meta<binapi2::fapi::types::delivery_price_entry>
{
    using T = binapi2::fapi::types::delivery_price_entry;
    static constexpr auto value = object("deliveryTime", &T::deliveryTime, "deliveryPrice", &T::deliveryPrice);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_base_asset>
{
    using T = binapi2::fapi::types::composite_index_base_asset;
    static constexpr auto value = object("baseAsset",
                                         &T::baseAsset,
                                         "quoteAsset",
                                         &T::quoteAsset,
                                         "weightInQuantity",
                                         &T::weightInQuantity,
                                         "weightInPercentage",
                                         &T::weightInPercentage);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_info>
{
    using T = binapi2::fapi::types::composite_index_info;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "time",
                                         &T::time,
                                         "component",
                                         &T::component,
                                         "baseAssetList",
                                         &T::baseAssetList);
};

template<>
struct glz::meta<binapi2::fapi::types::index_constituent>
{
    using T = binapi2::fapi::types::index_constituent;
    static constexpr auto value =
        object("exchange", &T::exchange, "symbol", &T::symbol, "price", &T::price, "weight", &T::weight);
};

template<>
struct glz::meta<binapi2::fapi::types::index_constituents_response>
{
    using T = binapi2::fapi::types::index_constituents_response;
    static constexpr auto value = object("symbol", &T::symbol, "time", &T::time, "constituents", &T::constituents);
};

template<>
struct glz::meta<binapi2::fapi::types::asset_index>
{
    using T = binapi2::fapi::types::asset_index;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "time",
                                         &T::time,
                                         "index",
                                         &T::index,
                                         "bidBuffer",
                                         &T::bidBuffer,
                                         "askBuffer",
                                         &T::askBuffer,
                                         "bidRate",
                                         &T::bidRate,
                                         "askRate",
                                         &T::askRate,
                                         "autoExchangeBidBuffer",
                                         &T::autoExchangeBidBuffer,
                                         "autoExchangeAskBuffer",
                                         &T::autoExchangeAskBuffer,
                                         "autoExchangeBidRate",
                                         &T::autoExchangeBidRate,
                                         "autoExchangeAskRate",
                                         &T::autoExchangeAskRate);
};

template<>
struct glz::meta<binapi2::fapi::types::insurance_fund_asset>
{
    using T = binapi2::fapi::types::insurance_fund_asset;
    static constexpr auto value = object("asset", &T::asset, "marginBalance", &T::marginBalance, "updateTime", &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::insurance_fund_response>
{
    using T = binapi2::fapi::types::insurance_fund_response;
    static constexpr auto value = object("symbols", &T::symbols, "assets", &T::assets);
};

template<>
struct glz::meta<binapi2::fapi::types::adl_risk_entry>
{
    using T = binapi2::fapi::types::adl_risk_entry;
    static constexpr auto value = object("symbol", &T::symbol, "adlRisk", &T::adlRisk, "updateTime", &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_schedule_response>
{
    using T = binapi2::fapi::types::trading_schedule_response;
    static constexpr auto value = object("updateTime", &T::updateTime, "marketSchedules", &T::marketSchedules);
};
