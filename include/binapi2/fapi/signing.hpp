// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file signing.hpp
/// @brief Utilities for building, percent-encoding, and signing Binance query
///        strings as required by the authenticated REST/WS API. Supports both
///        HMAC-SHA256 (legacy) and Ed25519 (recommended) signing methods.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace binapi2::fapi {

/// @brief Ordered key-value map representing URL query parameters.
///
/// Ordering is lexicographic by key, which matches the canonical form Binance
/// expects when computing the signature.
using query_map = std::map<std::string, std::string>;

/// @brief Compute the HMAC-SHA256 of @p data using @p key and return the
///        result as a lower-case hex string.
///
/// This is the core signing primitive required by Binance's authentication
/// scheme (SIGNED endpoints).
/// @param key   The secret key (raw bytes, not hex).
/// @param data  The message to authenticate (typically the full query string).
/// @return 64-character lower-case hex digest.
[[nodiscard]] std::string
hmac_sha256_hex(const std::string& key, const std::string& data);

/// @brief RFC 3986 percent-encode a single query-parameter value.
/// @param value  The raw value to encode.
/// @return The percent-encoded string safe for inclusion in a URL.
[[nodiscard]] std::string
percent_encode(std::string_view value);

/// @brief Serialise a `query_map` into a URL query string (`key=value&...`).
///
/// Values are percent-encoded; keys are assumed to be plain ASCII identifiers.
/// @param query  The parameters to serialise.
/// @return A string ready to append after `?` in a URL.
[[nodiscard]] std::string
build_query_string(const query_map& query);

/// @brief Insert `recvWindow` and `timestamp` parameters into @p query.
///
/// These fields are mandatory for every SIGNED endpoint.  Call this *before*
/// `sign_query()`.
/// @param query          The parameter map to mutate.
/// @param recv_window    Server-side timestamp tolerance in milliseconds.
/// @param timestamp_ms_t   Current UTC time in milliseconds since epoch.
void
inject_auth_query(query_map& query, std::uint64_t recv_window, types::timestamp_ms_t timestamp);

/// @brief Sign the canonical query string using an Ed25519 private key.
///
/// @param pem   PEM-encoded Ed25519 private key.
/// @param data  The message to sign (typically the canonical query string).
/// @return Base64-encoded Ed25519 signature.
[[nodiscard]] std::string
ed25519_sign_base64(const std::string& pem, const std::string& data);

/// @brief Compute and insert the `signature` parameter into @p query.
///
/// Builds the canonical query string from the current map contents, computes
/// the HMAC-SHA256 signature with @p secret_key, and stores the result under
/// the `"signature"` key.  Must be the **last** mutation before the request is
/// sent.
/// @param query       The parameter map to sign (modified in place).
/// @param secret_key  The Binance API secret key.
void
sign_query(query_map& query, const std::string& secret_key);

/// @brief Compute and insert the `signature` parameter using the configured
///        signing method (Ed25519 or HMAC-SHA256).
///
/// @param query  The parameter map to sign (modified in place).
/// @param cfg    Configuration containing keys and sign_method.
void
sign_query(query_map& query, const config& cfg);

} // namespace binapi2::fapi
