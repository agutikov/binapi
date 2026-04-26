// SPDX-License-Identifier: Apache-2.0

/// @file dir_provider.hpp
/// @brief Secret provider that reads plain files from a directory.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <filesystem>
#include <string_view>

namespace secret_provider {

/// @brief Reads secrets as plain files from a directory.
///
/// Each secret is a file named after the key (e.g. <dir>/binance_api_key,
/// or, for hierarchical keys, <dir>/binance/api_key). The file's contents
/// are returned verbatim with one trailing newline (and CR) stripped.
///
/// The provider performs no decryption. It expects the directory's
/// contents and permissions to be controlled by the host (compose
/// secrets at /run/secrets, tmpfs bind-mount, …).
///
/// Usage:
///   dir_provider provider("/run/secrets");
///   auto key = co_await provider.async_get_secret("binance_api_key");
class dir_provider : public secret_provider
{
public:
    /// @param dir  Directory containing per-key secret files.
    explicit dir_provider(std::filesystem::path dir);

    boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) override;

private:
    std::filesystem::path dir_;
};

} // namespace secret_provider
