// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_connection.hpp
/// @brief Raw WebSocket stream connection — owns the transport, manages
///        connect/close, produces raw JSON frames. Protocol-agnostic.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>

#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>

#include <string>
#include <utility>
#include <vector>

namespace binapi2::fapi::streams {

namespace detail {

/// @brief Control message for combined stream SUBSCRIBE/UNSUBSCRIBE/LIST_SUBSCRIPTIONS.
struct stream_control_request
{
    std::string method{};
    std::vector<std::string> params{};
    unsigned int id{};
};

} // namespace detail

} // namespace binapi2::fapi::streams

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_control_request>
{
    using T = binapi2::fapi::streams::detail::stream_control_request;
    static constexpr auto value = object("method", &T::method, "params", &T::params, "id", &T::id);
};

namespace binapi2::fapi::streams {

/// @brief Raw WebSocket stream connection.
///
/// Owns the WebSocket transport. Manages connect/close and raw frame I/O.
/// Knows nothing about Binance stream protocol, subscriptions, or event types.
///
/// Used directly by components that need raw access (local_order_book,
/// combined_market_stream). Higher-level components (market_stream,
/// user_stream) wrap this and add typed parsing.
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client>
class basic_stream_connection
{
public:
    explicit basic_stream_connection(config cfg) :
        transport_(cfg), cfg_(std::move(cfg))
    {
    }

    basic_stream_connection(const basic_stream_connection&) = delete;
    basic_stream_connection& operator=(const basic_stream_connection&) = delete;

    // -- Connection --

    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect(std::string host, std::string port, ws_target_t target)
    {
        co_return co_await transport_.async_connect(std::move(host), std::move(port), std::move(target));
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await transport_.async_close();
    }

    // -- Raw frame I/O --

    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text()
    {
        co_return co_await transport_.async_read_text();
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message)
    {
        co_return co_await transport_.async_write_text(std::move(message));
    }

    // -- Accessors --

    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }
    [[nodiscard]] Transport& transport() noexcept { return transport_; }

private:
    Transport transport_;
    config cfg_;
};

/// @brief Default stream connection using the WebSocket transport.
using stream_connection = basic_stream_connection<>;

} // namespace binapi2::fapi::streams
