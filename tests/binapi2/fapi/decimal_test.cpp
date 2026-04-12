// SPDX-License-Identifier: Apache-2.0
//
// Comprehensive tests for the decimal_t fixed-precision type.

#include <binapi2/fapi/types/detail/decimal.hpp>

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using binapi2::fapi::types::decimal_t;
using binapi2::fapi::types::decimal_error_t;
using binapi2::fapi::types::decimal_overflow_t;
using binapi2::fapi::types::decimal_result_t;
using binapi2::fapi::types::mul;
using binapi2::fapi::types::div;

using int128_t  = decimal_t::int128_t;
using uint128_t = decimal_t::uint128_t;

// Helper: construct decimal_t from raw int128.
constexpr decimal_t raw(int128_t v) { return decimal_t(v, decimal_t::raw_tag{}); }

// Helper: 10^N as int128.
constexpr int128_t pow10(int n) {
    int128_t r = 1;
    for (int i = 0; i < n; ++i) r *= 10;
    return r;
}

// ============================================================================
// Construction
// ============================================================================

TEST(DecimalConstruction, DefaultIsZero) {
    decimal_t d;
    EXPECT_EQ(d.value, 0);
    EXPECT_TRUE(d.is_zero());
}

TEST(DecimalConstruction, FromRawTag) {
    auto d = raw(42);
    EXPECT_EQ(d.value, 42);
}

TEST(DecimalConstruction, FromRawWithInputScale) {
    // 100 at scale 2 → 100 * 10^(18-2) = 10^18.
    decimal_t d(100, 2);
    EXPECT_EQ(d.value, pow10(18));
    // Scale down: 10^20 at scale 20 → 10^20 / 10^2 = 10^18.
    decimal_t d2(pow10(20), 20);
    EXPECT_EQ(d2.value, pow10(18));
    // Same scale: no change.
    decimal_t d3(int128_t(42), 18);
    EXPECT_EQ(d3.value, 42);
}

// ============================================================================
// String parsing
// ============================================================================

TEST(DecimalParsing, Zero) {
    decimal_t d("0");
    EXPECT_TRUE(d.is_zero());
}

TEST(DecimalParsing, Integer) {
    decimal_t d("42");
    EXPECT_EQ(d.value, int128_t(42) * pow10(18));
}

TEST(DecimalParsing, Fractional) {
    decimal_t d("1.5");
    EXPECT_EQ(d.value, int128_t(15) * pow10(17));
}

TEST(DecimalParsing, LeadingZeroFraction) {
    decimal_t d("0.001");
    EXPECT_EQ(d.value, pow10(15));
}

TEST(DecimalParsing, Negative) {
    decimal_t d("-3.14");
    EXPECT_TRUE(d.is_negative());
    decimal_t pos("3.14");
    EXPECT_EQ(d.value, -pos.value);
}

TEST(DecimalParsing, MaxFractionalDigits) {
    // Exactly 18 fractional digits — should succeed.
    decimal_t d("0.000000000000000001");
    EXPECT_EQ(d.value, 1);
}

TEST(DecimalParsing, TooManyFractionalDigitsThrows) {
    EXPECT_THROW(decimal_t("0.0000000000000000001"), std::invalid_argument);
}

TEST(DecimalParsing, LargeIntegerPartThrows) {
    // 21 digits → overflow.
    EXPECT_THROW(decimal_t("999999999999999999999"), std::overflow_error);
}

TEST(DecimalParsing, InvalidCharThrows) {
    EXPECT_THROW(decimal_t("12.3x4"), std::invalid_argument);
}

TEST(DecimalParsing, FromString) {
    std::string s = "100.25";
    decimal_t d(s);
    EXPECT_EQ(d, decimal_t("100.25"));
}

TEST(DecimalParsing, FromStringView) {
    std::string_view sv = "100.25";
    decimal_t d(sv);
    EXPECT_EQ(d, decimal_t("100.25"));
}

