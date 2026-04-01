// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

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

class websocket_client final : public session_base
{
public:
    using message_handler = std::function<bool(const std::string&)>;

    websocket_client(boost::asio::io_context& io_context, config cfg);
    ~websocket_client();

    // Async (primary implementation).
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string host, std::string port, std::string target);
    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message);
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    // Sync (wraps async).
    [[nodiscard]] result<void> connect(const std::string& host, const std::string& port, const std::string& target);
    [[nodiscard]] result<void> write_text(const std::string& message);
    [[nodiscard]] result<std::string> read_text();
    [[nodiscard]] result<void> run_read_loop(message_handler handler);
    [[nodiscard]] result<void> close();

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::fapi::transport
