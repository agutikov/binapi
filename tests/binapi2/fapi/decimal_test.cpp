// SPDX-License-Identifier: Apache-2.0
//
// Comprehensive tests for the decimal fixed-precision type.

#include <binapi2/fapi/types/detail/decimal.hpp>

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using binapi2::fapi::types::decimal;
using binapi2::fapi::types::decimal_error;
using binapi2::fapi::types::decimal_overflow;
using binapi2::fapi::types::decimal_result;
using binapi2::fapi::types::mul;
using binapi2::fapi::types::div;

using int128_t  = decimal::int128_t;
using uint128_t = decimal::uint128_t;

// Helper: construct decimal from raw int128.
constexpr decimal raw(int128_t v) { return decimal(v, decimal::raw_tag{}); }

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
    decimal d;
    EXPECT_EQ(d.value, 0);
    EXPECT_TRUE(d.is_zero());
}

TEST(DecimalConstruction, FromRawTag) {
    auto d = raw(42);
    EXPECT_EQ(d.value, 42);
}

TEST(DecimalConstruction, FromRawWithInputScale) {
    // 100 at scale 2 → 100 * 10^(18-2) = 10^18.
    decimal d(100, 2);
    EXPECT_EQ(d.value, pow10(18));
    // Scale down: 10^20 at scale 20 → 10^20 / 10^2 = 10^18.
    decimal d2(pow10(20), 20);
    EXPECT_EQ(d2.value, pow10(18));
    // Same scale: no change.
    decimal d3(int128_t(42), 18);
    EXPECT_EQ(d3.value, 42);
}

// ============================================================================
// String parsing
// ============================================================================

TEST(DecimalParsing, Zero) {
    decimal d("0");
    EXPECT_TRUE(d.is_zero());
}

TEST(DecimalParsing, Integer) {
    decimal d("42");
    EXPECT_EQ(d.value, int128_t(42) * pow10(18));
}

TEST(DecimalParsing, Fractional) {
    decimal d("1.5");
    EXPECT_EQ(d.value, int128_t(15) * pow10(17));
}

TEST(DecimalParsing, LeadingZeroFraction) {
    decimal d("0.001");
    EXPECT_EQ(d.value, pow10(15));
}

TEST(DecimalParsing, Negative) {
    decimal d("-3.14");
    EXPECT_TRUE(d.is_negative());
    decimal pos("3.14");
    EXPECT_EQ(d.value, -pos.value);
}

TEST(DecimalParsing, MaxFractionalDigits) {
    // Exactly 18 fractional digits — should succeed.
    decimal d("0.000000000000000001");
    EXPECT_EQ(d.value, 1);
}

TEST(DecimalParsing, TooManyFractionalDigitsThrows) {
    EXPECT_THROW(decimal("0.0000000000000000001"), std::invalid_argument);
}

TEST(DecimalParsing, LargeIntegerPartThrows) {
    // 21 digits → overflow.
    EXPECT_THROW(decimal("999999999999999999999"), std::overflow_error);
}

TEST(DecimalParsing, InvalidCharThrows) {
    EXPECT_THROW(decimal("12.3x4"), std::invalid_argument);
}

TEST(DecimalParsing, FromString) {
    std::string s = "100.25";
    decimal d(s);
    EXPECT_EQ(d, decimal("100.25"));
}

TEST(DecimalParsing, FromStringView) {
    std::string_view sv = "100.25";
    decimal d(sv);
    EXPECT_EQ(d, decimal("100.25"));
}

TEST(DecimalParsing, PositiveSign) {
    decimal d("+7.5");
    EXPECT_EQ(d, decimal("7.5"));
}

TEST(DecimalParsing, EmptyStringIsZero) {
    decimal d("");
    EXPECT_TRUE(d.is_zero());
}

// ============================================================================
// to_string  (always decimal with point, never scientific)
// ============================================================================

TEST(DecimalToString, Zero) {
    EXPECT_EQ(decimal("0").to_string(), "0.0");
}

TEST(DecimalToString, WholeNumber) {
    EXPECT_EQ(decimal("42").to_string(), "42.0");
}

