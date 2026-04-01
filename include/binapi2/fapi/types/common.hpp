// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file common.hpp
/// @brief Shared types used across multiple Binance USD-M Futures API endpoints.
///
/// Contains fundamental response/request types: rate limits, price levels,
/// exchange information, symbol filters, and error documents.

#pragma once

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

/// Placeholder response type for endpoints that return no payload (e.g. DELETE listen key).
struct empty_response
{};

/// API rate limit descriptor. Returned inside exchange_info_response and
/// WebSocket API responses. The optional `count` field is populated in
/// WebSocket API responses to indicate current usage.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
struct rate_limit
{
    std::string rateLimitType{};
    std::string interval{};
    int intervalNum{};
    int limit{};
    std::optional<int> count{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
struct server_time_response
{
    std::uint64_t serverTime{};
};

/// Standard Binance error response body (HTTP 4xx/5xx).
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
struct binance_error_document
{
    int code{};
    std::string msg{};
};

/// A single [price, quantity] level in an order book.
/// Serialized as a JSON array (positional), not an object.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
struct price_level
{
    std::string price{};
    std::string quantity{};
};

/// Asset descriptor from exchange info. Indicates whether the asset can be
/// used as margin and its auto-exchange threshold.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct exchange_info_asset
{
    std::string asset{};
    bool marginAvailable{};
    std::optional<std::string> autoAssetExchange{};
};

/// Symbol trading filter from exchange info. This is a union-like type:
/// filterType determines which optional fields are populated. For example,
/// PRICE_FILTER sets minPrice/maxPrice/tickSize, LOT_SIZE sets minQty/maxQty/stepSize,
/// PERCENT_PRICE sets multiplierUp/multiplierDown, etc.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct symbol_filter
{
    std::string filterType{};
    std::optional<std::string> maxPrice{};
    std::optional<std::string> minPrice{};
    std::optional<std::string> tickSize{};
    std::optional<std::string> maxQty{};
    std::optional<std::string> minQty{};
    std::optional<std::string> stepSize{};
    std::optional<int> limit{};
    std::optional<std::string> notional{};
    std::optional<std::string> multiplierUp{};
    std::optional<std::string> multiplierDown{};
    std::optional<std::string> multiplierDecimal{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct symbol_info
{
    std::string symbol{};
    std::string pair{};
    std::string contractType{};
    std::uint64_t deliveryDate{};
    std::uint64_t onboardDate{};
    std::string status{};
    std::string maintMarginPercent{};
    std::string requiredMarginPercent{};
    std::string baseAsset{};
    std::string quoteAsset{};
    std::string marginAsset{};
    int pricePrecision{};
    int quantityPrecision{};
    int baseAssetPrecision{};
    int quotePrecision{};
    std::string underlyingType{};
    std::vector<std::string> underlyingSubType{};
    int settlePlan{};
    std::string triggerProtect{};
    std::vector<symbol_filter> filters{};
    std::vector<std::string> orderTypes{};
    std::vector<std::string> timeInForce{};
    std::string liquidationFee{};
    std::string marketTakeBound{};
};

/// Full exchange information response. Contains rate limits, supported assets,
/// and per-symbol trading rules/filters.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
struct exchange_info_response
{
    std::string timezone{};
    std::uint64_t serverTime{};
    std::vector<glz::generic> exchangeFilters{};
    std::vector<rate_limit> rateLimits{};
    std::vector<exchange_info_asset> assets{};
    std::vector<symbol_info> symbols{};
};

/// Response from creating/querying a user data stream listen key.
// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream.md
struct listen_key_response
{
    std::string listenKey{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::empty_response>
{
    using T = binapi2::fapi::types::empty_response;
    static constexpr auto value = object();
};

template<>
struct glz::meta<binapi2::fapi::types::rate_limit>
{
    using T = binapi2::fapi::types::rate_limit;
    static constexpr auto value = object("rateLimitType",
                                         &T::rateLimitType,
                                         "interval",
                                         &T::interval,
                                         "intervalNum",
                                         &T::intervalNum,
                                         "limit",
                                         &T::limit,
                                         "count",
                                         &T::count);
};

template<>
struct glz::meta<binapi2::fapi::types::server_time_response>
{
    using T = binapi2::fapi::types::server_time_response;
    static constexpr auto value = object("serverTime", &T::serverTime);
};

template<>
struct glz::meta<binapi2::fapi::types::binance_error_document>
{
    using T = binapi2::fapi::types::binance_error_document;
    static constexpr auto value = object("code", &T::code, "msg", &T::msg);
};

template<>
struct glz::meta<binapi2::fapi::types::price_level>
{
    using T = binapi2::fapi::types::price_level;
    static constexpr auto value = array(&T::price, &T::quantity);
};

template<>
struct glz::meta<binapi2::fapi::types::exchange_info_asset>
{
    using T = binapi2::fapi::types::exchange_info_asset;
    static constexpr auto value =
        object("asset", &T::asset, "marginAvailable", &T::marginAvailable, "autoAssetExchange", &T::autoAssetExchange);
};

template<>
struct glz::meta<binapi2::fapi::types::symbol_filter>
{
    using T = binapi2::fapi::types::symbol_filter;
    static constexpr auto value = object("filterType",
                                         &T::filterType,
                                         "maxPrice",
                                         &T::maxPrice,
                                         "minPrice",
                                         &T::minPrice,
                                         "tickSize",
                                         &T::tickSize,
                                         "maxQty",
                                         &T::maxQty,
                                         "minQty",
                                         &T::minQty,
                                         "stepSize",
                                         &T::stepSize,
                                         "limit",
                                         &T::limit,
                                         "notional",
                                         &T::notional,
                                         "multiplierUp",
                                         &T::multiplierUp,
                                         "multiplierDown",
                                         &T::multiplierDown,
                                         "multiplierDecimal",
                                         &T::multiplierDecimal);
};

template<>
struct glz::meta<binapi2::fapi::types::symbol_info>
{
    using T = binapi2::fapi::types::symbol_info;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "pair",
                                         &T::pair,
                                         "contractType",
                                         &T::contractType,
                                         "deliveryDate",
                                         &T::deliveryDate,
                                         "onboardDate",
                                         &T::onboardDate,
                                         "status",
                                         &T::status,
                                         "maintMarginPercent",
                                         &T::maintMarginPercent,
                                         "requiredMarginPercent",
                                         &T::requiredMarginPercent,
                                         "baseAsset",
                                         &T::baseAsset,
                                         "quoteAsset",
                                         &T::quoteAsset,
                                         "marginAsset",
                                         &T::marginAsset,
                                         "pricePrecision",
                                         &T::pricePrecision,
                                         "quantityPrecision",
                                         &T::quantityPrecision,
                                         "baseAssetPrecision",
                                         &T::baseAssetPrecision,
                                         "quotePrecision",
                                         &T::quotePrecision,
                                         "underlyingType",
                                         &T::underlyingType,
                                         "underlyingSubType",
                                         &T::underlyingSubType,
                                         "settlePlan",
                                         &T::settlePlan,
                                         "triggerProtect",
                                         &T::triggerProtect,
                                         "filters",
                                         &T::filters,
                                         "orderTypes",
                                         &T::orderTypes,
                                         "timeInForce",
                                         &T::timeInForce,
                                         "liquidationFee",
                                         &T::liquidationFee,
                                         "marketTakeBound",
                                         &T::marketTakeBound);
};

template<>
struct glz::meta<binapi2::fapi::types::exchange_info_response>
{
    using T = binapi2::fapi::types::exchange_info_response;
    static constexpr auto value = object("timezone",
                                         &T::timezone,
                                         "serverTime",
                                         &T::serverTime,
                                         "exchangeFilters",
                                         &T::exchangeFilters,
                                         "rateLimits",
                                         &T::rateLimits,
                                         "assets",
                                         &T::assets,
                                         "symbols",
                                         &T::symbols);
};

template<>
struct glz::meta<binapi2::fapi::types::listen_key_response>
{
    using T = binapi2::fapi::types::listen_key_response;
    static constexpr auto value = object("listenKey", &T::listenKey);
};
