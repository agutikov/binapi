// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <glaze/glaze.hpp>
#include <cstdint>
#include <optional>
#include <string>

namespace binapi2::fapi::types {

struct convert_quote_request {
    std::string fromAsset{};
    std::string toAsset{};
    std::optional<std::string> fromAmount{};
    std::optional<std::string> toAmount{};
    std::optional<std::string> validTime{};
};

struct convert_quote_response {
    std::string quoteId{};
    std::string ratio{};
    std::string inverseRatio{};
    std::uint64_t validTimestamp{};
    std::string toAmount{};
    std::string fromAmount{};
};

struct convert_accept_request {
    std::string quoteId{};
};

struct convert_accept_response {
    std::string orderId{};
    std::uint64_t createTime{};
    std::string orderStatus{};
};

struct convert_order_status_request {
    std::optional<std::string> orderId{};
    std::optional<std::string> quoteId{};
};

struct convert_order_status_response {
    std::string orderId{};
    std::string orderStatus{};
    std::string fromAsset{};
    std::string fromAmount{};
    std::string toAsset{};
    std::string toAmount{};
    std::string ratio{};
    std::string inverseRatio{};
    std::uint64_t createTime{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::convert_quote_response>
{
    using T = binapi2::fapi::types::convert_quote_response;
    static constexpr auto value = object("quoteId",
                                         &T::quoteId,
                                         "ratio",
                                         &T::ratio,
                                         "inverseRatio",
                                         &T::inverseRatio,
                                         "validTimestamp",
                                         &T::validTimestamp,
                                         "toAmount",
                                         &T::toAmount,
                                         "fromAmount",
                                         &T::fromAmount);
};

template<>
struct glz::meta<binapi2::fapi::types::convert_accept_response>
{
    using T = binapi2::fapi::types::convert_accept_response;
    static constexpr auto value = object("orderId",
                                         &T::orderId,
                                         "createTime",
                                         &T::createTime,
                                         "orderStatus",
                                         &T::orderStatus);
};

template<>
struct glz::meta<binapi2::fapi::types::convert_order_status_response>
{
    using T = binapi2::fapi::types::convert_order_status_response;
    static constexpr auto value = object("orderId",
                                         &T::orderId,
                                         "orderStatus",
                                         &T::orderStatus,
                                         "fromAsset",
                                         &T::fromAsset,
                                         "fromAmount",
                                         &T::fromAmount,
                                         "toAsset",
                                         &T::toAsset,
                                         "toAmount",
                                         &T::toAmount,
                                         "ratio",
                                         &T::ratio,
                                         "inverseRatio",
                                         &T::inverseRatio,
                                         "createTime",
                                         &T::createTime);
};
