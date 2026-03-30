#pragma once

#include <binapi2/umf/types/common.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::umf::types {

struct ping_request {};
struct server_time_request {};
struct exchange_info_request {
    std::optional<std::string> symbol{};
};

struct order_book_request {
    std::string symbol{};
    std::optional<int> limit{};
};

struct order_book_response {
    std::uint64_t lastUpdateId{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::vector<price_level> bids{};
    std::vector<price_level> asks{};
};

struct recent_trades_request {
    std::string symbol{};
    std::optional<int> limit{};
};

struct recent_trade {
    std::uint64_t id{};
    std::string price{};
    std::string qty{};
    std::string quoteQty{};
    std::uint64_t time{};
    bool isBuyerMaker{};
};

struct book_ticker_request {
    std::optional<std::string> symbol{};
};

struct book_ticker {
    std::string symbol{};
    std::string bidPrice{};
    std::string bidQty{};
    std::string askPrice{};
    std::string askQty{};
    std::uint64_t time{};
    std::uint64_t lastUpdateId{};
};

struct price_ticker_request {
    std::optional<std::string> symbol{};
};

struct price_ticker {
    std::string symbol{};
    std::string price{};
    std::uint64_t time{};
};

struct ticker_24hr_request {
    std::optional<std::string> symbol{};
};

struct ticker_24hr {
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

struct mark_price_request {
    std::optional<std::string> symbol{};
};

struct mark_price {
    std::string symbol{};
    std::string markPrice{};
    std::string indexPrice{};
    std::string estimatedSettlePrice{};
    std::string lastFundingRate{};
    std::uint64_t nextFundingTime{};
    std::uint64_t time{};
};

struct open_interest_request {
    std::string symbol{};
};

struct open_interest {
    std::string openInterest{};
    std::string symbol{};
    std::uint64_t time{};
};

} // namespace binapi2::umf::types

template <>
struct glz::meta<binapi2::umf::types::order_book_response> {
    using T = binapi2::umf::types::order_book_response;
    static constexpr auto value = object(
        "lastUpdateId", &T::lastUpdateId,
        "E", &T::E,
        "T", &T::T,
        "bids", &T::bids,
        "asks", &T::asks
    );
};

template <>
struct glz::meta<binapi2::umf::types::recent_trade> {
    using T = binapi2::umf::types::recent_trade;
    static constexpr auto value = object(
        "id", &T::id,
        "price", &T::price,
        "qty", &T::qty,
        "quoteQty", &T::quoteQty,
        "time", &T::time,
        "isBuyerMaker", &T::isBuyerMaker
    );
};

template <>
struct glz::meta<binapi2::umf::types::book_ticker> {
    using T = binapi2::umf::types::book_ticker;
    static constexpr auto value = object(
        "symbol", &T::symbol,
        "bidPrice", &T::bidPrice,
        "bidQty", &T::bidQty,
        "askPrice", &T::askPrice,
        "askQty", &T::askQty,
        "time", &T::time,
        "lastUpdateId", &T::lastUpdateId
    );
};

template <>
struct glz::meta<binapi2::umf::types::price_ticker> {
    using T = binapi2::umf::types::price_ticker;
    static constexpr auto value = object(
        "symbol", &T::symbol,
        "price", &T::price,
        "time", &T::time
    );
};

template <>
struct glz::meta<binapi2::umf::types::ticker_24hr> {
    using T = binapi2::umf::types::ticker_24hr;
    static constexpr auto value = object(
        "symbol", &T::symbol,
        "priceChange", &T::priceChange,
        "priceChangePercent", &T::priceChangePercent,
        "weightedAvgPrice", &T::weightedAvgPrice,
        "lastPrice", &T::lastPrice,
        "lastQty", &T::lastQty,
        "openPrice", &T::openPrice,
        "highPrice", &T::highPrice,
        "lowPrice", &T::lowPrice,
        "volume", &T::volume,
        "quoteVolume", &T::quoteVolume,
        "openTime", &T::openTime,
        "closeTime", &T::closeTime,
        "firstId", &T::firstId,
        "lastId", &T::lastId,
        "count", &T::count
    );
};

template <>
struct glz::meta<binapi2::umf::types::mark_price> {
    using T = binapi2::umf::types::mark_price;
    static constexpr auto value = object(
        "symbol", &T::symbol,
        "markPrice", &T::markPrice,
        "indexPrice", &T::indexPrice,
        "estimatedSettlePrice", &T::estimatedSettlePrice,
        "lastFundingRate", &T::lastFundingRate,
        "nextFundingTime", &T::nextFundingTime,
        "time", &T::time
    );
};

template <>
struct glz::meta<binapi2::umf::types::open_interest> {
    using T = binapi2::umf::types::open_interest;
    static constexpr auto value = object(
        "openInterest", &T::openInterest,
        "symbol", &T::symbol,
        "time", &T::time
    );
};
