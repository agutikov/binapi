// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_streams.hpp
/// @brief Async market data stream client for Binance USD-M Futures WebSocket streams.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>

#include <boost/cobalt/task.hpp>

#include <string>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Async client for Binance USD-M Futures market data WebSocket streams.
///
/// Provides low-level async transport access. The caller builds the target path
/// and parses the received JSON frames.
///
/// Usage:
///   co_await streams.async_connect("/ws/btcusdt@bookTicker");
///   while (auto msg = co_await streams.async_read_text()) { ... }
///   co_await streams.async_close();
class market_streams
{
public:
    explicit market_streams(config cfg);

    /// @brief Connect to a stream endpoint.
    /// @param target Full WebSocket target path (e.g. "/ws/btcusdt@bookTicker").
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string target);

    /// @brief Read a single raw text frame.
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();

    /// @brief Close the stream connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    // -- Combined stream management --

    /// @brief Subscribe to stream topics on an existing combined connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_subscribe(const std::vector<std::string>& streams);

    /// @brief Unsubscribe from stream topics.
    [[nodiscard]] boost::cobalt::task<result<void>> async_unsubscribe(const std::vector<std::string>& streams);

    /// @brief List active subscriptions.
    [[nodiscard]] boost::cobalt::task<result<std::vector<std::string>>> async_list_subscriptions();

    /// @brief Access the stream config.
    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }

private:
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
