// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file decimal.hpp
/// @brief Fixed-precision decimal type backed by __int128 with 18 decimal places.
///
/// All Binance monetary values (prices, quantities, rates, balances) are
/// represented as this type. The internal representation is a 128-bit scaled
/// integer with a fixed scale of 18 decimal places, giving a range of
/// approximately +/- 1.7 * 10^20 with 18 digits of fractional precision.
///
/// Throws std::overflow_error if a parsed string exceeds the representable
/// range, and std::invalid_argument if it has more than 18 fractional digits.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>

namespace binapi2::fapi::types {

/// Fixed-precision decimal number: int128 value with 18 implicit decimal places.
///
/// Internally stores `value = unscaled_integer * 10^(-18)`. For example,
/// "1.5" is stored as 1'500'000'000'000'000'000. This representation enables
/// exact comparison and arithmetic without floating-point rounding.
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

    static constexpr int scale = 18;
    static constexpr int128_t scale_factor = static_cast<int128_t>(1'000'000'000) * 1'000'000'000;

    int128_t value{}; ///< Unscaled integer: actual_value = value * 10^(-scale).

    // -- Construction --

    constexpr decimal() = default;

    /// Construct from a raw value already at scale 18.
    /// Use decimal(raw, input_scale) to convert from a different scale.
    struct raw_tag {};
    explicit constexpr decimal(int128_t raw, raw_tag) : value(raw) {}

    /// Construct from an integer with arbitrary scale.
    /// Scales the value to 18 decimal places. Throws if the result overflows.
    constexpr decimal(int128_t raw, int input_scale);

    /// Parse from a decimal string. Throws std::overflow_error if the integer
    /// part is too large, std::invalid_argument if more than 18 fractional digits.
    decimal(const char* s);                // NOLINT
    decimal(const std::string& s);         // NOLINT
    decimal(std::string_view s);           // NOLINT

    // -- String conversion --

    /// Serialize to a decimal string, trimming trailing zeros after the decimal
    /// point. E.g. 1500000000000000000 -> "1.5", 0 -> "0".
    [[nodiscard]] std::string to_string() const;

    /// Approximate double conversion. Precision may be lost for large values.
    [[nodiscard]] double to_double() const;

    // -- Queries --

    [[nodiscard]] constexpr bool is_zero() const { return value == 0; }
    [[nodiscard]] constexpr bool is_negative() const { return value < 0; }
    [[nodiscard]] constexpr bool is_positive() const { return value > 0; }

    // -- Comparison (trivial — same scale, just compare the int128) --

    [[nodiscard]] constexpr auto operator<=>(const decimal& rhs) const = default;

    // -- Arithmetic --

    [[nodiscard]] constexpr decimal operator+(const decimal& rhs) const { return decimal(value + rhs.value, raw_tag{}); }
    [[nodiscard]] constexpr decimal operator-(const decimal& rhs) const { return decimal(value - rhs.value, raw_tag{}); }
    [[nodiscard]] constexpr decimal operator-() const { return decimal(-value, raw_tag{}); }

    /// Multiplication: (a * 10^-18) * (b * 10^-18) = (a*b) * 10^-36,
    /// so we divide by 10^18 to get back to the correct scale.
    [[nodiscard]] constexpr decimal operator*(const decimal& rhs) const
    {
        return decimal(value * rhs.value / scale_factor, raw_tag{});
    }

    constexpr decimal& operator+=(const decimal& rhs) { value += rhs.value; return *this; }
    constexpr decimal& operator-=(const decimal& rhs) { value -= rhs.value; return *this; }
    constexpr decimal& operator*=(const decimal& rhs) { return *this = *this * rhs; }
};

std::ostream& operator<<(std::ostream& os, const decimal& d);

} // namespace binapi2::fapi::types

// -- Glaze JSON serialization --

#include <glaze/glaze.hpp>

namespace glz {

template<>
struct from<JSON, binapi2::fapi::types::decimal>
{
    template<auto Opts>
    static void op(binapi2::fapi::types::decimal& d, is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string s;
        parse<JSON>::op<Opts>(s, ctx, it, end);
        d = binapi2::fapi::types::decimal(s);
    }
};

template<>
struct to<JSON, binapi2::fapi::types::decimal>
{
    template<auto Opts>
    static void op(const binapi2::fapi::types::decimal& d, is_context auto&& ctx, auto&& b, auto&& ix) noexcept
    {
        auto s = d.to_string();
        serialize<JSON>::op<Opts>(s, ctx, b, ix);
    }
};

} // namespace glz
