// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file error.hpp
/// @brief Error classification and diagnostic payload for all fapi operations.

#pragma once

#include <string>

namespace binapi2::fapi {

/// @brief Coarse error category indicating the stage at which a failure occurred.
///
/// Every `error` carries exactly one `error_code`. Callers should switch on this
/// value to decide recovery strategy (retry, reconnect, abort, etc.).
enum class error_code
{
    none = 0,           ///< No error -- the operation succeeded.
    invalid_argument,   ///< Caller supplied an invalid or missing parameter.
    transport,          ///< Network-level failure (DNS, TLS, TCP timeout, etc.).
    http_status,        ///< Server returned a non-2xx HTTP status not mapped to a Binance error.
    json,               ///< Response body could not be parsed as valid JSON.
    binance,            ///< Binance-specific error; see `error::binance_code` for details.
    websocket,          ///< WebSocket protocol error (handshake, frame, unexpected close).
    internal,           ///< Unexpected library-internal failure (bug).
};

/// @brief Rich error descriptor produced by every fapi operation that can fail.
///
/// On success `code` is `error_code::none` and the remaining fields are
/// default-initialised.  On failure the struct carries as much context as
/// available so that callers can log, display, or programmatically inspect the
/// cause.
struct error
{
    error_code code{ error_code::none };

    /// @brief HTTP status code when `code == error_code::http_status` or `binance`.
    int http_status{ 0 };

    /// @brief Binance-defined numeric error code (e.g. -1021 for timestamp issues).
    /// Only meaningful when `code == error_code::binance`.
    int binance_code{ 0 };

    /// @brief Human-readable description of the failure.
    std::string message{};

    /// @brief Raw response body, useful for debugging unexpected server replies.
    std::string payload{};
};

} // namespace binapi2::fapi
