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
    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message);
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::fapi::transport