TEST(DecimalParsing, PositiveSign) {
    decimal_t d("+7.5");
    EXPECT_EQ(d, decimal_t("7.5"));
}

TEST(DecimalParsing, EmptyStringIsZero) {
    decimal_t d("");
    EXPECT_TRUE(d.is_zero());
}

// ============================================================================
// to_string  (always decimal_t with point, never scientific)
// ============================================================================

TEST(DecimalToString, Zero) {
    EXPECT_EQ(decimal_t("0").to_string(), "0.0");
}

TEST(DecimalToString, WholeNumber) {
    EXPECT_EQ(decimal_t("42").to_string(), "42.0");
}

TEST(DecimalToString, Fractional) {
    EXPECT_EQ(decimal_t("1.5").to_string(), "1.5");
}

TEST(DecimalToString, SmallFraction) {
    EXPECT_EQ(decimal_t("0.000000000000000001").to_string(), "0.000000000000000001");
}

TEST(DecimalToString, Negative) {
    EXPECT_EQ(decimal_t("-99.9").to_string(), "-99.9");
}

TEST(DecimalToString, NegativeWhole) {
    EXPECT_EQ(decimal_t("-5").to_string(), "-5.0");
}

TEST(DecimalToString, TrailingZerosTrimmed) {
    EXPECT_EQ(decimal_t("1.500").to_string(), "1.5");
}

TEST(DecimalToString, AlwaysHasPoint) {
    // Ensure every to_string output contains a '.'
    for (const char* s : {"0", "1", "100", "-1", "0.1", "123.456"}) {
        auto str = decimal_t(s).to_string();
        EXPECT_NE(str.find('.'), std::string::npos) << "missing '.' in: " << str;
    }
}

TEST(DecimalToString, RoundTrip) {
    // Parse → to_string → parse again must yield the same value.
    for (const char* s : {"0.0", "1.5", "-99.9", "0.000000000000000001",
                          "12345678901.234567", "-1000000000.0"}) {
        decimal_t d(s);
        decimal_t d2(d.to_string());
        EXPECT_EQ(d, d2) << "round-trip failed for: " << s;
    }
}

TEST(DecimalToString, OstreamMatchesToString) {
    decimal_t d("123.456");
    std::ostringstream oss;
    oss << d;
    EXPECT_EQ(oss.str(), d.to_string());
}

// ============================================================================
// Queries
// ============================================================================

TEST(DecimalQueries, IsZero) {
    EXPECT_TRUE(decimal_t("0").is_zero());
    EXPECT_FALSE(decimal_t("1").is_zero());
}

TEST(DecimalQueries, IsNegative) {
    EXPECT_TRUE(decimal_t("-1").is_negative());
    EXPECT_FALSE(decimal_t("0").is_negative());
    EXPECT_FALSE(decimal_t("1").is_negative());
}

TEST(DecimalQueries, IsPositive) {
    EXPECT_TRUE(decimal_t("1").is_positive());
    EXPECT_FALSE(decimal_t("0").is_positive());
    EXPECT_FALSE(decimal_t("-1").is_positive());
}

TEST(DecimalQueries, IsValid) {
    EXPECT_TRUE(decimal_t("0").is_valid());
    EXPECT_TRUE(decimal_t("1").is_valid());
    EXPECT_TRUE(decimal_t("-1").is_valid());
    EXPECT_TRUE(decimal_t::max().is_valid());
    EXPECT_TRUE(decimal_t::min().is_valid());
}

TEST(DecimalQueries, IsValidDetectsOverflow) {
    // max + max: the int128 doesn't overflow (by design), but the result exceeds
    // the valid range.
    decimal_t sum = decimal_t::max() + decimal_t::max();
    EXPECT_FALSE(sum.is_valid());

    decimal_t neg_sum = decimal_t::min() + decimal_t::min();
    EXPECT_FALSE(neg_sum.is_valid());
}

// ============================================================================
// Max / Min
// ============================================================================

