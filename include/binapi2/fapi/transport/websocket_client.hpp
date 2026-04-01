// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file websocket_client.hpp
/// @brief Asynchronous-primary WebSocket client using the pimpl pattern.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/session_base.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>

#include <functional>
#include <memory>
#include <string>

namespace binapi2::fapi::transport {

/// @brief WebSocket client for the Binance Futures streaming and WebSocket API.
///
/// Follows an async-primary design: all async_ methods are the primary
/// coroutine-based implementations (Boost.Cobalt tasks), while the non-prefixed
/// methods are synchronous wrappers that drive the io_context to completion.
///
/// Uses the pimpl idiom to hide Boost.Beast WebSocket stream internals from
/// the public header.
class websocket_client final : public session_base
{
public:
    /// @brief Callback type for the synchronous read loop.
    ///
    /// Receives each incoming text message. Return @c true to continue reading,
    /// or @c false to break out of the loop and close gracefully.
    using message_handler = std::function<bool(const std::string&)>;

    /// @brief Construct a WebSocket client.
    /// @param io_context Boost.Asio I/O context used for async operations.
    /// @param cfg        Configuration containing endpoint and credential settings.
    websocket_client(boost::asio::io_context& io_context, config cfg);

    /// @brief Destructor; releases the pimpl resources.
    ~websocket_client();

    // -- Async (primary implementation) --

    /// @brief Asynchronously connect to a WebSocket endpoint via TLS.
    /// @param host   Hostname to connect to (e.g. "fstream.binance.com").
    /// @param port   Port number as a string (typically "443").
    /// @param target WebSocket request target path (e.g. "/ws/btcusdt@aggTrade").
    /// @return A cobalt task yielding a result indicating success or connection error.
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string host, std::string port, std::string target);

    /// @brief Asynchronously send a text message over the WebSocket.
    /// @param message The text payload to send (typically JSON).
    /// @return A cobalt task yielding a result indicating success or write error.
    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message);

    /// @brief Asynchronously read a single text message from the WebSocket.
    /// @return A cobalt task yielding a result containing the received text message.
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();

    /// @brief Asynchronously close the WebSocket connection.
    /// @return A cobalt task yielding a result indicating success or close error.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    // -- Sync (wraps async) --

    /// @brief Synchronously connect to a WebSocket endpoint via TLS.
    /// @param host   Hostname to connect to.
    /// @param port   Port number as a string.
    /// @param target WebSocket request target path.
    /// @return A result indicating success or connection error.
    [[nodiscard]] result<void> connect(const std::string& host, const std::string& port, const std::string& target);

    /// @brief Synchronously send a text message over the WebSocket.
    /// @param message The text payload to send.
    /// @return A result indicating success or write error.
    [[nodiscard]] result<void> write_text(const std::string& message);

    /// @brief Synchronously read a single text message from the WebSocket.
    /// @return A result containing the received text message.
    [[nodiscard]] result<std::string> read_text();

    /// @brief Run a synchronous read loop, dispatching each message to @p handler.
    ///
    /// Continuously reads text messages and invokes @p handler for each one.
    /// The loop terminates when the handler returns @c false or an error occurs.
    ///
    /// @param handler Callback invoked for each message; return @c false to stop.
    /// @return A result indicating success or the error that terminated the loop.
    [[nodiscard]] result<void> run_read_loop(message_handler handler);

    /// @brief Synchronously close the WebSocket connection.
    /// @return A result indicating success or close error.
    [[nodiscard]] result<void> close();

private:
    /// @brief Forward-declared implementation holding Boost.Beast stream state.
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::fapi::transport
