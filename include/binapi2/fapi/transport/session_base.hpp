// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file session_base.hpp
/// @brief Base class for transport sessions holding shared configuration.

#pragma once

#include <binapi2/fapi/config.hpp>

#include <utility>

namespace binapi2::fapi::transport {

/// @brief Abstract base class for transport sessions (HTTP and WebSocket).
///
/// Stores common configuration (endpoint URLs, API credentials) shared by all
/// concrete transport implementations.
class session_base
{
public:
    /// @brief Construct a session with the given configuration.
    /// @param cfg Configuration object containing endpoint and credential settings.
    explicit session_base(config cfg) : cfg_(std::move(cfg)) {}

    /// @brief Virtual destructor for safe polymorphic deletion.
    virtual ~session_base() = default;

protected:
    config cfg_; ///< Shared configuration for derived transport sessions.
};

} // namespace binapi2::fapi::transport