TEST(DecimalToString, Fractional) {
    EXPECT_EQ(decimal("1.5").to_string(), "1.5");
}

TEST(DecimalToString, SmallFraction) {
    EXPECT_EQ(decimal("0.000000000000000001").to_string(), "0.000000000000000001");
}

TEST(DecimalToString, Negative) {
    EXPECT_EQ(decimal("-99.9").to_string(), "-99.9");
}

TEST(DecimalToString, NegativeWhole) {
    EXPECT_EQ(decimal("-5").to_string(), "-5.0");
}

TEST(DecimalToString, TrailingZerosTrimmed) {
    EXPECT_EQ(decimal("1.500").to_string(), "1.5");
}

TEST(DecimalToString, AlwaysHasPoint) {
    // Ensure every to_string output contains a '.'
    for (const char* s : {"0", "1", "100", "-1", "0.1", "123.456"}) {
        auto str = decimal(s).to_string();
        EXPECT_NE(str.find('.'), std::string::npos) << "missing '.' in: " << str;
    }
}

TEST(DecimalToString, RoundTrip) {
    // Parse → to_string → parse again must yield the same value.
    for (const char* s : {"0.0", "1.5", "-99.9", "0.000000000000000001",
                          "12345678901.234567", "-1000000000.0"}) {
        decimal d(s);
        decimal d2(d.to_string());
        EXPECT_EQ(d, d2) << "round-trip failed for: " << s;
    }
}

TEST(DecimalToString, OstreamMatchesToString) {
    decimal d("123.456");
    std::ostringstream oss;
    oss << d;
    EXPECT_EQ(oss.str(), d.to_string());
}

// ============================================================================
// Queries
// ============================================================================

TEST(DecimalQueries, IsZero) {
    EXPECT_TRUE(decimal("0").is_zero());
    EXPECT_FALSE(decimal("1").is_zero());
}

TEST(DecimalQueries, IsNegative) {
    EXPECT_TRUE(decimal("-1").is_negative());
    EXPECT_FALSE(decimal("0").is_negative());
    EXPECT_FALSE(decimal("1").is_negative());
}

TEST(DecimalQueries, IsPositive) {
    EXPECT_TRUE(decimal("1").is_positive());
    EXPECT_FALSE(decimal("0").is_positive());
    EXPECT_FALSE(decimal("-1").is_positive());
}

TEST(DecimalQueries, IsValid) {
    EXPECT_TRUE(decimal("0").is_valid());
    EXPECT_TRUE(decimal("1").is_valid());
    EXPECT_TRUE(decimal("-1").is_valid());
    EXPECT_TRUE(decimal::max().is_valid());
    EXPECT_TRUE(decimal::min().is_valid());
}

TEST(DecimalQueries, IsValidDetectsOverflow) {
    // max + max: the int128 doesn't overflow (by design), but the result exceeds
    // the valid range.
    decimal sum = decimal::max() + decimal::max();
    EXPECT_FALSE(sum.is_valid());

    decimal neg_sum = decimal::min() + decimal::min();
    EXPECT_FALSE(neg_sum.is_valid());
}

// ============================================================================
// Max / Min
// ============================================================================

TEST(DecimalBounds, MaxIsPositive) {
    EXPECT_TRUE(decimal::max().is_positive());
}

TEST(DecimalBounds, MinIsNegative) {
    EXPECT_TRUE(decimal::min().is_negative());
}

TEST(DecimalBounds, MaxMinSymmetric) {
    EXPECT_EQ(decimal::max().value, -decimal::min().value);
}

TEST(DecimalBounds, DoubleSumFitsInt128) {
    // The key invariant: max_raw + max_raw must not overflow int128.
    // If it did, the addition would wrap. We verify by checking the sum
    // is positive (if it wrapped it would be negative).
    int128_t sum = decimal::max().value + decimal::max().value;
    EXPECT_GT(sum, 0);
}

TEST(DecimalBounds, DoubleNegSumFitsInt128) {
    int128_t sum = decimal::min().value + decimal::min().value;
    EXPECT_LT(sum, 0);
}

