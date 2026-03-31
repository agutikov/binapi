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

struct empty_response
{};

struct rate_limit
{
    std::string rateLimitType{};
    std::string interval{};
    int intervalNum{};
    int limit{};
    std::optional<int> count{};
};

struct server_time_response
{
    std::uint64_t serverTime{};
};

struct binance_error_document
{
    int code{};
    std::string msg{};
};

struct price_level
{
    std::string price{};
    std::string quantity{};
};

struct symbol_info
{
    std::string symbol{};
    std::string pair{};
    std::string contractType{};
    std::string status{};
    std::string baseAsset{};
    std::string quoteAsset{};
    std::string marginAsset{};
};

struct exchange_info_response
{
    std::string timezone{};
    std::uint64_t serverTime{};
    std::vector<rate_limit> rateLimits{};
    std::vector<symbol_info> symbols{};
};

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
struct glz::meta<binapi2::fapi::types::symbol_info>
{
    using T = binapi2::fapi::types::symbol_info;
    static constexpr auto value = object("symbol",
                                         &T::symbol,
                                         "pair",
                                         &T::pair,
                                         "contractType",
                                         &T::contractType,
                                         "status",
                                         &T::status,
                                         "baseAsset",
                                         &T::baseAsset,
                                         "quoteAsset",
                                         &T::quoteAsset,
                                         "marginAsset",
                                         &T::marginAsset);
};

template<>
struct glz::meta<binapi2::fapi::types::exchange_info_response>
{
    using T = binapi2::fapi::types::exchange_info_response;
    static constexpr auto value =
        object("timezone", &T::timezone, "serverTime", &T::serverTime, "rateLimits", &T::rateLimits, "symbols", &T::symbols);
};

template<>
struct glz::meta<binapi2::fapi::types::listen_key_response>
{
    using T = binapi2::fapi::types::listen_key_response;
    static constexpr auto value = object("listenKey", &T::listenKey);
};
