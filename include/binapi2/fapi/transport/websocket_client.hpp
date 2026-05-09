// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file websocket_client.hpp
/// @brief Async WebSocket client using the pimpl pattern.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/session_base.hpp>

#include <boost/cobalt/task.hpp>

#include <memory>
#include <string>

namespace binapi2::fapi::transport {

/// @brief Async WebSocket client for the Binance Futures streaming and WebSocket API.
///
/// All methods are coroutine-based (Boost.Cobalt tasks). The coroutine runs on
/// whatever executor drives it (via co_await this_coro::executor).
///
/// Uses the pimpl idiom to hide Boost.Beast WebSocket stream internals.
class websocket_client final : public session_base
{
public:
    explicit websocket_client(config cfg);
    ~websocket_client();

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string host, std::string port, ws_target_t target);

    /// @brief Connect to an explicit IP, keeping a separate hostname for SNI/TLS validation.
    ///
    /// Used by callers that want to steer around a known-bad endpoint in the
    /// resolver pool: skip DNS, TCP-connect to @p ip directly, but still
    /// validate the TLS certificate against @p host_for_sni and send the
    /// hostname (not the IP) in SNI and WS Host. The caller owns IP picking.
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect_to(std::string ip, std::string port, std::string host_for_sni, ws_target_t target);

    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message);
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    /// @brief Resolved peer endpoint of the most recent successful connect, formatted as "ip:port".
    /// Empty until a successful connect has happened on this client.
    [[nodiscard]] std::string last_remote_endpoint() const;

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::fapi::transport
