// SPDX-License-Identifier: Apache-2.0

/// @file secret_provider.hpp
/// @brief Abstract base class for secret/credential providers.

#pragma once

#include <boost/cobalt/task.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace secret_provider {

/// @brief Abstract base class for retrieving secrets from a credential store.
///
/// Implementations may use libsecret (GNOME Keyring / KDE Wallet),
/// systemd-creds (encrypted systemd credentials), environment variables,
/// or an in-memory map for testing.
class secret_provider
{
public:
    virtual ~secret_provider() = default;

    /// @brief Retrieve a secret by key.
    /// @param key  Secret identifier (e.g. "binance/api_key").
    /// @return The secret value on success, or an error message on failure.
    [[nodiscard]] virtual boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) = 0;
};

} // namespace secret_provider
