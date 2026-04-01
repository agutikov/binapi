// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements Binance request signing: HMAC-SHA256 signature computation,
/// RFC 3986 percent-encoding, canonical query-string construction, and auth
/// parameter injection. The signing procedure follows the Binance API docs:
/// build a sorted, percent-encoded query string from all parameters, then
/// HMAC-SHA256 it with the secret key and append the hex digest as "signature".

#include <binapi2/fapi/signing.hpp>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <array>
#include <cctype>
#include <cstdio>

namespace binapi2::fapi {

namespace {

// Converts raw bytes to a lowercase hex string. Uses a lookup table rather
// than snprintf for performance, since this runs on every signed request.
std::string
to_hex(const unsigned char* data, std::size_t size)
{
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        out.push_back(hex[(data[i] >> 4) & 0x0f]);
        out.push_back(hex[data[i] & 0x0f]);
    }
    return out;
}

} // namespace

// Computes HMAC-SHA256 of `data` keyed by `key` via OpenSSL, returning the
// digest as a lowercase hex string. The digest buffer is sized to
// EVP_MAX_MD_SIZE to accommodate any hash algorithm, though SHA-256 always
// produces 32 bytes.
std::string
hmac_sha256_hex(const std::string& key, const std::string& data)
{
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_size{};
    HMAC(EVP_sha256(),
         key.data(),
         static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()),
         data.size(),
         digest.data(),
         &digest_size);
    return to_hex(digest.data(), digest_size);
}

// RFC 3986 percent-encoding: unreserved characters (alphanum, '-', '_', '.',
// '~') pass through; everything else is encoded as %XX uppercase hex. This is
// critical for signature correctness -- the canonical query string must use
// exactly this encoding.
std::string
percent_encode(std::string_view value)
{
    std::string out;
    for (const unsigned char ch : value) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            out.push_back(static_cast<char>(ch));
        } else {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", ch);
            out.append(buf);
        }
    }
    return out;
}

// Assembles a canonical query string from a sorted map. Because query_map is
// std::map (ordered), iteration order is deterministic, which is essential
// for reproducible signatures -- the server reconstructs this same string
// from the received parameters to verify the HMAC.
std::string
build_query_string(const query_map& query)
{
    std::string out;
    for (const auto& [key, value] : query) {
        if (!out.empty()) {
            out.push_back('&');
        }
        out += percent_encode(key);
        out.push_back('=');
        out += percent_encode(value);
    }
    return out;
}

void
inject_auth_query(query_map& query, std::uint64_t recv_window, std::uint64_t timestamp_ms)
{
    query["recvWindow"] = std::to_string(recv_window);
    query["timestamp"] = std::to_string(timestamp_ms);
}

// Builds the canonical query string from all current parameters (excluding
// "signature" itself, which must not yet be present), then appends the HMAC
// hex digest as the "signature" parameter. This must be the last mutation
// before the query is sent.
void
sign_query(query_map& query, const std::string& secret_key)
{
    const auto canonical = build_query_string(query);
    query["signature"] = hmac_sha256_hex(secret_key, canonical);
}

} // namespace binapi2::fapi
