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
#include <utility>

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
    using uint128_t = unsigned __int128;
#pragma GCC diagnostic pop


    static constexpr int scale = 18;
    static constexpr int128_t scale_factor = static_cast<int128_t>(1'000'000'000) * 1'000'000'000;

    /// Maximum valid decimal number: 10^19 (raw 10^37).
    /// Guarantees: max + max fits in int128 (no hardware overflow on +/-).
    static constexpr decimal max() { return decimal(max_raw_, raw_tag{}); }
    static constexpr decimal min() { return decimal(min_raw_, raw_tag{}); }

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

    /// Serialize to a decimal string with a decimal point, trimming trailing
    /// zeros but always keeping at least one fractional digit.
    /// E.g. 1500000000000000000 -> "1.5", 0 -> "0.0", 1e18 -> "1.0".
    [[nodiscard]] std::string to_string() const;

    /// Convert to double. Returns {value, fits} where fits==true means the
    /// raw int128 round-trips through double without precision loss.
    [[nodiscard]] std::pair<double, bool> to_double() const;

    /// Convert to long double. Returns {value, fits} where fits==true means the
    /// raw int128 round-trips through long double without precision loss.
    [[nodiscard]] std::pair<long double, bool> to_long_double() const;

    // -- Queries --

    [[nodiscard]] constexpr bool is_zero() const { return value == 0; }
    [[nodiscard]] constexpr bool is_negative() const { return value < 0; }
    [[nodiscard]] constexpr bool is_positive() const { return value > 0; }

    /// True if value is within [min, max]. False indicates overflow from arithmetic.
    [[nodiscard]] constexpr bool is_valid() const { return value >= min_raw_ && value <= max_raw_; }

    // -- Comparison (trivial — same scale, just compare the int128) --

    [[nodiscard]] constexpr auto operator<=>(const decimal& rhs) const = default;

    // -- Arithmetic --

    [[nodiscard]] constexpr decimal operator+(const decimal& rhs) const { return decimal(value + rhs.value, raw_tag{}); }
    [[nodiscard]] constexpr decimal operator-(const decimal& rhs) const { return decimal(value - rhs.value, raw_tag{}); }
    [[nodiscard]] constexpr decimal operator-() const { return decimal(-value, raw_tag{}); }

    /// Multiplication via 256-bit intermediate. Truncates sub-10^-18 remainder.
    /// Throws std::overflow_error if the result exceeds int128 range.
    [[nodiscard]] decimal operator*(const decimal& rhs) const;
    decimal& operator*=(const decimal& rhs);

    constexpr decimal& operator+=(const decimal& rhs) { value += rhs.value; return *this; }
    constexpr decimal& operator-=(const decimal& rhs) { value -= rhs.value; return *this; }

private:
    // Raw int128 limits: 10^37 (decimal 10^19). 2 * 10^37 < 2^127, so +/- of two
    // valid decimals never overflows the underlying int128.
    static constexpr int128_t max_raw_ =
        scale_factor * (static_cast<int128_t>(10'000'000'000) * 1'000'000'000); // 10^37
    static constexpr int128_t min_raw_ = -max_raw_;
};

/// Remainder from decimal mul/div. Separate type prevents accidental
/// arithmetic with decimal values.
///
/// For mul(a,b): value = |a.value * b.value| % 10^18, at scale 10^-36.
/// For div(a,b): value = |a.value * 10^18| % |b.value|; actual error = value / |b.value| * 10^-18.
/// Sign follows the sign of the result.
struct decimal_error
{
    decimal::int128_t value{};
    [[nodiscard]] constexpr bool is_zero() const { return value == 0; }
    [[nodiscard]] constexpr auto operator<=>(const decimal_error&) const = default;
    /// Scientific notation of the raw int128 value.
    [[nodiscard]] std::string to_string() const;
};

/// Overflow from decimal mul/div — nonzero means result exceeded int128 range.
/// Separate type prevents accidental arithmetic with decimal values.
struct decimal_overflow
{
    decimal::int128_t value{};
    [[nodiscard]] constexpr bool is_zero() const { return value == 0; }
    [[nodiscard]] constexpr auto operator<=>(const decimal_overflow&) const = default;
    /// Scientific notation of the raw int128 value.
    [[nodiscard]] std::string to_string() const;
};

/// Complete result of decimal mul or div.
struct decimal_result
{
    decimal value;              ///< Main result at scale 10^-18.
    decimal_error error;        ///< Precision remainder (scale depends on operation).
    decimal_overflow overflow;  ///< Nonzero if result exceeded int128 range.

    [[nodiscard]] constexpr bool is_exact() const { return error.is_zero() && overflow.is_zero(); }
    [[nodiscard]] constexpr bool has_overflow() const { return !overflow.is_zero(); }
};

/// Safe multiplication via 256-bit intermediate with overflow detection.
[[nodiscard]] decimal_result mul(decimal a, decimal b);

/// Safe division via 256-bit intermediate with overflow detection.
/// Throws std::domain_error on division by zero.
[[nodiscard]] decimal_result div(decimal a, decimal b);

std::ostream& operator<<(std::ostream& os, const decimal& d);
std::ostream& operator<<(std::ostream& os, const decimal_error& e);
std::ostream& operator<<(std::ostream& os, const decimal_overflow& o);

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
