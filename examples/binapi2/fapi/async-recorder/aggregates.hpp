// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — per-symbol aggregates for the selector.

/// @file aggregates.hpp
/// @brief Minimal per-symbol rolling aggregates shared between the
///        screener (producer) and the selector (consumer).
///
/// Step 5 scope: only an event-count proxy for "volume" is populated,
/// sourced from `!bookTicker` events. Later steps will extend this to
/// the full set of TF windows (volume / trades / range / NATR on
/// 1m/5m/1h/4h/1d) once we start parsing the array streams.

#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace binapi2::examples::async_recorder {

/// @brief Per-symbol rolling stats. Read by the selector, written by
///        the screener. Same-executor-only — no synchronisation.
struct symbol_agg
{
    /// @brief Monotonic count of bookTicker events observed for this
    /// symbol since startup. Acts as a "volume" proxy for the selector's
    /// scoring until the real TF windows are plumbed through.
    std::size_t events_total{ 0 };
};

/// @brief Map from uppercase symbol string to its aggregate slot.
using aggregates_map = std::unordered_map<std::string, symbol_agg>;

} // namespace binapi2::examples::async_recorder