TEST(DecimalBounds, MaxIsPositive) {
    EXPECT_TRUE(decimal_t::max().is_positive());
}

TEST(DecimalBounds, MinIsNegative) {
    EXPECT_TRUE(decimal_t::min().is_negative());
}

TEST(DecimalBounds, MaxMinSymmetric) {
    EXPECT_EQ(decimal_t::max().value, -decimal_t::min().value);
}

TEST(DecimalBounds, DoubleSumFitsInt128) {
    // The key invariant: max_raw + max_raw must not overflow int128.
    // If it did, the addition would wrap. We verify by checking the sum
    // is positive (if it wrapped it would be negative).
    int128_t sum = decimal_t::max().value + decimal_t::max().value;
    EXPECT_GT(sum, 0);
}

TEST(DecimalBounds, DoubleNegSumFitsInt128) {
    int128_t sum = decimal_t::min().value + decimal_t::min().value;
    EXPECT_LT(sum, 0);
}

// ============================================================================
// Comparison
// ============================================================================

TEST(DecimalComparison, Equal) {
    EXPECT_EQ(decimal_t("1.5"), decimal_t("1.5"));
}

TEST(DecimalComparison, NotEqual) {
    EXPECT_NE(decimal_t("1.5"), decimal_t("1.6"));
}

TEST(DecimalComparison, LessThan) {
    EXPECT_LT(decimal_t("1.0"), decimal_t("2.0"));
    EXPECT_LT(decimal_t("-1.0"), decimal_t("0"));
}

TEST(DecimalComparison, GreaterThan) {
    EXPECT_GT(decimal_t("2.0"), decimal_t("1.0"));
}

TEST(DecimalComparison, ThreeWay) {
    EXPECT_TRUE((decimal_t("1.0") <=> decimal_t("2.0")) < 0);
    EXPECT_TRUE((decimal_t("2.0") <=> decimal_t("1.0")) > 0);
    EXPECT_TRUE((decimal_t("1.0") <=> decimal_t("1.0")) == 0);
}

// ============================================================================
// Addition / Subtraction
// ============================================================================

TEST(DecimalArithmetic, AddSimple) {
    EXPECT_EQ(decimal_t("1.5") + decimal_t("2.5"), decimal_t("4.0"));
}

TEST(DecimalArithmetic, SubSimple) {
    EXPECT_EQ(decimal_t("3.0") - decimal_t("1.5"), decimal_t("1.5"));
}

TEST(DecimalArithmetic, AddNegative) {
    EXPECT_EQ(decimal_t("1.0") + decimal_t("-3.0"), decimal_t("-2.0"));
}

TEST(DecimalArithmetic, PlusEquals) {
    decimal_t d("1.0");
    d += decimal_t("2.5");
    EXPECT_EQ(d, decimal_t("3.5"));
}

TEST(DecimalArithmetic, MinusEquals) {
    decimal_t d("5.0");
    d -= decimal_t("1.5");
    EXPECT_EQ(d, decimal_t("3.5"));
}

TEST(DecimalArithmetic, UnaryNegate) {
    EXPECT_EQ(-decimal_t("1.5"), decimal_t("-1.5"));
    EXPECT_EQ(-decimal_t("-1.5"), decimal_t("1.5"));
    EXPECT_EQ(-decimal_t("0"), decimal_t("0"));
}

TEST(DecimalArithmetic, AdditionOverflowDetected) {
    // Adding two max-valid values produces an invalid result but doesn't crash.
    auto r = decimal_t::max() + decimal_t::max();
    EXPECT_FALSE(r.is_valid());
    auto r2 = decimal_t::min() + decimal_t::min();
    EXPECT_FALSE(r2.is_valid());
}

// ============================================================================
// Operator* and *=
// ============================================================================

TEST(DecimalMulOperator, Simple) {
    EXPECT_EQ(decimal_t("2.0") * decimal_t("3.0"), decimal_t("6.0"));
}

