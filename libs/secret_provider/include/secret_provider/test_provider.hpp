// SPDX-License-Identifier: Apache-2.0

/// @file test_provider.hpp
/// @brief In-memory secret provider for testing.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <map>
#include <string>
#include <string_view>

namespace secret_provider {

/// @brief In-memory secret provider. For tests only.
class test_provider : public secret_provider
{
public:
    void set(std::string key, std::string value) { secrets_[std::move(key)] = std::move(value); }

    boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) override
    {
        auto it = secrets_.find(key);
        if (it != secrets_.end())
            co_return it->second;
        co_return std::unexpected("key not found: " + std::string(key));
    }

private:
    std::map<std::string, std::string, std::less<>> secrets_;
};

} // namespace secret_provider
