// SPDX-License-Identifier: Apache-2.0

/// @file systemd_creds_provider.hpp
/// @brief Secret provider using systemd-creds for encrypted credential files.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <filesystem>
#include <string>
#include <string_view>

namespace secret_provider {

/// @brief Retrieves secrets by decrypting systemd credential files.
///
/// Each secret is stored as an encrypted file. The provider runs
/// `systemd-creds decrypt <file> -` and captures the decrypted output.
///
/// Usage:
///   systemd_creds_provider provider("/etc/credstore");
///   auto key = co_await provider.async_get_secret("binance-api-key");
class systemd_creds_provider : public secret_provider
{
public:
    /// @param creds_dir  Directory containing encrypted credential files.
    explicit systemd_creds_provider(std::filesystem::path creds_dir);

    boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) override;

private:
    std::filesystem::path creds_dir_;
};

} // namespace secret_provider
