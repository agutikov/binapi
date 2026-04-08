// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file decimal.cpp
/// @brief Implementation of the fixed-precision decimal type.

#include <binapi2/fapi/types/detail/decimal.hpp>

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace binapi2::fapi::types {

namespace {

using int128_t  = decimal_t::int128_t;
using uint128_t = decimal_t::uint128_t;

// -- 256-bit unsigned helpers (manual, no compiler extension) --

struct uint256_t {
    uint128_t lo{};
    uint128_t hi{};
};

/// Negate int128 to uint128, safe even for INT128_MIN.
uint128_t to_unsigned(int128_t v) {
    if (v >= 0) return static_cast<uint128_t>(v);
    return ~static_cast<uint128_t>(v) + 1;
}

/// Multiply two uint128 → uint256 (schoolbook on 64-bit halves).
uint256_t mul256(uint128_t a, uint128_t b) {
    auto a0 = static_cast<std::uint64_t>(a);
    auto a1 = static_cast<std::uint64_t>(a >> 64);
    auto b0 = static_cast<std::uint64_t>(b);
    auto b1 = static_cast<std::uint64_t>(b >> 64);

    uint128_t p00 = static_cast<uint128_t>(a0) * b0;
    uint128_t p01 = static_cast<uint128_t>(a0) * b1;
    uint128_t p10 = static_cast<uint128_t>(a1) * b0;
    uint128_t p11 = static_cast<uint128_t>(a1) * b1;

    uint128_t mid       = p01 + p10;
    uint128_t mid_carry = (mid < p01) ? 1 : 0;

    uint128_t lo       = p00 + (mid << 64);
    uint128_t lo_carry = (lo < p00) ? 1 : 0;

    uint128_t hi = p11 + (mid >> 64) + (mid_carry << 64) + lo_carry;

    return {lo, hi};
}

/// Divide uint256 by uint128 → {quotient (uint256), remainder (uint128)}.
/// Uses bit-by-bit long division (256 iterations).
struct divmod_result {
    uint256_t quotient;
    uint128_t remainder;
};

divmod_result divmod256(uint256_t n, uint128_t d) {
    if (n.hi == 0 && n.lo < d)
        return {{0, 0}, n.lo};

    if (n.hi == 0)
        return {{n.lo / d, 0}, n.lo % d};

    uint256_t  q{0, 0};
    uint128_t rem = 0;

    for (int i = 255; i >= 0; --i) {
        // Detect if left-shift of rem will overflow uint128.
        bool rem_high = (rem >> 127) & 1;
        rem <<= 1;

        uint128_t bit = (i >= 128)
            ? (n.hi >> (i - 128)) & 1
            : (n.lo >> i) & 1;
        rem |= bit;

        // If rem_high, the true remainder is rem + 2^128 > d, always subtract.
        if (rem_high || rem >= d) {
            rem -= d;
            if (i >= 128) q.hi |= uint128_t(1) << (i - 128);
            else          q.lo |= uint128_t(1) << i;
        }
    }

    return {q, rem};
}

/// Pack a 256-bit unsigned quotient + sign into decimal_result_t fields.
decimal_result_t pack_result(uint256_t quot, uint128_t rem, bool negative) {
    // int128 hardware limit (2^127 - 1).
    constexpr uint128_t int128_max = ~uint128_t(0) >> 1;

    int128_t result_raw;
    decimal_overflow_t overflow{};

    if (quot.hi == 0 && quot.lo <= int128_max) {
        result_raw = static_cast<int128_t>(quot.lo);
    } else {
        overflow.value = (quot.hi != 0)
            ? static_cast<int128_t>(quot.hi)
            : int128_t{1};
        // Store low bits; not meaningful on its own when has_overflow().
        result_raw = static_cast<int128_t>(quot.lo & int128_max);
    }

    auto rem_signed = static_cast<int128_t>(rem);

    if (negative) {
        result_raw = -result_raw;
        overflow.value = -overflow.value;
        rem_signed = -rem_signed;
    }

    return {
        decimal_t(result_raw, decimal_t::raw_tag{}),
        decimal_error_t{rem_signed},
        overflow
    };
}

/// Parse a decimal string into a scaled int128 with fixed 18 decimal places.
/// Throws std::overflow_error if the integer part exceeds int128 range.
/// Throws std::invalid_argument if there are more than 18 fractional digits.
decimal_t::int128_t parse_decimal_t(std::string_view s)
{
    using int128_t = decimal_t::int128_t;

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

    if (frac_digits > decimal_t::scale)
        throw std::invalid_argument(
            "decimal: more than 18 fractional digits in '" + std::string(s) + "'");

    // Scale the integer part to 18 decimal places.
    int128_t result = integer_part * decimal_t::scale_factor;

    // Scale the fractional part up to fill the remaining decimal places.
    // E.g. "1.5" has frac_part=5, frac_digits=1, needs 17 more zeros.
    int128_t frac_scale = 1;
    for (std::uint8_t i = 0; i < decimal_t::scale - frac_digits; ++i)
        frac_scale *= 10;

    result += frac_part * frac_scale;

    // Check for overflow: integer_part * 10^18 must not have wrapped.
    // If the integer part was too large, the result would be smaller than expected.
    if (integer_part != 0 && result / decimal_t::scale_factor != integer_part)
        throw std::overflow_error("decimal: value too large in '" + std::string(s) + "'");

    return negative ? -result : result;
}

} // anonymous namespace

// -- Constructors --

decimal_t::decimal_t(const char* s) : value(parse_decimal_t(std::string_view(s))) {}
decimal_t::decimal_t(const std::string& s) : value(parse_decimal_t(std::string_view(s))) {}
decimal_t::decimal_t(std::string_view s) : value(parse_decimal_t(s)) {}



// -- to_string --

std::string
decimal_t::to_string() const
{
    if (value == 0)
        return "0.0";

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

    // Always include decimal point.
    if (frac_part == 0) {
        std::string result;
        if (negative) result.push_back('-');
        result += int_str;
        result += ".0";
        return result;
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

// -- to_double / to_long_double --

std::pair<double, bool>
decimal_t::to_double() const
{
    bool negative = value < 0;
    int128_t abs_val = negative ? -value : value;
    int128_t int_part = abs_val / scale_factor;
    int128_t frac_part = abs_val % scale_factor;

    double d = static_cast<double>(int_part)
             + static_cast<double>(frac_part) / static_cast<double>(scale_factor);
    if (negative) d = -d;

    // Round-trip: check if the raw int128 survives double conversion.
    auto uval = static_cast<uint128_t>(abs_val);
    bool fits = (static_cast<uint128_t>(static_cast<double>(uval)) == uval);

    return {d, fits};
}

std::pair<long double, bool>
decimal_t::to_long_double() const
{
    bool negative = value < 0;
    int128_t abs_val = negative ? -value : value;
    int128_t int_part = abs_val / scale_factor;
    int128_t frac_part = abs_val % scale_factor;

    long double ld = static_cast<long double>(int_part)
                   + static_cast<long double>(frac_part) / static_cast<long double>(scale_factor);
    if (negative) ld = -ld;

    auto uval = static_cast<uint128_t>(abs_val);
    bool fits = (static_cast<uint128_t>(static_cast<long double>(uval)) == uval);

    return {ld, fits};
}

// -- operator* / operator*= (safe, via 256-bit) --

decimal_t decimal_t::operator*(const decimal_t& rhs) const {
    auto r = mul(*this, rhs);
    if (r.has_overflow())
        throw std::overflow_error("decimal: multiplication overflow");
    return r.value;
}

decimal_t& decimal_t::operator*=(const decimal_t& rhs) {
    return *this = *this * rhs;
}

// -- Scientific notation for int128 --

namespace {

std::string int128_to_scientific(int128_t v) {
    if (v == 0) return "0.0e+0";

    bool negative = v < 0;
    uint128_t abs_v = to_unsigned(v);

    // Convert to decimal digits (reversed).
    char digits[40];
    int n = 0;
    while (abs_v > 0) {
        digits[n++] = static_cast<char>('0' + static_cast<int>(abs_v % 10));
        abs_v /= 10;
    }

    std::string result;
    if (negative) result += '-';
    result += digits[n - 1]; // most significant digit
    result += '.';
    if (n > 1) {
        for (int i = n - 2; i >= 0; --i)
            result += digits[i];
        // Trim trailing zeros, keep at least one digit after '.'.
        auto last = result.find_last_not_of('0');
        if (result[last] == '.') ++last;
        result.resize(last + 1);
    } else {
        result += '0';
    }
    result += "e+";
    result += std::to_string(n - 1);

    return result;
}

} // anonymous namespace

// -- decimal_error_t --

std::string decimal_error_t::to_string() const { return int128_to_scientific(value); }

// -- decimal_overflow_t --

std::string decimal_overflow_t::to_string() const { return int128_to_scientific(value); }

// -- operator<< --

std::ostream&
operator<<(std::ostream& os, const decimal_t& d)
{
    return os << d.to_string();
}

std::ostream&
operator<<(std::ostream& os, const decimal_error_t& e)
{
    return os << e.to_string();
}

std::ostream&
operator<<(std::ostream& os, const decimal_overflow_t& o)
{
    return os << o.to_string();
}

// -- Safe mul / div --

decimal_result_t mul(decimal_t a, decimal_t b) {
    bool negative = (a.value < 0) != (b.value < 0);
    uint128_t abs_a = to_unsigned(a.value);
    uint128_t abs_b = to_unsigned(b.value);

    // Full 256-bit product, then divide by scale_factor to restore scale 10^-18.
    uint256_t product = mul256(abs_a, abs_b);
    auto [quot, rem]  = divmod256(product, static_cast<uint128_t>(decimal_t::scale_factor));

    return pack_result(quot, rem, negative);
}

decimal_result_t div(decimal_t a, decimal_t b) {
    if (b.value == 0)
        throw std::domain_error("decimal: division by zero");

    bool negative = (a.value < 0) != (b.value < 0);
    uint128_t abs_a = to_unsigned(a.value);
    uint128_t abs_b = to_unsigned(b.value);

    // Multiply numerator by scale_factor first (256-bit), then divide by b.
    uint256_t numerator = mul256(abs_a, static_cast<uint128_t>(decimal_t::scale_factor));
    auto [quot, rem]    = divmod256(numerator, abs_b);

    return pack_result(quot, rem, negative);
}

} // namespace binapi2::fapi::types
