// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file pair.hpp
/// @brief Strong type wrapper for Binance trading pairs (e.g. "BTCUSDT").
///
/// A trading pair identifies the base/quote asset combination (e.g. "ETHUSDT").
/// For perpetual contracts, pair and symbol are identical. For delivery contracts,
/// the symbol includes an expiry suffix (e.g. "ETHUSDT_250627") while the pair
/// remains "ETHUSDT".

#pragma once

#include <algorithm>
#include <cctype>
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>

namespace binapi2::fapi::types {

/// A Binance trading pair (e.g. "BTCUSDT", "ETHUSDT").
///
/// Thin wrapper over std::string. Implicitly convertible from string literals
/// and std::string for ergonomic initialisation, and explicitly convertible
/// back via str() / operator string_view.
class pair_t
{
    std::string value_{};

    static std::string to_upper(std::string s)
    {
        std::ranges::transform(s, s.begin(), [](unsigned char c) { return std::toupper(c); });
        return s;
    }

public:
    pair_t() = default;
    pair_t(const char* s) : value_(to_upper(s)) {}                 // NOLINT(google-explicit-constructor)
    pair_t(std::string s) : value_(to_upper(std::move(s))) {}      // NOLINT(google-explicit-constructor)
    pair_t(std::string_view s) : value_(to_upper(std::string(s))) {} // NOLINT(google-explicit-constructor)

    [[nodiscard]] const std::string& str() const noexcept { return value_; }
    [[nodiscard]] operator const std::string&() const noexcept { return value_; }  // NOLINT(google-explicit-constructor)
    [[nodiscard]] std::string_view view() const noexcept { return value_; }
    [[nodiscard]] const char* c_str() const noexcept { return value_.c_str(); }
    [[nodiscard]] bool empty() const noexcept { return value_.empty(); }

    bool operator==(const pair_t&) const = default;
    auto operator<=>(const pair_t&) const = default;

    bool operator==(const std::string& s) const { return value_ == s; }
    bool operator==(std::string_view s) const { return value_ == s; }
    bool operator==(const char* s) const { return value_ == s; }

    friend std::ostream& operator<<(std::ostream& os, const pair_t& p)
    {
        return os << p.value_;
    }

    friend std::string operator+(const std::string& lhs, const pair_t& rhs) { return lhs + rhs.value_; }
    friend std::string operator+(const pair_t& lhs, const std::string& rhs) { return lhs.value_ + rhs; }
    friend std::string operator+(const pair_t& lhs, const char* rhs) { return lhs.value_ + rhs; }
    friend std::string operator+(const char* lhs, const pair_t& rhs) { return lhs + rhs.value_; }
};

} // namespace binapi2::fapi::types

// ---------------------------------------------------------------------------
// Glaze JSON serialization — read/write pair_t as a plain JSON string.
// ---------------------------------------------------------------------------

#include <glaze/glaze.hpp>

namespace glz {

template<>
struct from<JSON, binapi2::fapi::types::pair_t>
{
    template<auto Opts>
    static void op(binapi2::fapi::types::pair_t& p, is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string tmp;
        parse<JSON>::op<Opts>(tmp, ctx, it, end);
        p = binapi2::fapi::types::pair_t(std::move(tmp));
    }
};

template<>
struct to<JSON, binapi2::fapi::types::pair_t>
{
    template<auto Opts>
    static void op(const binapi2::fapi::types::pair_t& p, is_context auto&& ctx, auto&& b, auto&& ix) noexcept
    {
        serialize<JSON>::op<Opts>(p.str(), ctx, b, ix);
    }
};

} // namespace glz

// ---------------------------------------------------------------------------
// fmt formatter
// ---------------------------------------------------------------------------

#include <fmt/format.h>

template<>
struct fmt::formatter<binapi2::fapi::types::pair_t> : fmt::formatter<std::string_view>
{
    auto format(const binapi2::fapi::types::pair_t& p, fmt::format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(p.view(), ctx);
    }
};
