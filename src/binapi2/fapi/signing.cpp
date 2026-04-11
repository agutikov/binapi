// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements Binance request signing: HMAC-SHA256 and Ed25519 signature
/// computation, RFC 3986 percent-encoding, canonical query-string construction,
/// and auth parameter injection. The signing procedure follows the Binance API
/// docs: build a sorted, percent-encoded query string from all parameters, then
/// sign it with the configured method and append the result as "signature".

#include <binapi2/fapi/signing.hpp>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>

#include <array>
#include <cctype>
#include <cstdio>
#include <stdexcept>
#include <vector>

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
inject_auth_query(query_map& query, std::uint64_t recv_window, types::timestamp_ms_t timestamp)
{
    query["recvWindow"] = std::to_string(recv_window);
    query["timestamp"] = timestamp.to_string();
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

namespace {

// Base64-encode raw bytes using OpenSSL BIO chain.
std::string
base64_encode(const unsigned char* data, std::size_t len)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, data, static_cast<int>(len));
    BIO_flush(b64);

    BUF_MEM* buf{};
    BIO_get_mem_ptr(b64, &buf);
    std::string result(buf->data, buf->length);
    BIO_free_all(b64);
    return result;
}

} // namespace

// Signs data with an Ed25519 private key (PEM-encoded) using OpenSSL's EVP
// API. Returns the signature as a base64-encoded string, which is the format
// Binance expects for Ed25519 signatures.
std::string
ed25519_sign_base64(const std::string& pem, const std::string& data)
{
    BIO* bio = BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size()));
    if (!bio)
        throw std::runtime_error("ed25519_sign: failed to create BIO");

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey)
        throw std::runtime_error("ed25519_sign: failed to parse Ed25519 private key PEM");

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("ed25519_sign: failed to create EVP_MD_CTX");
    }

    std::string result;
    std::size_t sig_len = 0;

    if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, pkey) != 1 ||
        EVP_DigestSign(ctx, nullptr, &sig_len,
                       reinterpret_cast<const unsigned char*>(data.data()), data.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("ed25519_sign: EVP_DigestSign init failed");
    }

    std::vector<unsigned char> sig(sig_len);
    if (EVP_DigestSign(ctx, sig.data(), &sig_len,
                       reinterpret_cast<const unsigned char*>(data.data()), data.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("ed25519_sign: EVP_DigestSign failed");
    }

    result = base64_encode(sig.data(), sig_len);

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return result;
}

// Config-aware sign_query: dispatches to Ed25519 or HMAC based on
// cfg.sign_method.
void
sign_query(query_map& query, const config& cfg)
{
    const auto canonical = build_query_string(query);
    switch (cfg.sign_method) {
    case sign_method_t::ed25519:
        query["signature"] = ed25519_sign_base64(cfg.ed25519_private_key_pem, canonical);
        break;
    case sign_method_t::hmac:
        query["signature"] = hmac_sha256_hex(cfg.secret_key, canonical);
        break;
    }
}

} // namespace binapi2::fapi