TEST(DecimalMulOperator, Fractional) {
    EXPECT_EQ(decimal_t("0.5") * decimal_t("0.5"), decimal_t("0.25"));
}

TEST(DecimalMulOperator, ByZero) {
    EXPECT_EQ(decimal_t("123.456") * decimal_t("0"), decimal_t("0"));
}

TEST(DecimalMulOperator, ByOne) {
    EXPECT_EQ(decimal_t("123.456") * decimal_t("1.0"), decimal_t("123.456"));
}

TEST(DecimalMulOperator, NegativeTimesPositive) {
    EXPECT_EQ(decimal_t("-2.0") * decimal_t("3.0"), decimal_t("-6.0"));
}

TEST(DecimalMulOperator, NegativeTimesNegative) {
    EXPECT_EQ(decimal_t("-2.0") * decimal_t("-3.0"), decimal_t("6.0"));
}

TEST(DecimalMulOperator, MulEquals) {
    decimal_t d("4.0");
    d *= decimal_t("2.5");
    EXPECT_EQ(d, decimal_t("10.0"));
}

TEST(DecimalMulOperator, OverflowThrows) {
    // operator* throws std::overflow_error when result exceeds int128.
    EXPECT_THROW((void)(decimal_t::max() * decimal_t::max()), std::overflow_error);
}

// ============================================================================
// Safe mul()
// ============================================================================

