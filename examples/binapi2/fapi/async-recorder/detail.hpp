// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 3: per-symbol detail monitor.

/// @file detail.hpp
/// @brief Detail monitor stage. Subscribes to Tier-0 per-symbol streams
///        (aggTrade + bookTicker in this step) for every symbol the
///        selector admits, writes raw frames to
///        `detail/<SYMBOL>/<stream>/...jsonl[.zst]`, and drops those
///        subscriptions when the selector evicts the symbol.

#pragma once

#include "config.hpp"

#include <boost/cobalt/task.hpp>

namespace binapi2::examples::async_recorder {

class selector;
class status_reporter;

/// @brief Run the detail monitor. Polls `sel.active()` on an interval,
///        sends SUBSCRIBE / UNSUBSCRIBE control messages on one
///        dynamic_market_stream, and routes incoming frames to
///        per-symbol rotating JSONL sinks. Registers a "detail" source
///        on the reporter. Returns on shutdown.
boost::cobalt::task<void>
detail_monitor_run(const recorder_config& cfg,
                   selector& sel,
                   status_reporter& status);

} // namespace binapi2::examples::async_recorder
