// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file websocket_transport.hpp
/// @brief Concept defining the WebSocket transport interface.
///
/// Any type satisfying websocket_transport can be used as the Transport
/// parameter for basic_market_stream and basic_user_stream.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/task.hpp>

#include <concepts>
#include <string>

namespace binapi2::fapi::transport {

template<class T>
concept websocket_transport = std::constructible_from<T, config> &&
    requires(T t, std::string s, ws_target_t target) {
        { t.async_connect(s, s, target) } -> std::same_as<boost::cobalt::task<result<void>>>;
        { t.async_read_text() } -> std::same_as<boost::cobalt::task<result<std::string>>>;
        { t.async_write_text(s) } -> std::same_as<boost::cobalt::task<result<void>>>;
        { t.async_close() } -> std::same_as<boost::cobalt::task<result<void>>>;
    };

} // namespace binapi2::fapi::transport
