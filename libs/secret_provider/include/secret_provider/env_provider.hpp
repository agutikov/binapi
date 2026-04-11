// SPDX-License-Identifier: Apache-2.0

/// @file env_provider.hpp
/// @brief DEPRECATED: Secret provider reading from environment variables.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <cstdlib>
#include <string>
#include <string_view>

namespace secret_provider {

/// @brief DEPRECATED: Reads secrets from environment variables.
///
/// The key is used directly as the environment variable name.
/// e.g. async_get_secret("BINANCE_API_KEY") reads $BINANCE_API_KEY.
class env_provider : public secret_provider
{
public:
    boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) override
    {
        std::string env_name(key);
        const char* val = std::getenv(env_name.c_str());
        if (val)
            co_return std::string(val);
        co_return std::unexpected("environment variable not set: " + env_name);
    }
};

} // namespace secret_provider
