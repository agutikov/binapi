// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_data.hpp
/// @brief Request and response types for Binance USD-M Futures market data endpoints.
///
/// Types are grouped by functional area: connectivity, order book, trades,
/// klines, tickers, funding rates, open interest, and analytics.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// ---------------------------------------------------------------------------
// Connectivity / Server status
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
struct ping_request
{};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
struct server_time_request
{};

// ---------------------------------------------------------------------------
// Exchange information
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct exchange_info_request
{
    std::optional<std::string> symbol{};
};

// ---------------------------------------------------------------------------
// Order book
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
struct order_book_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
struct order_book_response
{
    std::uint64_t lastUpdateId{};
    timestamp_ms message_output_time{};
    timestamp_ms transaction_time{};
    std::vector<price_level> bids{};
    std::vector<price_level> asks{};
};

// ---------------------------------------------------------------------------
// Trades (recent, historical, aggregate)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
struct recent_trades_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
struct recent_trade
{
    std::uint64_t id{};
    decimal price{};
    decimal qty{};
    decimal quoteQty{};
    timestamp_ms time{};
    bool isBuyerMaker{};
    std::optional<bool> isRPITrade{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
struct aggregate_trades_request
{
    std::string symbol{};
    std::optional<std::uint64_t> fromId{};
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
struct aggregate_trade
{
    std::uint64_t agg_trade_id{};
    decimal price{};
    decimal quantity{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    timestamp_ms timestamp{};
    bool is_buyer_maker{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Old-Trades-Lookup.md
struct historical_trades_request
{
    std::string symbol{};
    std::optional<int> limit{};
    std::optional<std::uint64_t> fromId{};
};

// ---------------------------------------------------------------------------
// Klines / Candlesticks
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
struct kline_request
{
    std::string symbol{};
    kline_interval interval{ kline_interval::m1 };
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Continuous-Contract-Kline-Candlestick-Data.md
struct continuous_kline_request
{
    std::string pair{};
    contract_type contractType{ contract_type::perpetual };
    kline_interval interval{ kline_interval::m1 };
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Price-Kline-Candlestick-Data.md
struct index_price_kline_request
{
    std::string pair{};
    kline_interval interval{ kline_interval::m1 };
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> limit{};
};

/// Kline (candlestick) data. Serialized as a positional JSON array (not an
/// object), so field order matters. Fields map to array indices:
/// [openTime, open, high, low, close, volume, closeTime, quoteAssetVolume,
///  numberOfTrades, takerBuyBaseAssetVolume, takerBuyQuoteAssetVolume, ignore].
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
struct kline
{
    timestamp_ms openTime{};
    decimal open{};
    decimal high{};
    decimal low{};
    decimal close{};
    decimal volume{};
    timestamp_ms closeTime{};
    decimal quoteAssetVolume{};
    std::uint64_t numberOfTrades{};
    decimal takerBuyBaseAssetVolume{};
    decimal takerBuyQuoteAssetVolume{};
    decimal ignore{};
};

// ---------------------------------------------------------------------------
// Tickers (book, price, 24hr)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
struct book_ticker_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
struct book_ticker
{
    std::string symbol{};
    decimal bidPrice{};
    decimal bidQty{};
    decimal askPrice{};
    decimal askQty{};
    timestamp_ms time{};
    std::uint64_t lastUpdateId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
struct price_ticker_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
struct price_ticker
{
    std::string symbol{};
    decimal price{};
    timestamp_ms time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
struct ticker_24hr_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
struct ticker_24hr
{
    std::string symbol{};
    decimal priceChange{};
    decimal priceChangePercent{};
    decimal weightedAvgPrice{};
    decimal lastPrice{};
    decimal lastQty{};
    decimal openPrice{};
    decimal highPrice{};
    decimal lowPrice{};
    decimal volume{};
    decimal quoteVolume{};
    timestamp_ms openTime{};
    timestamp_ms closeTime{};
    std::uint64_t firstId{};
    std::uint64_t lastId{};
    std::uint64_t count{};
};

// ---------------------------------------------------------------------------
// Mark price and funding rates
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
struct mark_price_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
struct mark_price
{
    std::string symbol{};
    decimal markPrice{};
    decimal indexPrice{};
    decimal estimatedSettlePrice{};
    decimal lastFundingRate{};
    decimal interestRate{};
    timestamp_ms nextFundingTime{};
    timestamp_ms time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
struct funding_rate_history_request
{
    std::optional<std::string> symbol{};
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
struct funding_rate_history_entry
{
    std::string symbol{};
    decimal fundingRate{};
    timestamp_ms fundingTime{};
    decimal markPrice{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-Info.md
struct funding_rate_info
{
    std::string symbol{};
    decimal adjustedFundingRateCap{};
    decimal adjustedFundingRateFloor{};
    int fundingIntervalHours{};
    bool disclaimer{};
};

// ---------------------------------------------------------------------------
// Open interest and futures analytics
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
struct open_interest_request
{
    std::string symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
struct open_interest
{
    decimal openInterest{};
    std::string symbol{};
    timestamp_ms time{};
};

/// Generic request for futures analytics endpoints (open interest statistics,
/// long/short ratio, taker volume). Shares the same parameter shape.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
struct futures_data_request
{
    std::string symbol{};
    futures_data_period period{ futures_data_period::m5 };
    std::optional<int> limit{};
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
struct open_interest_statistics_entry
{
    std::string symbol{};
    decimal sumOpenInterest{};
    decimal sumOpenInterestValue{};
    decimal CMCCirculatingSupply{};
    timestamp_ms timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Long-Short-Ratio.md
struct long_short_ratio_entry
{
    std::string symbol{};
    decimal longShortRatio{};
    decimal longAccount{};
    decimal shortAccount{};
    timestamp_ms timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Taker-BuySell-Volume.md
struct taker_buy_sell_volume_entry
{
    decimal buySellRatio{};
    decimal buyVol{};
    decimal sellVol{};
    timestamp_ms timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
struct basis_request
{
    std::string pair{};
    contract_type contractType{ contract_type::perpetual };
    futures_data_period period{ futures_data_period::m5 };
    std::optional<int> limit{};
    std::optional<timestamp_ms> startTime{};
    std::optional<timestamp_ms> endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
struct basis_entry
{
    std::string pair{};
    contract_type contractType{};
    decimal basis{};
    decimal basisRate{};
    decimal futuresPrice{};
    decimal indexPrice{};
    decimal annualizedBasisRate{};
    timestamp_ms timestamp{};
};

// ---------------------------------------------------------------------------
// Delivery, composite index, index constituents, asset index
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker-v2.md
struct price_ticker_v2_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
struct delivery_price_request
{
    std::string pair{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
struct delivery_price_entry
{
    timestamp_ms deliveryTime{};
    decimal deliveryPrice{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_info_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_base_asset
{
    std::string baseAsset{};
    std::string quoteAsset{};
    decimal weightInQuantity{};
    decimal weightInPercentage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_info
{
    std::string symbol{};
    timestamp_ms time{};
    std::string component{};
    std::vector<composite_index_base_asset> baseAssetList{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituents_request
{
    std::string symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituent
{
    std::string exchange{};
    std::string symbol{};
    decimal price{};
    decimal weight{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituents_response
{
    std::string symbol{};
    timestamp_ms time{};
    std::vector<index_constituent> constituents{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
struct asset_index_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
struct asset_index
{
    std::string symbol{};
    timestamp_ms time{};
    decimal index{};
    decimal bidBuffer{};
    decimal askBuffer{};
    decimal bidRate{};
    decimal askRate{};
    decimal autoExchangeBidBuffer{};
    decimal autoExchangeAskBuffer{};
    decimal autoExchangeBidRate{};
    decimal autoExchangeAskRate{};
};

// ---------------------------------------------------------------------------
// Insurance fund, ADL risk, RPI depth, trading schedule
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_asset
{
    std::string asset{};
    decimal marginBalance{};
    timestamp_ms updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_response
{
    std::vector<std::string> symbols{};
    std::vector<insurance_fund_asset> assets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
struct adl_risk_request
{
    std::optional<std::string> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
struct adl_risk_entry
{
    std::string symbol{};
    decimal adlRisk{};
    timestamp_ms updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book-RPI.md
struct rpi_depth_request
{
    std::string symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_schedule_request
{};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_session_entry
{
    timestamp_ms startTime{};
    timestamp_ms endTime{};
    std::string type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct market_schedule
{
    std::vector<trading_session_entry> sessions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_schedule_response
{
    timestamp_ms updateTime{};
    std::map<std::string, market_schedule> marketSchedules{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::order_book_response>
{
    using T = binapi2::fapi::types::order_book_response;
    static constexpr auto value =
        object("lastUpdateId", &T::lastUpdateId, "E", &T::message_output_time, "T", &T::transaction_time, "bids", &T::bids, "asks", &T::asks);
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
    static constexpr auto value = object("a", &T::agg_trade_id, "p", &T::price, "q", &T::quantity, "f", &T::first_trade_id, "l", &T::last_trade_id, "T", &T::timestamp, "m", &T::is_buyer_maker);
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
                                         "interestRate",
                                         &T::interestRate,
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
struct glz::meta<binapi2::fapi::types::trading_session_entry>
{
    using T = binapi2::fapi::types::trading_session_entry;
    static constexpr auto value = object("startTime", &T::startTime, "endTime", &T::endTime, "type", &T::type);
};

template<>
struct glz::meta<binapi2::fapi::types::market_schedule>
{
    using T = binapi2::fapi::types::market_schedule;
    static constexpr auto value = object("sessions", &T::sessions);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_schedule_response>
{
    using T = binapi2::fapi::types::trading_schedule_response;
    static constexpr auto value = object("updateTime", &T::updateTime, "marketSchedules", &T::marketSchedules);
};
