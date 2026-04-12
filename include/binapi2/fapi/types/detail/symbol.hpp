// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file symbol.hpp
/// @brief Strong type wrapper for Binance trading pair symbols (e.g. "BTCUSDT").
///
/// Provides type safety so that a symbol cannot be accidentally confused with
/// other string parameters (client order IDs, asset names, etc.).

#pragma once

#include <algorithm>
#include <cctype>
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>

namespace binapi2::fapi::types {

/// A Binance trading pair symbol (e.g. "BTCUSDT", "ETHUSDT").
///
/// Thin wrapper over std::string. Implicitly convertible from string literals
/// and std::string for ergonomic initialisation, and explicitly convertible
/// back via str() / operator string_view.
class symbol_t
{
    std::string value_{};

    static std::string to_upper(std::string s)
    {
        std::ranges::transform(s, s.begin(), [](unsigned char c) { return std::toupper(c); });
        return s;
    }

public:
    symbol_t() = default;
    symbol_t(const char* s) : value_(to_upper(s)) {}                 // NOLINT(google-explicit-constructor)
    symbol_t(std::string s) : value_(to_upper(std::move(s))) {}      // NOLINT(google-explicit-constructor)
    symbol_t(std::string_view s) : value_(to_upper(std::string(s))) {} // NOLINT(google-explicit-constructor)

    [[nodiscard]] const std::string& str() const noexcept { return value_; }
    [[nodiscard]] operator const std::string&() const noexcept { return value_; }  // NOLINT(google-explicit-constructor)
    [[nodiscard]] std::string_view view() const noexcept { return value_; }
    [[nodiscard]] const char* c_str() const noexcept { return value_.c_str(); }
    [[nodiscard]] bool empty() const noexcept { return value_.empty(); }

    bool operator==(const symbol_t&) const = default;
    auto operator<=>(const symbol_t&) const = default;

    /// Allow comparison with raw strings.
    bool operator==(const std::string& s) const { return value_ == s; }
    bool operator==(std::string_view s) const { return value_ == s; }
    bool operator==(const char* s) const { return value_ == s; }

    friend std::ostream& operator<<(std::ostream& os, const symbol_t& s)
    {
        return os << s.value_;
    }

    friend std::string operator+(const std::string& lhs, const symbol_t& rhs) { return lhs + rhs.value_; }
    friend std::string operator+(const symbol_t& lhs, const std::string& rhs) { return lhs.value_ + rhs; }
    friend std::string operator+(const symbol_t& lhs, const char* rhs) { return lhs.value_ + rhs; }
    friend std::string operator+(const char* lhs, const symbol_t& rhs) { return lhs + rhs.value_; }
};

} // namespace binapi2::fapi::types

// ---------------------------------------------------------------------------
// Glaze JSON serialization — read/write symbol_t as a plain JSON string.
// ---------------------------------------------------------------------------

#include <glaze/glaze.hpp>

namespace glz {

template<>
struct from<JSON, binapi2::fapi::types::symbol_t>
{
    template<auto Opts>
    static void op(binapi2::fapi::types::symbol_t& s, is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string tmp;
        parse<JSON>::op<Opts>(tmp, ctx, it, end);
        s = binapi2::fapi::types::symbol_t(std::move(tmp));
    }
};

template<>
struct to<JSON, binapi2::fapi::types::symbol_t>
{
    template<auto Opts>
    static void op(const binapi2::fapi::types::symbol_t& s, is_context auto&& ctx, auto&& b, auto&& ix) noexcept
    {
        serialize<JSON>::op<Opts>(s.str(), ctx, b, ix);
    }
};

} // namespace glz

// ---------------------------------------------------------------------------
// fmt formatter
// ---------------------------------------------------------------------------

#include <fmt/format.h>

/// spdlog / fmt formatter support — format as the underlying string.
template<>
struct fmt::formatter<binapi2::fapi::types::symbol_t> : fmt::formatter<std::string_view>
{
    auto format(const binapi2::fapi::types::symbol_t& s, fmt::format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(s.view(), ctx);
    }
};
