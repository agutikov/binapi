// SPDX-License-Identifier: Apache-2.0

/// @file factory.hpp
/// @brief Factory for creating secret providers by name.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace secret_provider {

/// @brief Create a secret provider by name.
///
/// Supported names:
///   "libsecret"             — GNOME Keyring / KDE Wallet (requires libsecret-1)
///   "systemd-creds:<dir>"   — systemd encrypted credentials from <dir>
///   "env"                   — environment variables (deprecated)
///   "test"                  — empty in-memory provider (for testing)
///
/// @return Provider instance, or nullptr if the name is unknown or unsupported.
std::unique_ptr<secret_provider> create(std::string_view name);

/// @brief List available provider names on this build.
std::vector<std::string> available();

} // namespace secret_provider
