// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 4: REST periodic sync.

/// @file rest_sync.hpp
/// @brief Periodic REST fetches for data that isn't in the WS streams:
///        funding rate, 1m klines, open interest history, long/short
///        ratio. Scoped to the selector's active set (except fundingRate,
///        which is market-wide).

#pragma once

#include "config.hpp"

#include <boost/cobalt/task.hpp>

namespace binapi2::examples::async_recorder {

class selector;
class status_reporter;

/// @brief Run the REST periodic sync. Creates its own rest::client,
///        schedules fundingRate / klines_1m / open_interest_hist /
///        long_short_ratio fetches at their configured cadences, and
///        appends each response as one JSONL line to a per-endpoint
///        (per-symbol for scoped endpoints) file under `rest/`.
///        Registers a "rest" source on the reporter.
boost::cobalt::task<void>
rest_sync_run(const recorder_config& cfg,
              selector& sel,
              status_reporter& status);

} // namespace binapi2::examples::async_recorder
