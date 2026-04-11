// SPDX-License-Identifier: Apache-2.0

/// @file libsecret_provider.hpp
/// @brief Secret provider using libsecret (GNOME Keyring / KDE Wallet / Secret Service).
///
/// Only available when SECRET_PROVIDER_HAS_LIBSECRET is defined (libsecret-1 found at build time).

#pragma once

#if !defined(SECRET_PROVIDER_HAS_LIBSECRET)
#error "libsecret_provider requires SECRET_PROVIDER_HAS_LIBSECRET (libsecret-1 not found)"
#endif

#include <secret_provider/secret_provider.hpp>

#include <string>
#include <string_view>

namespace secret_provider {

/// @brief Retrieves secrets from the system keyring via libsecret.
///
/// Uses the Secret Service D-Bus API (GNOME Keyring, KDE Wallet, etc.).
/// Secrets are stored with attributes { "service": service_name, "key": key }.
///
/// Usage:
///   libsecret_provider provider("binapi2");
///   auto key = co_await provider.async_get_secret("binance/api_key");
///
/// Store secrets with secret-tool:
///   secret-tool store --label "binapi2 API key" service binapi2 key binance/api_key
class libsecret_provider : public secret_provider
{
public:
    /// @param service_name  Application identifier for the keyring schema.
    explicit libsecret_provider(std::string service_name);

    boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) override;

private:
    std::string service_name_;
};

} // namespace secret_provider
