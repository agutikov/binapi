// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file transport_logger.hpp
/// @brief Optional transport-level logging callback for inspecting raw
///        HTTP and WebSocket traffic.

#pragma once

#include <functional>
#include <string>

namespace binapi2::fapi {

/// @brief Direction of the logged message.
enum class transport_direction { sent = 0, received = 1 };

/// @brief A single transport-level log entry describing an HTTP request/response
///        or a WebSocket message.
struct transport_log_entry
{
    transport_direction direction;
    std::string protocol;    ///< "HTTP" or "WS".
    std::string method;      ///< HTTP verb ("GET", "POST", ...) or "WS" for WebSocket.
    std::string target;      ///< HTTP request target path (empty for WS messages).
    int status{};            ///< HTTP response status code (0 for requests and WS).
    std::string body;        ///< Request/response body or WebSocket message text.
    std::string raw;         ///< Full serialized HTTP message with headers (empty for WS).
};

/// @brief Optional callback invoked by the transport layer for every message
///        sent or received. Set on `config::logger` to enable.
using transport_logger = std::function<void(const transport_log_entry&)>;

} // namespace binapi2::fapi
