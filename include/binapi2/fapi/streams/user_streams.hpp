// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_streams.hpp
/// @brief Async user data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace binapi2::fapi::streams {

/// @brief Typed async event generator for user data streams.
using user_event_generator = boost::cobalt::generator<result<types::user_stream_event_t>>;

/// @brief Async client for user data stream events.
///
/// Usage (generator — recommended):
///   auto stream = user_streams.subscribe(listen_key);
///   while (stream) {
///       auto event = co_await stream;
///       if (!event) break;
///       std::visit(overloaded{
///           [](const types::order_trade_update_event_t& e) { ... },
///           [](const auto&) {}
///       }, *event);
///   }
///
/// Usage (low-level):
///   co_await user_streams.async_connect(listen_key);
///   auto msg = co_await user_streams.async_read_text();
class user_streams
{
public:
    explicit user_streams(config cfg);

    // -- Generator --

    /// @brief Subscribe and return a typed async generator yielding user_stream_event_t variants.
    user_event_generator subscribe(std::string listen_key);

    // -- Low-level --

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string listen_key);
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

private:
    transport::websocket_client transport_;
    config cfg_;

    /// @brief Detect event type string from raw payload and parse into variant.
    static result<types::user_stream_event_t> parse_event(const std::string& payload);
};

} // namespace binapi2::fapi::streams