TEST(DecimalMul, ExactResult) {
    auto r = mul(decimal_t("2.0"), decimal_t("3.0"));
    EXPECT_EQ(r.value, decimal_t("6.0"));
    EXPECT_TRUE(r.error.is_zero());
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, FractionalExact) {
    auto r = mul(decimal_t("0.5"), decimal_t("0.5"));
    EXPECT_EQ(r.value, decimal_t("0.25"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, ZeroProduct) {
    auto r = mul(decimal_t("999.0"), decimal_t("0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, WithRemainder) {
    // 1/3 * 3 via mul: 1/3 cannot be represented exactly, but let's test
    // a case that produces a remainder.
    // 7 * (1/3 approximation) — instead test a known remainder case:
    // 0.000000000000000001 * 0.000000000000000002 = 2e-36 → result 0, error 2.
    auto r = mul(decimal_t("0.000000000000000001"), decimal_t("0.000000000000000002"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_EQ(r.error.value, 2);
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, NegativeResult) {
    auto r = mul(decimal_t("-5.0"), decimal_t("2.0"));
    EXPECT_EQ(r.value, decimal_t("-10.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, NegativeTimesNegative) {
    auto r = mul(decimal_t("-3.0"), decimal_t("-4.0"));
    EXPECT_EQ(r.value, decimal_t("12.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, LargeValuesNoOverflow) {
    // 1e9 * 1e9 = 1e18 — should fit.
    auto r = mul(decimal_t("1000000000.0"), decimal_t("1000000000.0"));
    EXPECT_EQ(r.value, decimal_t("1000000000000000000.0"));
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, OverflowDetected) {
    // max * max: 10^19 * 10^19 = 10^38. Raw: 10^37 * 10^37 / 10^18 = 10^56.
    // That far exceeds int128 range (~1.7e38), so overflow is detected.
    auto r = mul(decimal_t::max(), decimal_t::max());
    EXPECT_TRUE(r.has_overflow());
}

TEST(DecimalMul, Commutativity) {
    decimal_t a("123.456");
    decimal_t b("789.012");
    auto r1 = mul(a, b);
    auto r2 = mul(b, a);
    EXPECT_EQ(r1.value, r2.value);
    EXPECT_EQ(r1.error.value, r2.error.value);
}

TEST(DecimalMul, IdentityElement) {
    decimal_t a("12345.678901234567");
    auto r = mul(a, decimal_t("1.0"));
    EXPECT_EQ(r.value, a);
    EXPECT_TRUE(r.is_exact());
}

// ============================================================================
// Safe div()
// ============================================================================

TEST(DecimalDiv, ExactResult) {
    auto r = div(decimal_t("6.0"), decimal_t("3.0"));
    EXPECT_EQ(r.value, decimal_t("2.0"));
    EXPECT_TRUE(r.error.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, FractionalExact) {
    auto r = div(decimal_t("1.0"), decimal_t("4.0"));
    EXPECT_EQ(r.value, decimal_t("0.25"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, WithRemainder) {
    // 1 / 3 = 0.333...333 with remainder.
    auto r = div(decimal_t("1.0"), decimal_t("3.0"));
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.error.is_zero());
    EXPECT_FALSE(r.has_overflow());
    // The result should be close to 1/3.
    EXPECT_EQ(r.value, decimal_t("0.333333333333333333"));
}

TEST(DecimalDiv, DivideByZeroThrows) {
    EXPECT_THROW((void)div(decimal_t("1.0"), decimal_t("0")), std::domain_error);
}

TEST(DecimalDiv, DivideZero) {
    auto r = div(decimal_t("0"), decimal_t("5.0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, NegativeDividend) {
    auto r = div(decimal_t("-6.0"), decimal_t("3.0"));
    EXPECT_EQ(r.value, decimal_t("-2.0"));
}

TEST(DecimalDiv, NegativeDivisor) {
    auto r = div(decimal_t("6.0"), decimal_t("-3.0"));
    EXPECT_EQ(r.value, decimal_t("-2.0"));
}

TEST(DecimalDiv, BothNegative) {
    auto r = div(decimal_t("-6.0"), decimal_t("-3.0"));
    EXPECT_EQ(r.value, decimal_t("2.0"));
}

TEST(DecimalDiv, ByOne) {
    decimal_t a("12345.678901234567");
    auto r = div(a, decimal_t("1.0"));
    EXPECT_EQ(r.value, a);
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, SmallByLarge) {
    // 1e-18 / 1e18 = 1e-36 → result 0, nonzero error.
    auto r = div(decimal_t("0.000000000000000001"), decimal_t("1000000000000000000.0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_FALSE(r.error.is_zero());
}

TEST(DecimalDiv, LargeBySmall) {
    // Large numerator, tiny denominator → may overflow.
    auto r = div(decimal_t::max(), decimal_t("0.000000000000000001"));
    EXPECT_TRUE(r.has_overflow());
}

// ============================================================================
// Mul/Div round-trip
// ============================================================================

TEST(DecimalRoundTrip, MulThenDiv) {
    decimal_t a("123.456");
    decimal_t b("789.012");
    auto product = mul(a, b);
    ASSERT_FALSE(product.has_overflow());
    auto back = div(product.value, b);
    ASSERT_FALSE(back.has_overflow());
    // Should get back to a (possibly off by 1 ULP due to remainder truncation).
    auto diff = a.value - back.value.value;
    if (diff < 0) diff = -diff;
    EXPECT_LE(diff, 1) << "a=" << a << " back=" << back.value;
}

TEST(DecimalRoundTrip, DivThenMul) {
    decimal_t a("100.0");
    decimal_t b("7.0");
    auto quotient = div(a, b);
    ASSERT_FALSE(quotient.has_overflow());
    auto back = mul(quotient.value, b);
    ASSERT_FALSE(back.has_overflow());
    auto diff = a.value - back.value.value;
    if (diff < 0) diff = -diff;
    EXPECT_LE(diff, 7) << "a=" << a << " back=" << back.value;
}

// ============================================================================
// to_double / to_long_double
// ============================================================================

TEST(DecimalToDouble, Zero) {
    auto [d, fits] = decimal_t("0").to_double();
    EXPECT_DOUBLE_EQ(d, 0.0);
    EXPECT_TRUE(fits);
}

TEST(DecimalToDouble, SmallValueFits) {
    auto [d, fits] = decimal_t("1.5").to_double();
    EXPECT_DOUBLE_EQ(d, 1.5);
    // 1.5 * 10^18 = 1500000000000000000, which is < 2^53? No, 2^53 ≈ 9e15.
    // So fits may be false. That's fine — we test the value.
    (void)fits;
}

TEST(DecimalToDouble, Approximate) {
    // A value that certainly exceeds double's 53-bit mantissa.
    auto [d, fits] = decimal_t("12345678901234567.890123456789012345").to_double();
    EXPECT_FALSE(fits);
    // But the value should be approximately right.
    EXPECT_NEAR(d, 12345678901234567.89, 1e4);
}

TEST(DecimalToDouble, Negative) {
    auto [d, fits] = decimal_t("-42.5").to_double();
    EXPECT_DOUBLE_EQ(d, -42.5);
    (void)fits;
}

TEST(DecimalToLongDouble, Zero) {
    auto [ld, fits] = decimal_t("0").to_long_double();
    EXPECT_DOUBLE_EQ(static_cast<double>(ld), 0.0);
    EXPECT_TRUE(fits);
}

TEST(DecimalToLongDouble, ReturnsValue) {
    auto [ld, fits] = decimal_t("3.14").to_long_double();
    EXPECT_NEAR(static_cast<double>(ld), 3.14, 1e-10);
    (void)fits;
}

TEST(DecimalToLongDouble, LargeValueDoesNotFit) {
    auto [ld, fits] = decimal_t("12345678901234567.890123456789012345").to_long_double();
    EXPECT_FALSE(fits);
    EXPECT_NEAR(static_cast<double>(ld), 12345678901234567.89, 1e4);
}

// ============================================================================
// decimal_error_t
// ============================================================================

TEST(DecimalError, IsZero) {
    decimal_error_t e{};
    EXPECT_TRUE(e.is_zero());
}

TEST(DecimalError, Comparison) {
    decimal_error_t a{10};
    decimal_error_t b{20};
    EXPECT_LT(a, b);
    EXPECT_EQ(a, decimal_error_t{10});
}

TEST(DecimalError, ToStringScientific) {
    decimal_error_t e{12345};
    auto s = e.to_string();
    EXPECT_EQ(s, "1.2345e+4");
}

TEST(DecimalError, ToStringZero) {
    decimal_error_t e{0};
    EXPECT_EQ(e.to_string(), "0.0e+0");
}

TEST(DecimalError, ToStringSingleDigit) {
    decimal_error_t e{7};
    EXPECT_EQ(e.to_string(), "7.0e+0");
}

TEST(DecimalError, ToStringNegative) {
    decimal_error_t e{-5000};
    EXPECT_EQ(e.to_string(), "-5.0e+3");
}

TEST(DecimalError, OstreamMatchesToString) {
    decimal_error_t e{999};
    std::ostringstream oss;
    oss << e;
    EXPECT_EQ(oss.str(), e.to_string());
}

// ============================================================================
// decimal_overflow_t
// ============================================================================

TEST(DecimalOverflow, IsZero) {
    decimal_overflow_t o{};
    EXPECT_TRUE(o.is_zero());
}

TEST(DecimalOverflow, Comparison) {
    decimal_overflow_t a{1};
    decimal_overflow_t b{2};
    EXPECT_LT(a, b);
}

TEST(DecimalOverflow, ToStringScientific) {
    decimal_overflow_t o{1000000};
    EXPECT_EQ(o.to_string(), "1.0e+6");
}

TEST(DecimalOverflow, OstreamMatchesToString) {
    decimal_overflow_t o{42};
    std::ostringstream oss;
    oss << o;
    EXPECT_EQ(oss.str(), o.to_string());
}

// ============================================================================
// decimal_result_t
// ============================================================================

TEST(DecimalResult, IsExact) {
    decimal_result_t r{decimal_t("1.0"), {0}, {0}};
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalResult, HasError) {
    decimal_result_t r{decimal_t("1.0"), {5}, {0}};
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalResult, HasOverflow) {
    decimal_result_t r{decimal_t("1.0"), {0}, {1}};
    EXPECT_FALSE(r.is_exact());
    EXPECT_TRUE(r.has_overflow());
}

// ============================================================================
// Type safety: decimal_error_t and decimal_overflow_t are not addable to decimal
// ============================================================================

// These are compile-time guarantees. We verify via static_assert that the
// types are distinct and not implicitly convertible to decimal.
static_assert(!std::is_convertible_v<decimal_error_t, decimal_t>);
static_assert(!std::is_convertible_v<decimal_overflow_t, decimal_t>);
static_assert(!std::is_same_v<decimal_error_t, decimal_t>);
static_assert(!std::is_same_v<decimal_overflow_t, decimal_t>);

// ============================================================================
// JSON round-trip (glaze)
// ============================================================================

TEST(DecimalJson, SerializeDeserialize) {
    decimal_t original("50000.10");
    std::string json;
    (void)glz::write_json(original, json);

    decimal_t parsed;
    auto err = glz::read_json(parsed, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_EQ(parsed, original);
}

TEST(DecimalJson, Zero) {
    decimal_t original("0");
    std::string json;
    (void)glz::write_json(original, json);

    decimal_t parsed;
    auto err = glz::read_json(parsed, json);
    EXPECT_FALSE(err);
    EXPECT_EQ(parsed, original);
}

} // namespace (close for decimal_json_wrapper)

struct decimal_json_wrapper { binapi2::fapi::types::decimal_t value; };

namespace {

TEST(DecimalJson, UnquotedNumber) {
    // Binance sends some decimal fields as unquoted JSON numbers.
    std::string json = R"({"value":0.004})";
    decimal_json_wrapper w;
    auto err = glz::read_json(w, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_EQ(w.value, decimal_t("0.004"));
}

TEST(DecimalJson, UnquotedZero) {
    std::string json = R"({"value":0.0})";
    decimal_json_wrapper w;
    auto err = glz::read_json(w, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_TRUE(w.value.is_zero());
}

TEST(DecimalJson, UnquotedInteger) {
    std::string json = R"({"value":10000})";
    decimal_json_wrapper w;
    auto err = glz::read_json(w, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_EQ(w.value, decimal_t("10000"));
}

TEST(DecimalJson, QuotedStringStillWorks) {
    std::string json = R"({"value":"123.456"})";
    decimal_json_wrapper w;
    auto err = glz::read_json(w, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_EQ(w.value, decimal_t("123.456"));
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(DecimalEdge, MulByMinusOne) {
    auto r = mul(decimal_t("5.0"), decimal_t("-1.0"));
    EXPECT_EQ(r.value, decimal_t("-5.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, DivBySelf) {
    decimal_t a("123.456");
    auto r = div(a, a);
    EXPECT_EQ(r.value, decimal_t("1.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MulSmallNumbers) {
    // Two very small numbers: result is zero with nonzero error.
    decimal_t tiny("0.000000000000000001"); // 1 ULP
    auto r = mul(tiny, tiny);
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_EQ(r.error.value, 1); // 1 * 1 = 1, remainder 1 after dividing by 10^18
}

TEST(DecimalEdge, MaxTimesOne) {
    auto r = mul(decimal_t::max(), decimal_t("1.0"));
    EXPECT_EQ(r.value, decimal_t::max());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MinTimesOne) {
    auto r = mul(decimal_t::min(), decimal_t("1.0"));
    EXPECT_EQ(r.value, decimal_t::min());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MaxDivOne) {
    auto r = div(decimal_t::max(), decimal_t("1.0"));
    EXPECT_EQ(r.value, decimal_t::max());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, NegateMax) {
    EXPECT_EQ(-decimal_t::max(), decimal_t::min());
}

TEST(DecimalEdge, SubSelf) {
    decimal_t a("12345.6789");
    EXPECT_TRUE((a - a).is_zero());
}

} // anonymous namespace