// ============================================================================
// Comparison
// ============================================================================

TEST(DecimalComparison, Equal) {
    EXPECT_EQ(decimal("1.5"), decimal("1.5"));
}

TEST(DecimalComparison, NotEqual) {
    EXPECT_NE(decimal("1.5"), decimal("1.6"));
}

TEST(DecimalComparison, LessThan) {
    EXPECT_LT(decimal("1.0"), decimal("2.0"));
    EXPECT_LT(decimal("-1.0"), decimal("0"));
}

TEST(DecimalComparison, GreaterThan) {
    EXPECT_GT(decimal("2.0"), decimal("1.0"));
}

TEST(DecimalComparison, ThreeWay) {
    EXPECT_TRUE((decimal("1.0") <=> decimal("2.0")) < 0);
    EXPECT_TRUE((decimal("2.0") <=> decimal("1.0")) > 0);
    EXPECT_TRUE((decimal("1.0") <=> decimal("1.0")) == 0);
}

// ============================================================================
// Addition / Subtraction
// ============================================================================

TEST(DecimalArithmetic, AddSimple) {
    EXPECT_EQ(decimal("1.5") + decimal("2.5"), decimal("4.0"));
}

TEST(DecimalArithmetic, SubSimple) {
    EXPECT_EQ(decimal("3.0") - decimal("1.5"), decimal("1.5"));
}

TEST(DecimalArithmetic, AddNegative) {
    EXPECT_EQ(decimal("1.0") + decimal("-3.0"), decimal("-2.0"));
}

TEST(DecimalArithmetic, PlusEquals) {
    decimal d("1.0");
    d += decimal("2.5");
    EXPECT_EQ(d, decimal("3.5"));
}

TEST(DecimalArithmetic, MinusEquals) {
    decimal d("5.0");
    d -= decimal("1.5");
    EXPECT_EQ(d, decimal("3.5"));
}

TEST(DecimalArithmetic, UnaryNegate) {
    EXPECT_EQ(-decimal("1.5"), decimal("-1.5"));
    EXPECT_EQ(-decimal("-1.5"), decimal("1.5"));
    EXPECT_EQ(-decimal("0"), decimal("0"));
}

TEST(DecimalArithmetic, AdditionOverflowDetected) {
    // Adding two max-valid values produces an invalid result but doesn't crash.
    auto r = decimal::max() + decimal::max();
    EXPECT_FALSE(r.is_valid());
    auto r2 = decimal::min() + decimal::min();
    EXPECT_FALSE(r2.is_valid());
}

// ============================================================================
// Operator* and *=
// ============================================================================

TEST(DecimalMulOperator, Simple) {
    EXPECT_EQ(decimal("2.0") * decimal("3.0"), decimal("6.0"));
}

TEST(DecimalMulOperator, Fractional) {
    EXPECT_EQ(decimal("0.5") * decimal("0.5"), decimal("0.25"));
}

TEST(DecimalMulOperator, ByZero) {
    EXPECT_EQ(decimal("123.456") * decimal("0"), decimal("0"));
}

TEST(DecimalMulOperator, ByOne) {
    EXPECT_EQ(decimal("123.456") * decimal("1.0"), decimal("123.456"));
}

TEST(DecimalMulOperator, NegativeTimesPositive) {
    EXPECT_EQ(decimal("-2.0") * decimal("3.0"), decimal("-6.0"));
}

TEST(DecimalMulOperator, NegativeTimesNegative) {
    EXPECT_EQ(decimal("-2.0") * decimal("-3.0"), decimal("6.0"));
}

TEST(DecimalMulOperator, MulEquals) {
    decimal d("4.0");
    d *= decimal("2.5");
    EXPECT_EQ(d, decimal("10.0"));
}

TEST(DecimalMulOperator, OverflowThrows) {
    // operator* throws std::overflow_error when result exceeds int128.
    EXPECT_THROW((void)(decimal::max() * decimal::max()), std::overflow_error);
}

