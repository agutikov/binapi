// SPDX-License-Identifier: Apache-2.0
//
// Comprehensive tests for the signing utilities (HMAC-SHA256, percent-encoding,
// query-string construction, auth injection, and full request signing).

#include <binapi2/fapi/signing.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace {

using binapi2::fapi::build_query_string;
using binapi2::fapi::hmac_sha256_hex;
using binapi2::fapi::inject_auth_query;
using binapi2::fapi::percent_encode;
using binapi2::fapi::query_map;
using binapi2::fapi::sign_query;

// ============================================================================
// hmac_sha256_hex
// ============================================================================

TEST(HmacSha256Hex, EmptyKeyAndData) {
    // RFC 4231-style known vector: HMAC-SHA256("", "").
    auto result = hmac_sha256_hex("", "");
    EXPECT_EQ(result,
              "b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad");
}

TEST(HmacSha256Hex, KnownVector) {
    // HMAC-SHA256("key", "The quick brown fox jumps over the lazy dog").
    auto result = hmac_sha256_hex(
        "key", "The quick brown fox jumps over the lazy dog");
    EXPECT_EQ(result,
              "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

TEST(HmacSha256Hex, ResultIs64HexChars) {
    auto result = hmac_sha256_hex("any-key", "any-data");
    EXPECT_EQ(result.size(), 64u);
}

TEST(HmacSha256Hex, ResultIsLowercase) {
    auto result = hmac_sha256_hex("key", "data");
    EXPECT_EQ(result.size(), 64u);
    for (char c : result) {
        EXPECT_TRUE(std::isxdigit(static_cast<unsigned char>(c)))
            << "non-hex character: " << c;
        if (std::isalpha(static_cast<unsigned char>(c))) {
            EXPECT_TRUE(std::islower(static_cast<unsigned char>(c)))
                << "uppercase hex character: " << c;
        }
    }
}

// ============================================================================
// percent_encode
// ============================================================================

TEST(PercentEncode, AlphanumericPassesThrough) {
    EXPECT_EQ(percent_encode("abc123"), "abc123");
}

TEST(PercentEncode, UnreservedCharsPassThrough) {
    EXPECT_EQ(percent_encode("-._~"), "-._~");
}

TEST(PercentEncode, SpaceEncoded) {
    EXPECT_EQ(percent_encode(" "), "%20");
}

TEST(PercentEncode, SpecialChars) {
    EXPECT_EQ(percent_encode("hello world!"), "hello%20world%21");
}

TEST(PercentEncode, SlashEncoded) {
    EXPECT_EQ(percent_encode("/"), "%2F");
}

TEST(PercentEncode, EmptyString) {
    EXPECT_EQ(percent_encode(""), "");
}

TEST(PercentEncode, UnicodeByte) {
    // Euro sign U+20AC is UTF-8 bytes 0xE2 0x82 0xAC.
    std::string euro = "\xE2\x82\xAC";
    EXPECT_EQ(percent_encode(euro), "%E2%82%AC");
}

// ============================================================================
// build_query_string
// ============================================================================

TEST(BuildQueryString, EmptyMap) {
    query_map q;
    EXPECT_EQ(build_query_string(q), "");
}

TEST(BuildQueryString, SingleEntry) {
    query_map q{{"symbol", "BTCUSDT"}};
    EXPECT_EQ(build_query_string(q), "symbol=BTCUSDT");
}

TEST(BuildQueryString, MultipleEntriesOrdered) {
    // std::map orders by key: "limit" < "symbol".
    query_map q{{"symbol", "BTCUSDT"}, {"limit", "10"}};
    EXPECT_EQ(build_query_string(q), "limit=10&symbol=BTCUSDT");
}

TEST(BuildQueryString, ValuesArePercentEncoded) {
    query_map q{{"msg", "hello world"}};
    EXPECT_EQ(build_query_string(q), "msg=hello%20world");
}

// ============================================================================
// inject_auth_query
// ============================================================================

TEST(InjectAuthQuery, AddsRecvWindowAndTimestamp) {
    query_map q;
    inject_auth_query(q, 5000, 1234567890);
    EXPECT_EQ(q.at("recvWindow"), "5000");
    EXPECT_EQ(q.at("timestamp"), "1234567890");
}

TEST(InjectAuthQuery, PreservesExistingKeys) {
    query_map q{{"symbol", "BTCUSDT"}};
    inject_auth_query(q, 5000, 1234567890);
    EXPECT_EQ(q.at("symbol"), "BTCUSDT");
    EXPECT_EQ(q.at("recvWindow"), "5000");
    EXPECT_EQ(q.at("timestamp"), "1234567890");
    EXPECT_EQ(q.size(), 3u);
}

// ============================================================================
// sign_query
// ============================================================================

TEST(SignQuery, SignatureKeyIsPresent) {
    query_map q{{"symbol", "BTCUSDT"}, {"timestamp", "1234567890"}};
    sign_query(q, "test_secret");
    EXPECT_TRUE(q.count("signature"));
}

TEST(SignQuery, SignatureIs64HexChars) {
    query_map q{{"symbol", "BTCUSDT"}, {"timestamp", "1234567890"}};
    sign_query(q, "test_secret");
    const auto& sig = q.at("signature");
    EXPECT_EQ(sig.size(), 64u);
    for (char c : sig) {
        EXPECT_TRUE(std::isxdigit(static_cast<unsigned char>(c)));
    }
}

TEST(SignQuery, KnownVector) {
    // The canonical query string for this map (ordered by key) is:
    //   "symbol=BTCUSDT&timestamp=1234567890"
    // The signature = hmac_sha256_hex("test_secret", <canonical>).
    query_map q{{"symbol", "BTCUSDT"}, {"timestamp", "1234567890"}};

    const std::string canonical = "symbol=BTCUSDT&timestamp=1234567890";
    const std::string expected_sig = hmac_sha256_hex("test_secret", canonical);

    sign_query(q, "test_secret");
    EXPECT_EQ(q.at("signature"), expected_sig);
}

TEST(SignQuery, FullPipeline) {
    // Simulate a realistic signing flow: start with trading params, inject auth,
    // then sign.
    query_map q{{"symbol", "BTCUSDT"}, {"side", "BUY"}, {"quantity", "0.1"}};
    inject_auth_query(q, 5000, 1699999999999);
    sign_query(q, "my_api_secret");

    // All original keys preserved.
    EXPECT_EQ(q.at("symbol"), "BTCUSDT");
    EXPECT_EQ(q.at("side"), "BUY");
    EXPECT_EQ(q.at("quantity"), "0.1");
    // Auth keys present.
    EXPECT_EQ(q.at("recvWindow"), "5000");
    EXPECT_EQ(q.at("timestamp"), "1699999999999");
    // Signature present and valid length.
    EXPECT_EQ(q.at("signature").size(), 64u);

    // Verify the signature is consistent with a manual computation.
    // Rebuild canonical from all keys except "signature".
    query_map without_sig = q;
    without_sig.erase("signature");
    const auto canonical = build_query_string(without_sig);
    EXPECT_EQ(q.at("signature"),
              hmac_sha256_hex("my_api_secret", canonical));
}

} // anonymous namespace
