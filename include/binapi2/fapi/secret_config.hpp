// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file secret_config.hpp
/// @brief Load API credentials from a secret_provider into a config.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>

#include <secret_provider/secret_provider.hpp>

#include <boost/cobalt/task.hpp>

#include <string_view>

namespace binapi2::fapi {

/// @brief Load API credentials into a config from a secret provider.
///
/// Retrieves api_key and the signing key(s) from the provider.
/// For Ed25519 (default): loads api_key and ed25519_private_key (PEM).
/// For HMAC: loads api_key and secret_key.
///
/// Usage:
///   secret_provider::libsecret_provider provider("binapi2");
///   auto r = co_await async_load_credentials(cfg, provider,
///       "demo/api_key", "demo/ed25519_private_key", "demo/secret_key");
[[nodiscard]] inline boost::cobalt::task<result<void>>
async_load_credentials(config& cfg, secret_provider::secret_provider& provider,
                       std::string_view api_key_name,
                       std::string_view ed25519_key_name,
                       std::string_view secret_key_name)
{
    auto trim = [](std::string& s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
            s.pop_back();
    };

    auto api_key = co_await provider.async_get_secret(api_key_name);
    if (!api_key.has_value())
        co_return result<void>::failure(
            {error_code::internal, 0, 0, "failed to load api_key: " + api_key.error(), {}});
    trim(*api_key);
    cfg.api_key = std::move(*api_key);

    if (cfg.sign_method == sign_method_t::ed25519) {
        auto pem = co_await provider.async_get_secret(ed25519_key_name);
        if (!pem.has_value())
            co_return result<void>::failure(
                {error_code::internal, 0, 0,
                 "failed to load ed25519_private_key: " + pem.error(), {}});
        // PEM keys need trailing newline — only trim carriage returns and spaces.
        while (!pem->empty() && (pem->back() == '\r' || pem->back() == ' '))
            pem->pop_back();
        cfg.ed25519_private_key_pem = std::move(*pem);
    } else {
        auto secret_key = co_await provider.async_get_secret(secret_key_name);
        if (!secret_key.has_value())
            co_return result<void>::failure(
                {error_code::internal, 0, 0,
                 "failed to load secret_key: " + secret_key.error(), {}});
        trim(*secret_key);
        cfg.secret_key = std::move(*secret_key);
    }

    co_return result<void>::success();
}

} // namespace binapi2::fapi
