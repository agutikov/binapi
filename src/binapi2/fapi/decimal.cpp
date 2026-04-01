// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file decimal.cpp
/// @brief Implementation of the fixed-precision decimal type.

#include <binapi2/fapi/types/decimal.hpp>

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace binapi2::fapi::types {

namespace {

/// Parse a decimal string into a scaled int128 with fixed 18 decimal places.
/// Throws std::overflow_error if the integer part exceeds int128 range.
/// Throws std::invalid_argument if there are more than 18 fractional digits.
decimal::int128_t parse_decimal(std::string_view s)
{
    using int128_t = decimal::int128_t;

    if (s.empty())
        return 0;

    bool negative = false;
    std::size_t pos = 0;

    if (s[0] == '-') {
        negative = true;
        pos = 1;
    } else if (s[0] == '+') {
        pos = 1;
    }

    // Parse integer part.
    int128_t integer_part = 0;
    // Max int128 is ~1.7e38. With 18 decimal digits of scale, the integer
    // part can be at most ~1.7e20 (about 21 digits). We track digit count
    // to detect overflow before it wraps.
    static constexpr int max_integer_digits = 20;
    int integer_digits = 0;

    while (pos < s.size() && s[pos] != '.') {
        if (s[pos] < '0' || s[pos] > '9')
            throw std::invalid_argument("decimal: invalid character in '" + std::string(s) + "'");
        integer_part = integer_part * 10 + (s[pos] - '0');
        ++integer_digits;
        if (integer_digits > max_integer_digits)
            throw std::overflow_error("decimal: integer part too large in '" + std::string(s) + "'");
        ++pos;
    }

    // Parse fractional part.
    int128_t frac_part = 0;
    std::uint8_t frac_digits = 0;

    if (pos < s.size() && s[pos] == '.') {
        ++pos;
        while (pos < s.size()) {
            if (s[pos] < '0' || s[pos] > '9')
                throw std::invalid_argument("decimal: invalid character in '" + std::string(s) + "'");
            frac_part = frac_part * 10 + (s[pos] - '0');
            ++frac_digits;
            ++pos;
        }
    }

    if (frac_digits > decimal::scale)
        throw std::invalid_argument(
            "decimal: more than 18 fractional digits in '" + std::string(s) + "'");

    // Scale the integer part to 18 decimal places.
    int128_t result = integer_part * decimal::scale_factor;

    // Scale the fractional part up to fill the remaining decimal places.
    // E.g. "1.5" has frac_part=5, frac_digits=1, needs 17 more zeros.
    int128_t frac_scale = 1;
    for (std::uint8_t i = 0; i < decimal::scale - frac_digits; ++i)
        frac_scale *= 10;

    result += frac_part * frac_scale;

    // Check for overflow: integer_part * 10^18 must not have wrapped.
    // If the integer part was too large, the result would be smaller than expected.
    if (integer_part != 0 && result / decimal::scale_factor != integer_part)
        throw std::overflow_error("decimal: value too large in '" + std::string(s) + "'");

    return negative ? -result : result;
}

} // anonymous namespace

// -- Constructors --

decimal::decimal(const char* s) : value(parse_decimal(std::string_view(s))) {}
decimal::decimal(const std::string& s) : value(parse_decimal(std::string_view(s))) {}
decimal::decimal(std::string_view s) : value(parse_decimal(s)) {}

constexpr decimal::decimal(int128_t raw, int input_scale) : value(raw)
{
    // Scale from input_scale to 18.
    if (input_scale < scale) {
        for (int i = 0; i < scale - input_scale; ++i)
            value *= 10;
    } else if (input_scale > scale) {
        for (int i = 0; i < input_scale - scale; ++i)
            value /= 10;
    }
}

// -- to_string --

std::string
decimal::to_string() const
{
    if (value == 0)
        return "0";

    bool negative = value < 0;
    int128_t abs_val = negative ? -value : value;

    // Extract integer and fractional parts.
    int128_t int_part = abs_val / scale_factor;
    int128_t frac_part = abs_val % scale_factor;

    // Convert integer part to string.
    std::string int_str;
    if (int_part == 0) {
        int_str = "0";
    } else {
        while (int_part > 0) {
            int_str.push_back(static_cast<char>('0' + static_cast<int>(int_part % 10)));
            int_part /= 10;
        }
        std::reverse(int_str.begin(), int_str.end());
    }

    if (frac_part == 0) {
        return negative ? ("-" + int_str) : int_str;
    }

    // Convert fractional part to a zero-padded string of exactly 18 digits,
    // then trim trailing zeros.
    std::string frac_str(scale, '0');
    for (int i = scale - 1; i >= 0 && frac_part > 0; --i) {
        frac_str[static_cast<std::size_t>(i)] = static_cast<char>('0' + static_cast<int>(frac_part % 10));
        frac_part /= 10;
    }

    // Trim trailing zeros.
    auto last_nonzero = frac_str.find_last_not_of('0');
    if (last_nonzero != std::string::npos)
        frac_str.resize(last_nonzero + 1);

    std::string result;
    if (negative) result.push_back('-');
    result += int_str;
    result.push_back('.');
    result += frac_str;
    return result;
}

// -- to_double --

double
decimal::to_double() const
{
    // Split to avoid losing precision in the cast.
    bool negative = value < 0;
    int128_t abs_val = negative ? -value : value;
    int128_t int_part = abs_val / scale_factor;
    int128_t frac_part = abs_val % scale_factor;

    double result = static_cast<double>(int_part)
                  + static_cast<double>(frac_part) / static_cast<double>(scale_factor);
    return negative ? -result : result;
}

// -- operator<< --

std::ostream&
operator<<(std::ostream& os, const decimal& d)
{
    return os << d.to_string();
}

} // namespace binapi2::fapi::types
