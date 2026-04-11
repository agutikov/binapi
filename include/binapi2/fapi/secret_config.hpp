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
/// Retrieves api_key and secret_key from the provider and sets
/// cfg.api_key and cfg.secret_key.
///
/// Usage:
///   secret_provider::libsecret_provider provider("binapi2");
///   auto r = co_await async_load_credentials(cfg, provider,
///       "binance/api_key", "binance/secret_key");
[[nodiscard]] inline boost::cobalt::task<result<void>>
async_load_credentials(config& cfg, secret_provider::secret_provider& provider,
                       std::string_view api_key_name,
                       std::string_view secret_key_name)
{
    auto api_key = co_await provider.async_get_secret(api_key_name);
    if (!api_key.has_value())
        co_return result<void>::failure(
            {error_code::internal, 0, 0, "failed to load api_key: " + api_key.error(), {}});

    auto secret_key = co_await provider.async_get_secret(secret_key_name);
    if (!secret_key.has_value())
        co_return result<void>::failure(
            {error_code::internal, 0, 0, "failed to load secret_key: " + secret_key.error(), {}});

    cfg.api_key = std::move(*api_key);
    cfg.secret_key = std::move(*secret_key);
    co_return result<void>::success();
}

} // namespace binapi2::fapi
