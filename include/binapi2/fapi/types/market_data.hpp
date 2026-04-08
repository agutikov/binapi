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
struct ping_request_t
{};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
struct server_time_request_t
{};

// ---------------------------------------------------------------------------
// Exchange information
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct exchange_info_request_t
{
    std::optional<symbol_t> symbol{};
};

// ---------------------------------------------------------------------------
// Order book
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
struct order_book_request_t
{
    symbol_t symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
struct order_book_response_t
{
    std::uint64_t lastUpdateId{};
    timestamp_ms_t message_output_time{};
    timestamp_ms_t transaction_time{};
    std::vector<price_level_t> bids{};
    std::vector<price_level_t> asks{};
};

// ---------------------------------------------------------------------------
// Trades (recent, historical, aggregate)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
struct recent_trades_request_t
{
    symbol_t symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
struct recent_trade_t
{
    std::uint64_t id{};
    decimal_t price{};
    decimal_t qty{};
    decimal_t quoteQty{};
    timestamp_ms_t time{};
    bool isBuyerMaker{};
    std::optional<bool> isRPITrade{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
struct aggregate_trades_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> fromId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
struct aggregate_trade_t
{
    std::uint64_t agg_trade_id{};
    decimal_t price{};
    decimal_t quantity{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    timestamp_ms_t timestamp{};
    bool is_buyer_maker{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Old-Trades-Lookup.md
struct historical_trades_request_t
{
    symbol_t symbol{};
    std::optional<int> limit{};
    std::optional<std::uint64_t> fromId{};
};

// ---------------------------------------------------------------------------
// Klines / Candlesticks
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
struct kline_request_t
{
    symbol_t symbol{};
    kline_interval_t interval{ kline_interval_t::m1 };
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Continuous-Contract-Kline-Candlestick-Data.md
struct continuous_kline_request_t
{
    pair_t pair{};
    contract_type_t contractType{ contract_type_t::perpetual };
    kline_interval_t interval{ kline_interval_t::m1 };
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Price-Kline-Candlestick-Data.md
struct index_price_kline_request_t
{
    pair_t pair{};
    kline_interval_t interval{ kline_interval_t::m1 };
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

/// Kline (candlestick) data. Serialized as a positional JSON array (not an
/// object), so field order matters. Fields map to array indices:
/// [openTime, open, high, low, close, volume, closeTime, quoteAssetVolume,
///  numberOfTrades, takerBuyBaseAssetVolume, takerBuyQuoteAssetVolume, ignore].
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
struct kline_t
{
    timestamp_ms_t openTime{};
    decimal_t open{};
    decimal_t high{};
    decimal_t low{};
    decimal_t close{};
    decimal_t volume{};
    timestamp_ms_t closeTime{};
    decimal_t quoteAssetVolume{};
    std::uint64_t numberOfTrades{};
    decimal_t takerBuyBaseAssetVolume{};
    decimal_t takerBuyQuoteAssetVolume{};
    decimal_t ignore{};
};

// ---------------------------------------------------------------------------
// Tickers (book, price, 24hr)
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
struct book_ticker_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
struct book_ticker_t
{
    symbol_t symbol{};
    decimal_t bidPrice{};
    decimal_t bidQty{};
    decimal_t askPrice{};
    decimal_t askQty{};
    timestamp_ms_t time{};
    std::optional<std::uint64_t> lastUpdateId{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
struct price_ticker_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
struct price_ticker_t
{
    symbol_t symbol{};
    decimal_t price{};
    timestamp_ms_t time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
struct ticker_24hr_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
struct ticker_24hr_t
{
    symbol_t symbol{};
    decimal_t priceChange{};
    decimal_t priceChangePercent{};
    decimal_t weightedAvgPrice{};
    decimal_t lastPrice{};
    decimal_t lastQty{};
    decimal_t openPrice{};
    decimal_t highPrice{};
    decimal_t lowPrice{};
    decimal_t volume{};
    decimal_t quoteVolume{};
    timestamp_ms_t openTime{};
    timestamp_ms_t closeTime{};
    std::uint64_t firstId{};
    std::uint64_t lastId{};
    std::uint64_t count{};
};

// ---------------------------------------------------------------------------
// Mark price and funding rates
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
struct mark_price_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
struct mark_price_t
{
    symbol_t symbol{};
    decimal_t markPrice{};
    decimal_t indexPrice{};
    decimal_t estimatedSettlePrice{};
    decimal_t lastFundingRate{};
    decimal_t interestRate{};
    timestamp_ms_t nextFundingTime{};
    timestamp_ms_t time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
struct funding_rate_history_request_t
{
    std::optional<symbol_t> symbol{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
struct funding_rate_history_entry_t
{
    symbol_t symbol{};
    decimal_t fundingRate{};
    timestamp_ms_t fundingTime{};
    decimal_t markPrice{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-Info.md
struct funding_rate_info_t
{
    symbol_t symbol{};
    decimal_t adjustedFundingRateCap{};
    decimal_t adjustedFundingRateFloor{};
    int fundingIntervalHours{};
    bool disclaimer{};
};

// ---------------------------------------------------------------------------
// Open interest and futures analytics
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
struct open_interest_request_t
{
    symbol_t symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
struct open_interest_t
{
    decimal_t openInterest{};
    symbol_t symbol{};
    timestamp_ms_t time{};
};

/// Generic request for futures analytics endpoints (open interest statistics,
/// long/short ratio, taker volume). Shares the same parameter shape.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
struct futures_data_request_t
{
    symbol_t symbol{};
    futures_data_period_t period{ futures_data_period_t::m5 };
    std::optional<int> limit{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
struct open_interest_statistics_entry_t
{
    symbol_t symbol{};
    decimal_t sumOpenInterest{};
    decimal_t sumOpenInterestValue{};
    decimal_t CMCCirculatingSupply{};
    timestamp_ms_t timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Long-Short-Ratio.md
struct long_short_ratio_entry_t
{
    symbol_t symbol{};
    decimal_t longShortRatio{};
    decimal_t longAccount{};
    decimal_t shortAccount{};
    timestamp_ms_t timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Taker-BuySell-Volume.md
struct taker_buy_sell_volume_entry_t
{
    decimal_t buySellRatio{};
    decimal_t buyVol{};
    decimal_t sellVol{};
    timestamp_ms_t timestamp{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
struct basis_request_t
{
    pair_t pair{};
    contract_type_t contractType{ contract_type_t::perpetual };
    futures_data_period_t period{ futures_data_period_t::m5 };
    std::optional<int> limit{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
struct basis_entry_t
{
    pair_t pair{};
    contract_type_t contractType{};
    decimal_t basis{};
    decimal_t basisRate{};
    decimal_t futuresPrice{};
    decimal_t indexPrice{};
    decimal_t annualizedBasisRate{};
    timestamp_ms_t timestamp{};
};

// ---------------------------------------------------------------------------
// Delivery, composite index, index constituents, asset index
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker-v2.md
struct price_ticker_v2_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
struct delivery_price_request_t
{
    pair_t pair{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
struct delivery_price_entry_t
{
    timestamp_ms_t deliveryTime{};
    decimal_t deliveryPrice{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_info_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_base_asset_t
{
    std::string baseAsset{};
    std::string quoteAsset{};
    decimal_t weightInQuantity{};
    decimal_t weightInPercentage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
struct composite_index_info_t
{
    symbol_t symbol{};
    timestamp_ms_t time{};
    std::string component{};
    std::vector<composite_index_base_asset_t> baseAssetList{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituents_request_t
{
    symbol_t symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituent_t
{
    std::string exchange{};
    symbol_t symbol{};
    decimal_t price{};
    decimal_t weight{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
struct index_constituents_response_t
{
    symbol_t symbol{};
    timestamp_ms_t time{};
    std::vector<index_constituent_t> constituents{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
struct asset_index_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
struct asset_index_t
{
    symbol_t symbol{};
    timestamp_ms_t time{};
    decimal_t index{};
    decimal_t bidBuffer{};
    decimal_t askBuffer{};
    decimal_t bidRate{};
    decimal_t askRate{};
    decimal_t autoExchangeBidBuffer{};
    decimal_t autoExchangeAskBuffer{};
    decimal_t autoExchangeBidRate{};
    decimal_t autoExchangeAskRate{};
};

// ---------------------------------------------------------------------------
// Insurance fund, ADL risk, RPI depth, trading schedule
// ---------------------------------------------------------------------------

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_asset_t
{
    std::string asset{};
    decimal_t marginBalance{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
struct insurance_fund_response_t
{
    std::vector<std::string> symbols{};
    std::vector<insurance_fund_asset_t> assets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
struct adl_risk_request_t
{
    std::optional<symbol_t> symbol{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
struct adl_risk_entry_t
{
    symbol_t symbol{};
    decimal_t adlRisk{};
    timestamp_ms_t updateTime{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book-RPI.md
struct rpi_depth_request_t
{
    symbol_t symbol{};
    std::optional<int> limit{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_schedule_request_t
{};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_session_entry_t
{
    timestamp_ms_t startTime{};
    timestamp_ms_t endTime{};
    std::string type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct market_schedule_t
{
    std::vector<trading_session_entry_t> sessions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
struct trading_schedule_response_t
{
    timestamp_ms_t updateTime{};
    std::map<std::string, market_schedule_t> marketSchedules{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::order_book_response_t>
{
    using T = binapi2::fapi::types::order_book_response_t;
    static constexpr auto value =
        object("lastUpdateId", &T::lastUpdateId, "E", &T::message_output_time, "T", &T::transaction_time, "bids", &T::bids, "asks", &T::asks);
};

template<>
struct glz::meta<binapi2::fapi::types::recent_trade_t>
{
    using T = binapi2::fapi::types::recent_trade_t;
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
struct glz::meta<binapi2::fapi::types::aggregate_trade_t>
{
    using T = binapi2::fapi::types::aggregate_trade_t;
    static constexpr auto value = object("a", &T::agg_trade_id, "p", &T::price, "q", &T::quantity, "f", &T::first_trade_id, "l", &T::last_trade_id, "T", &T::timestamp, "m", &T::is_buyer_maker);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_t>
{
    using T = binapi2::fapi::types::kline_t;
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
struct glz::meta<binapi2::fapi::types::book_ticker_t>
{
    using T = binapi2::fapi::types::book_ticker_t;
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
struct glz::meta<binapi2::fapi::types::price_ticker_t>
{
    using T = binapi2::fapi::types::price_ticker_t;
    static constexpr auto value = object("symbol", &T::symbol, "price", &T::price, "time", &T::time);
};

template<>
struct glz::meta<binapi2::fapi::types::ticker_24hr_t>
{
    using T = binapi2::fapi::types::ticker_24hr_t;
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
struct glz::meta<binapi2::fapi::types::mark_price_t>
{
    using T = binapi2::fapi::types::mark_price_t;
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
struct glz::meta<binapi2::fapi::types::funding_rate_history_entry_t>
{
    using T = binapi2::fapi::types::funding_rate_history_entry_t;
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
struct glz::meta<binapi2::fapi::types::funding_rate_info_t>
{
    using T = binapi2::fapi::types::funding_rate_info_t;
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
struct glz::meta<binapi2::fapi::types::open_interest_t>
{
    using T = binapi2::fapi::types::open_interest_t;
    static constexpr auto value = object("openInterest", &T::openInterest, "symbol", &T::symbol, "time", &T::time);
};

template<>
struct glz::meta<binapi2::fapi::types::open_interest_statistics_entry_t>
{
    using T = binapi2::fapi::types::open_interest_statistics_entry_t;
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
struct glz::meta<binapi2::fapi::types::long_short_ratio_entry_t>
{
    using T = binapi2::fapi::types::long_short_ratio_entry_t;
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
struct glz::meta<binapi2::fapi::types::taker_buy_sell_volume_entry_t>
{
    using T = binapi2::fapi::types::taker_buy_sell_volume_entry_t;
    static constexpr auto value =
        object("buySellRatio", &T::buySellRatio, "buyVol", &T::buyVol, "sellVol", &T::sellVol, "timestamp", &T::timestamp);
};

template<>
struct glz::meta<binapi2::fapi::types::basis_entry_t>
{
    using T = binapi2::fapi::types::basis_entry_t;
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
struct glz::meta<binapi2::fapi::types::delivery_price_entry_t>
{
    using T = binapi2::fapi::types::delivery_price_entry_t;
    static constexpr auto value = object("deliveryTime", &T::deliveryTime, "deliveryPrice", &T::deliveryPrice);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_base_asset_t>
{
    using T = binapi2::fapi::types::composite_index_base_asset_t;
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
struct glz::meta<binapi2::fapi::types::composite_index_info_t>
{
    using T = binapi2::fapi::types::composite_index_info_t;
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
struct glz::meta<binapi2::fapi::types::index_constituent_t>
{
    using T = binapi2::fapi::types::index_constituent_t;
    static constexpr auto value =
        object("exchange", &T::exchange, "symbol", &T::symbol, "price", &T::price, "weight", &T::weight);
};

template<>
struct glz::meta<binapi2::fapi::types::index_constituents_response_t>
{
    using T = binapi2::fapi::types::index_constituents_response_t;
    static constexpr auto value = object("symbol", &T::symbol, "time", &T::time, "constituents", &T::constituents);
};

template<>
struct glz::meta<binapi2::fapi::types::asset_index_t>
{
    using T = binapi2::fapi::types::asset_index_t;
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
struct glz::meta<binapi2::fapi::types::insurance_fund_asset_t>
{
    using T = binapi2::fapi::types::insurance_fund_asset_t;
    static constexpr auto value = object("asset", &T::asset, "marginBalance", &T::marginBalance, "updateTime", &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::insurance_fund_response_t>
{
    using T = binapi2::fapi::types::insurance_fund_response_t;
    static constexpr auto value = object("symbols", &T::symbols, "assets", &T::assets);
};

template<>
struct glz::meta<binapi2::fapi::types::adl_risk_entry_t>
{
    using T = binapi2::fapi::types::adl_risk_entry_t;
    static constexpr auto value = object("symbol", &T::symbol, "adlRisk", &T::adlRisk, "updateTime", &T::updateTime);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_session_entry_t>
{
    using T = binapi2::fapi::types::trading_session_entry_t;
    static constexpr auto value = object("startTime", &T::startTime, "endTime", &T::endTime, "type", &T::type);
};

template<>
struct glz::meta<binapi2::fapi::types::market_schedule_t>
{
    using T = binapi2::fapi::types::market_schedule_t;
    static constexpr auto value = object("sessions", &T::sessions);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_schedule_response_t>
{
    using T = binapi2::fapi::types::trading_schedule_response_t;
    static constexpr auto value = object("updateTime", &T::updateTime, "marketSchedules", &T::marketSchedules);
};