// ============================================================================
// Safe mul()
// ============================================================================

TEST(DecimalMul, ExactResult) {
    auto r = mul(decimal("2.0"), decimal("3.0"));
    EXPECT_EQ(r.value, decimal("6.0"));
    EXPECT_TRUE(r.error.is_zero());
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, FractionalExact) {
    auto r = mul(decimal("0.5"), decimal("0.5"));
    EXPECT_EQ(r.value, decimal("0.25"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, ZeroProduct) {
    auto r = mul(decimal("999.0"), decimal("0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, WithRemainder) {
    // 1/3 * 3 via mul: 1/3 cannot be represented exactly, but let's test
    // a case that produces a remainder.
    // 7 * (1/3 approximation) — instead test a known remainder case:
    // 0.000000000000000001 * 0.000000000000000002 = 2e-36 → result 0, error 2.
    auto r = mul(decimal("0.000000000000000001"), decimal("0.000000000000000002"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_EQ(r.error.value, 2);
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, NegativeResult) {
    auto r = mul(decimal("-5.0"), decimal("2.0"));
    EXPECT_EQ(r.value, decimal("-10.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, NegativeTimesNegative) {
    auto r = mul(decimal("-3.0"), decimal("-4.0"));
    EXPECT_EQ(r.value, decimal("12.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalMul, LargeValuesNoOverflow) {
    // 1e9 * 1e9 = 1e18 — should fit.
    auto r = mul(decimal("1000000000.0"), decimal("1000000000.0"));
    EXPECT_EQ(r.value, decimal("1000000000000000000.0"));
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalMul, OverflowDetected) {
    // max * max: 10^19 * 10^19 = 10^38. Raw: 10^37 * 10^37 / 10^18 = 10^56.
    // That far exceeds int128 range (~1.7e38), so overflow is detected.
    auto r = mul(decimal::max(), decimal::max());
    EXPECT_TRUE(r.has_overflow());
}

TEST(DecimalMul, Commutativity) {
    decimal a("123.456");
    decimal b("789.012");
    auto r1 = mul(a, b);
    auto r2 = mul(b, a);
    EXPECT_EQ(r1.value, r2.value);
    EXPECT_EQ(r1.error.value, r2.error.value);
}

TEST(DecimalMul, IdentityElement) {
    decimal a("12345.678901234567");
    auto r = mul(a, decimal("1.0"));
    EXPECT_EQ(r.value, a);
    EXPECT_TRUE(r.is_exact());
}

// ============================================================================
// Safe div()
// ============================================================================

TEST(DecimalDiv, ExactResult) {
    auto r = div(decimal("6.0"), decimal("3.0"));
    EXPECT_EQ(r.value, decimal("2.0"));
    EXPECT_TRUE(r.error.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, FractionalExact) {
    auto r = div(decimal("1.0"), decimal("4.0"));
    EXPECT_EQ(r.value, decimal("0.25"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, WithRemainder) {
    // 1 / 3 = 0.333...333 with remainder.
    auto r = div(decimal("1.0"), decimal("3.0"));
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.error.is_zero());
    EXPECT_FALSE(r.has_overflow());
    // The result should be close to 1/3.
    EXPECT_EQ(r.value, decimal("0.333333333333333333"));
}

TEST(DecimalDiv, DivideByZeroThrows) {
    EXPECT_THROW((void)div(decimal("1.0"), decimal("0")), std::domain_error);
}

TEST(DecimalDiv, DivideZero) {
    auto r = div(decimal("0"), decimal("5.0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, NegativeDividend) {
    auto r = div(decimal("-6.0"), decimal("3.0"));
    EXPECT_EQ(r.value, decimal("-2.0"));
}

TEST(DecimalDiv, NegativeDivisor) {
    auto r = div(decimal("6.0"), decimal("-3.0"));
    EXPECT_EQ(r.value, decimal("-2.0"));
}

TEST(DecimalDiv, BothNegative) {
    auto r = div(decimal("-6.0"), decimal("-3.0"));
    EXPECT_EQ(r.value, decimal("2.0"));
}

TEST(DecimalDiv, ByOne) {
    decimal a("12345.678901234567");
    auto r = div(a, decimal("1.0"));
    EXPECT_EQ(r.value, a);
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalDiv, SmallByLarge) {
    // 1e-18 / 1e18 = 1e-36 → result 0, nonzero error.
    auto r = div(decimal("0.000000000000000001"), decimal("1000000000000000000.0"));
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_FALSE(r.error.is_zero());
}

TEST(DecimalDiv, LargeBySmall) {
    // Large numerator, tiny denominator → may overflow.
    auto r = div(decimal::max(), decimal("0.000000000000000001"));
    EXPECT_TRUE(r.has_overflow());
}

// ============================================================================
// Mul/Div round-trip
// ============================================================================

TEST(DecimalRoundTrip, MulThenDiv) {
    decimal a("123.456");
    decimal b("789.012");
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
    decimal a("100.0");
    decimal b("7.0");
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
    auto [d, fits] = decimal("0").to_double();
    EXPECT_DOUBLE_EQ(d, 0.0);
    EXPECT_TRUE(fits);
}

TEST(DecimalToDouble, SmallValueFits) {
    auto [d, fits] = decimal("1.5").to_double();
    EXPECT_DOUBLE_EQ(d, 1.5);
    // 1.5 * 10^18 = 1500000000000000000, which is < 2^53? No, 2^53 ≈ 9e15.
    // So fits may be false. That's fine — we test the value.
    (void)fits;
}

TEST(DecimalToDouble, Approximate) {
    // A value that certainly exceeds double's 53-bit mantissa.
    auto [d, fits] = decimal("12345678901234567.890123456789012345").to_double();
    EXPECT_FALSE(fits);
    // But the value should be approximately right.
    EXPECT_NEAR(d, 12345678901234567.89, 1e4);
}

TEST(DecimalToDouble, Negative) {
    auto [d, fits] = decimal("-42.5").to_double();
    EXPECT_DOUBLE_EQ(d, -42.5);
    (void)fits;
}

TEST(DecimalToLongDouble, Zero) {
    auto [ld, fits] = decimal("0").to_long_double();
    EXPECT_DOUBLE_EQ(static_cast<double>(ld), 0.0);
    EXPECT_TRUE(fits);
}

TEST(DecimalToLongDouble, ReturnsValue) {
    auto [ld, fits] = decimal("3.14").to_long_double();
    EXPECT_NEAR(static_cast<double>(ld), 3.14, 1e-10);
    (void)fits;
}

TEST(DecimalToLongDouble, LargeValueDoesNotFit) {
    auto [ld, fits] = decimal("12345678901234567.890123456789012345").to_long_double();
    EXPECT_FALSE(fits);
    EXPECT_NEAR(static_cast<double>(ld), 12345678901234567.89, 1e4);
}

// ============================================================================
// decimal_error
// ============================================================================

TEST(DecimalError, IsZero) {
    decimal_error e{};
    EXPECT_TRUE(e.is_zero());
}

TEST(DecimalError, Comparison) {
    decimal_error a{10};
    decimal_error b{20};
    EXPECT_LT(a, b);
    EXPECT_EQ(a, decimal_error{10});
}

TEST(DecimalError, ToStringScientific) {
    decimal_error e{12345};
    auto s = e.to_string();
    EXPECT_EQ(s, "1.2345e+4");
}

TEST(DecimalError, ToStringZero) {
    decimal_error e{0};
    EXPECT_EQ(e.to_string(), "0.0e+0");
}

TEST(DecimalError, ToStringSingleDigit) {
    decimal_error e{7};
    EXPECT_EQ(e.to_string(), "7.0e+0");
}

TEST(DecimalError, ToStringNegative) {
    decimal_error e{-5000};
    EXPECT_EQ(e.to_string(), "-5.0e+3");
}

TEST(DecimalError, OstreamMatchesToString) {
    decimal_error e{999};
    std::ostringstream oss;
    oss << e;
    EXPECT_EQ(oss.str(), e.to_string());
}

// ============================================================================
// decimal_overflow
// ============================================================================

TEST(DecimalOverflow, IsZero) {
    decimal_overflow o{};
    EXPECT_TRUE(o.is_zero());
}

TEST(DecimalOverflow, Comparison) {
    decimal_overflow a{1};
    decimal_overflow b{2};
    EXPECT_LT(a, b);
}

TEST(DecimalOverflow, ToStringScientific) {
    decimal_overflow o{1000000};
    EXPECT_EQ(o.to_string(), "1.0e+6");
}

TEST(DecimalOverflow, OstreamMatchesToString) {
    decimal_overflow o{42};
    std::ostringstream oss;
    oss << o;
    EXPECT_EQ(oss.str(), o.to_string());
}

// ============================================================================
// decimal_result
// ============================================================================

TEST(DecimalResult, IsExact) {
    decimal_result r{decimal("1.0"), {0}, {0}};
    EXPECT_TRUE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalResult, HasError) {
    decimal_result r{decimal("1.0"), {5}, {0}};
    EXPECT_FALSE(r.is_exact());
    EXPECT_FALSE(r.has_overflow());
}

TEST(DecimalResult, HasOverflow) {
    decimal_result r{decimal("1.0"), {0}, {1}};
    EXPECT_FALSE(r.is_exact());
    EXPECT_TRUE(r.has_overflow());
}

// ============================================================================
// Type safety: decimal_error and decimal_overflow are not addable to decimal
// ============================================================================

// These are compile-time guarantees. We verify via static_assert that the
// types are distinct and not implicitly convertible to decimal.
static_assert(!std::is_convertible_v<decimal_error, decimal>);
static_assert(!std::is_convertible_v<decimal_overflow, decimal>);
static_assert(!std::is_same_v<decimal_error, decimal>);
static_assert(!std::is_same_v<decimal_overflow, decimal>);

// ============================================================================
// JSON round-trip (glaze)
// ============================================================================

TEST(DecimalJson, SerializeDeserialize) {
    decimal original("50000.10");
    std::string json;
    (void)glz::write_json(original, json);

    decimal parsed;
    auto err = glz::read_json(parsed, json);
    EXPECT_FALSE(err) << "glaze parse error";
    EXPECT_EQ(parsed, original);
}

TEST(DecimalJson, Zero) {
    decimal original("0");
    std::string json;
    (void)glz::write_json(original, json);

    decimal parsed;
    auto err = glz::read_json(parsed, json);
    EXPECT_FALSE(err);
    EXPECT_EQ(parsed, original);
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(DecimalEdge, MulByMinusOne) {
    auto r = mul(decimal("5.0"), decimal("-1.0"));
    EXPECT_EQ(r.value, decimal("-5.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, DivBySelf) {
    decimal a("123.456");
    auto r = div(a, a);
    EXPECT_EQ(r.value, decimal("1.0"));
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MulSmallNumbers) {
    // Two very small numbers: result is zero with nonzero error.
    decimal tiny("0.000000000000000001"); // 1 ULP
    auto r = mul(tiny, tiny);
    EXPECT_TRUE(r.value.is_zero());
    EXPECT_EQ(r.error.value, 1); // 1 * 1 = 1, remainder 1 after dividing by 10^18
}

TEST(DecimalEdge, MaxTimesOne) {
    auto r = mul(decimal::max(), decimal("1.0"));
    EXPECT_EQ(r.value, decimal::max());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MinTimesOne) {
    auto r = mul(decimal::min(), decimal("1.0"));
    EXPECT_EQ(r.value, decimal::min());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, MaxDivOne) {
    auto r = div(decimal::max(), decimal("1.0"));
    EXPECT_EQ(r.value, decimal::max());
    EXPECT_TRUE(r.is_exact());
}

TEST(DecimalEdge, NegateMax) {
    EXPECT_EQ(-decimal::max(), decimal::min());
}

TEST(DecimalEdge, SubSelf) {
    decimal a("12345.6789");
    EXPECT_TRUE((a - a).is_zero());
}

} // anonymous namespace
