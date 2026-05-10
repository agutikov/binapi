// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor subscription management.

/// @file detail/subs.hpp

#pragma once

#include "../config.hpp"
#include "types.hpp"

#include <binapi2/fapi/rest/client.hpp>
#include <binapi2/fapi/streams/dynamic_market_stream.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/cobalt/task.hpp>

namespace binapi2::examples::async_recorder {
class selector;
}

namespace binapi2::examples::async_recorder::detail_impl {

/// @brief Periodic loop: snapshot the selector's active set, diff
///        against `st.subscribed`, fire `SUBSCRIBE` packs for new symbols
///        (with REST depth snapshot first, partitioned across the two
///        per-category connections) and `UNSUBSCRIBE` packs for removed
///        ones. Closes the buffer on shutdown so the drain loop unwinds.
///
/// Binance's 2026 URL split (see binapi2/fapi/stream_category.hpp)
/// requires `bookTicker` + depth on one `/public/stream` connection and
/// `aggTrade` + `markPrice` + `kline` + `forceOrder` on a separate
/// `/market/stream` connection. The example owns both and the loop
/// fans SUBSCRIBE/UNSUBSCRIBE to each as appropriate per stream type.
boost::cobalt::task<void>
manage_subs_loop(const recorder_config& cfg,
                 selector& sel,
                 ::binapi2::fapi::streams::dynamic_market_stream& dyn_pub,
                 ::binapi2::fapi::streams::dynamic_market_stream& dyn_mkt,
                 ::binapi2::fapi::rest::client& rest_client,
                 detail_state& st,
                 boost::asio::io_context& ioc);

/// @brief Drive the connection's read loop so raw frames keep flowing
///        into the attached `record_buf`. Free function (not a lambda)
///        to dodge the GCC 15 coroutine-lambda ICE.
boost::cobalt::task<void>
connection_read_loop(::binapi2::fapi::streams::dynamic_market_stream& d);

} // namespace binapi2::examples::async_recorder::detail_impl
