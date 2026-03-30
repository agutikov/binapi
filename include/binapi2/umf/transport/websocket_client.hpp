#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/transport/session_base.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <memory>
#include <string>

namespace binapi2::umf::transport {

class websocket_client final : public session_base
{
public:
    using message_handler = std::function<bool(const std::string&)>;

    websocket_client(boost::asio::io_context& io_context, config cfg);
    ~websocket_client();

    [[nodiscard]] result<void> connect(const std::string& host, const std::string& port, const std::string& target);
    [[nodiscard]] result<void> write_text(const std::string& message);
    [[nodiscard]] result<std::string> read_text();
    [[nodiscard]] result<void> run_read_loop(message_handler handler);
    [[nodiscard]] result<void> close();

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::umf::transport
