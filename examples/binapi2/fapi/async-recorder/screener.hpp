// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 1: screener.

/// @file screener.hpp
/// @brief Market-wide screener stage: subscribe to the all-* streams and
///        write each raw frame to its own rotating JSONL segment.
///
/// Scope for Step 3: raw archive only. Aggregates map / scoring / selector
/// are added in later steps.

#pragma once

#include "aggregates.hpp"
#include "config.hpp"

#include <boost/cobalt/task.hpp>

namespace binapi2::examples::async_recorder {

class status_reporter;

/// @brief Run the full screener: three all-market streams (bookTicker,
///        markPriceArr, tickerArr) recorded in parallel to three rotating
///        JSONL sinks. Populates `aggs` from bookTicker events. Registers
///        a status source named "screener" on the reporter. Returns when
///        all read loops have exited.
boost::cobalt::task<int>
screener_run(const recorder_config& cfg,
             aggregates_map& aggs,
             status_reporter& status);

/// @brief Run the single-stream debug screener. Subscribes to exactly one
///        of the all-market feeds (chosen by `cfg.debug_stream`) and
///        records it. Aggregates are populated only when the feed is
///        bookTicker. Also registers a "screener" source on the reporter.
boost::cobalt::task<int>
debug_screener_run(const recorder_config& cfg,
                   aggregates_map& aggs,
                   status_reporter& status);

} // namespace binapi2::examples::async_recorder
