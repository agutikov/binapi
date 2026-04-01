// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file decimal.hpp
/// @brief Fixed-precision decimal type backed by __int128 for exact arithmetic
///        on prices, quantities, and other monetary values from Binance.
///
/// Binance sends all prices, quantities, rates, and balances as quoted decimal
/// strings in JSON. This type parses them into a scaled integer (__int128) plus
/// a scale (number of decimal places), enabling exact arithmetic without
/// floating-point rounding. JSON serialization round-trips through strings.

#pragma once

#include <glaze/glaze.hpp>

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace binapi2::fapi::types {

/// Fixed-precision decimal number backed by a 128-bit scaled integer.
///
/// Internally stores `unscaled * 10^(-scale)`. For example, "123.456" is
/// stored as unscaled=123456, scale=3. This preserves the exact Binance
/// precision and supports exact comparison and arithmetic.
///
/// Implicitly constructible from string literals for ergonomic request building:
/// @code
///   new_order_request req{ .price = "50000.10", .quantity = "0.001" };
/// @endcode
struct decimal
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    using int128_t = __int128;
#pragma GCC diagnostic pop

    int128_t unscaled{}; ///< Value multiplied by 10^scale.
    std::uint8_t scale{}; ///< Number of decimal places.

    // -- Construction --

    constexpr decimal() = default;

    /// Parse from a decimal string (e.g. "123.456", "-0.01", "42").
    decimal(const char* s) { parse(std::string_view(s)); }         // NOLINT
    decimal(const std::string& s) { parse(std::string_view(s)); }  // NOLINT
    decimal(std::string_view s) { parse(s); }                      // NOLINT

    /// Construct from an unscaled integer and scale directly.
    constexpr decimal(int128_t unscaled_, std::uint8_t scale_)
        : unscaled(unscaled_), scale(scale_) {}

    // -- String conversion --

    /// Serialize to a decimal string preserving the original scale.
    /// E.g. decimal(123456, 3) -> "123.456", decimal(-1, 2) -> "-0.01".
    [[nodiscard]] std::string to_string() const
    {
        if (unscaled == 0 && scale == 0)
            return "0";

        bool negative = unscaled < 0;
        int128_t abs_val = negative ? -unscaled : unscaled;

        std::string digits;
        if (abs_val == 0) {
            digits = "0";
        } else {
            while (abs_val > 0) {
                digits.push_back(static_cast<char>('0' + static_cast<int>(abs_val % 10)));
                abs_val /= 10;
            }
            std::reverse(digits.begin(), digits.end());
        }

        if (scale == 0) {
            return negative ? ("-" + digits) : digits;
        }

        // Pad with leading zeros if needed: e.g. unscaled=1, scale=3 -> "0.001"
        while (digits.size() <= scale) {
            digits.insert(digits.begin(), '0');
        }

        std::string result;
        if (negative) result.push_back('-');
        result.append(digits, 0, digits.size() - scale);
        result.push_back('.');
        result.append(digits, digits.size() - scale, scale);
        return result;
    }

    /// Approximate double conversion. Precision may be lost.
    [[nodiscard]] double to_double() const
    {
        double result = static_cast<double>(unscaled);
        for (std::uint8_t i = 0; i < scale; ++i)
            result /= 10.0;
        return result;
    }

    // -- Queries --

    [[nodiscard]] constexpr bool is_zero() const { return unscaled == 0; }
    [[nodiscard]] constexpr bool is_negative() const { return unscaled < 0; }
    [[nodiscard]] constexpr bool is_positive() const { return unscaled > 0; }

    // -- Comparison --

    /// Exact comparison, normalizing scale differences.
    [[nodiscard]] constexpr auto operator<=>(const decimal& rhs) const
    {
        auto [lhs_u, rhs_u] = normalize(rhs);
        return lhs_u <=> rhs_u;
    }

    [[nodiscard]] constexpr bool operator==(const decimal& rhs) const
    {
        auto [lhs_u, rhs_u] = normalize(rhs);
        return lhs_u == rhs_u;
    }

    // -- Arithmetic --

    /// Addition. Result scale is max(lhs.scale, rhs.scale).
    [[nodiscard]] constexpr decimal operator+(const decimal& rhs) const
    {
        auto target_scale = std::max(scale, rhs.scale);
        auto lhs_u = scale_to(unscaled, scale, target_scale);
        auto rhs_u = scale_to(rhs.unscaled, rhs.scale, target_scale);
        return {lhs_u + rhs_u, target_scale};
    }

    /// Subtraction.
    [[nodiscard]] constexpr decimal operator-(const decimal& rhs) const
    {
        auto target_scale = std::max(scale, rhs.scale);
        auto lhs_u = scale_to(unscaled, scale, target_scale);
        auto rhs_u = scale_to(rhs.unscaled, rhs.scale, target_scale);
        return {lhs_u - rhs_u, target_scale};
    }

    /// Unary negation.
    [[nodiscard]] constexpr decimal operator-() const { return {-unscaled, scale}; }

    /// Multiplication. Result scale is lhs.scale + rhs.scale.
    [[nodiscard]] constexpr decimal operator*(const decimal& rhs) const
    {
        return {unscaled * rhs.unscaled, static_cast<std::uint8_t>(scale + rhs.scale)};
    }

    constexpr decimal& operator+=(const decimal& rhs) { return *this = *this + rhs; }
    constexpr decimal& operator-=(const decimal& rhs) { return *this = *this - rhs; }
    constexpr decimal& operator*=(const decimal& rhs) { return *this = *this * rhs; }

private:
    void parse(std::string_view s)
    {
        if (s.empty()) {
            unscaled = 0;
            scale = 0;
            return;
        }

        bool negative = false;
        std::size_t pos = 0;

        if (s[0] == '-') {
            negative = true;
            pos = 1;
        } else if (s[0] == '+') {
            pos = 1;
        }

        int128_t integer_part = 0;
        while (pos < s.size() && s[pos] != '.') {
            integer_part = integer_part * 10 + (s[pos] - '0');
            ++pos;
        }

        int128_t frac_part = 0;
        std::uint8_t frac_digits = 0;

        if (pos < s.size() && s[pos] == '.') {
            ++pos;
            while (pos < s.size()) {
                frac_part = frac_part * 10 + (s[pos] - '0');
                ++frac_digits;
                ++pos;
            }
        }

        int128_t pow10 = 1;
        for (std::uint8_t i = 0; i < frac_digits; ++i)
            pow10 *= 10;

        unscaled = integer_part * pow10 + frac_part;
        if (negative) unscaled = -unscaled;
        scale = frac_digits;
    }

    /// Scale an unscaled value from `from_scale` to `to_scale`.
    [[nodiscard]] static constexpr int128_t scale_to(int128_t val, std::uint8_t from_scale, std::uint8_t to_scale)
    {
        if (from_scale < to_scale) {
            for (std::uint8_t i = 0; i < to_scale - from_scale; ++i)
                val *= 10;
        } else {
            for (std::uint8_t i = 0; i < from_scale - to_scale; ++i)
                val /= 10;
        }
        return val;
    }

    /// Normalize two decimals to the same scale and return their unscaled values.
    [[nodiscard]] constexpr std::pair<int128_t, int128_t> normalize(const decimal& rhs) const
    {
        auto target_scale = std::max(scale, rhs.scale);
        return {scale_to(unscaled, scale, target_scale), scale_to(rhs.unscaled, rhs.scale, target_scale)};
    }
};

inline std::ostream& operator<<(std::ostream& os, const decimal& d) { return os << d.to_string(); }

} // namespace binapi2::fapi::types

/// Glaze custom serialization: decimal is read/written as a bare JSON string,
/// round-tripping through to_string()/parse().
namespace glz {

template<>
struct from<JSON, binapi2::fapi::types::decimal>
{
    template<auto Opts>
    static void op(binapi2::fapi::types::decimal& value, is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string s;
        parse<JSON>::op<Opts>(s, ctx, it, end);
        value = binapi2::fapi::types::decimal(s);
    }
};

template<>
struct to<JSON, binapi2::fapi::types::decimal>
{
    template<auto Opts>
    static void op(const binapi2::fapi::types::decimal& value, is_context auto&& ctx, auto&& b, auto&& ix) noexcept
    {
        auto s = value.to_string();
        serialize<JSON>::op<Opts>(s, ctx, b, ix);
    }
};

} // namespace glz
