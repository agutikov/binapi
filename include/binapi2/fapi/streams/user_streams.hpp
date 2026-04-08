// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_streams.hpp
/// @brief Async user data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>

#include <boost/cobalt/task.hpp>

#include <string>

namespace binapi2::fapi::streams {

/// @brief Async client for user data stream events (orders, balances, margin calls).
///
/// Requires a listen key obtained from the REST API.
/// The caller reads raw frames via async_read_text() and dispatches by event type.
class user_streams
{
public:
    explicit user_streams(config cfg);

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string listen_key);
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

private:
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
