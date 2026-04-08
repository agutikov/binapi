// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file json_opts.hpp
/// @brief Shared glaze JSON parse options for all binapi2 deserialization.
///
/// These options ensure consistent behavior across REST responses, WebSocket
/// stream events, and WebSocket API RPC responses:
///  - Unknown keys are silently skipped (Binance adds fields without notice).
///  - Missing non-optional keys are an error (catches renamed/removed fields).

#pragma once

#include <glaze/glaze.hpp>

namespace binapi2::fapi::detail {

inline constexpr glz::opts json_read_opts{
    .error_on_unknown_keys = false,
    .error_on_missing_keys = true,
};

} // namespace binapi2::fapi::detail
