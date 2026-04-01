// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file time.hpp
/// @brief Wall-clock timestamp helper used for request signing.

#pragma once

#include <chrono>
#include <cstdint>

namespace binapi2::fapi {

/// @brief Return the current UTC wall-clock time as milliseconds since the
///        Unix epoch.
///
/// Used to populate the `timestamp` field required by every SIGNED Binance
/// endpoint.  The value comes from `std::chrono::system_clock`, so it is
/// subject to NTP adjustments.
/// @return Milliseconds since 1970-01-01T00:00:00Z.
[[nodiscard]] inline std::uint64_t
current_timestamp_ms()
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

} // namespace binapi2::fapi
