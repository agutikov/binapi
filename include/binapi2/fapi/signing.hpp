// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace binapi2::fapi {

using query_map = std::map<std::string, std::string>;

[[nodiscard]] std::string
hmac_sha256_hex(const std::string& key, const std::string& data);

[[nodiscard]] std::string
percent_encode(std::string_view value);

[[nodiscard]] std::string
build_query_string(const query_map& query);

void
inject_auth_query(query_map& query, std::uint64_t recv_window, std::uint64_t timestamp_ms);

void
sign_query(query_map& query, const std::string& secret_key);

} // namespace binapi2::fapi
