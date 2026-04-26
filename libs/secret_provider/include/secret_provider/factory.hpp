// SPDX-License-Identifier: Apache-2.0

/// @file factory.hpp
/// @brief Factory for creating secret providers by name.

#pragma once

#include <secret_provider/secret_provider.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace secret_provider {

/// @brief Create a secret provider by name.
///
/// Built-in names:
///   "libsecret"             — GNOME Keyring / KDE Wallet (requires libsecret-1)
///   "systemd-creds:<dir>"   — systemd encrypted credentials from <dir>
///   "dir:<dir>"             — plain-file-per-key from <dir>
///   "env"                   — environment variables (deprecated)
///   "test"                  — empty in-memory provider (for testing)
///
/// Names registered via register_provider() are resolved as a fallback
/// after built-ins, so built-in names cannot be overridden.
///
/// @return Provider instance, or nullptr if the name is unknown or unsupported.
std::unique_ptr<secret_provider> create(std::string_view name);

/// @brief List available provider names on this build (built-ins + registered).
std::vector<std::string> available();

/// @brief Factory function for a custom secret provider.
///
/// @param arg  The portion of the name after the prefix's ":", or empty
///             when the registered prefix matched as an exact name.
using factory_fn = std::function<std::unique_ptr<secret_provider>(std::string_view arg)>;

/// @brief Register a custom secret provider.
///
/// The @p prefix matches @c create() input either:
///   * exactly (prefix "myvault" matches "myvault"), or
///   * as the head of a "<prefix>:<arg>" form (prefix "myvault" matches "myvault:foo").
///
/// In both cases @p fn receives the trailing argument (empty for the exact-match form).
/// Registration is process-global and thread-safe; intended to be called once at startup.
/// Built-in names cannot be overridden — registration of a name that collides with a
/// built-in is silently inert at lookup time. Prefixes containing ':' are not supported.
void register_provider(std::string prefix, factory_fn fn);

} // namespace secret_provider
