// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/transport/websocket_client.hpp>

#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/transport/ssl_context.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/run.hpp>

#include <openssl/err.h>

namespace binapi2::fapi::transport {

struct websocket_client::impl
{
    impl(boost::asio::io_context& io_context, config cfg) :
        ssl_ctx(make_ssl_context()), resolver(io_context), stream(io_context, ssl_ctx), cfg(std::move(cfg))
    {
    }

    boost::asio::ssl::context ssl_ctx;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> stream;
    boost::beast::flat_buffer buffer;
    config cfg;
    bool connected{ false };
};

websocket_client::websocket_client(boost::asio::io_context& io_context, config cfg) :
    session_base(cfg), impl_(std::make_unique<impl>(io_context, std::move(cfg)))
{
}

websocket_client::~websocket_client() = default;

// --- Async (primary) ---

boost::cobalt::task<result<void>>
websocket_client::async_connect(std::string host, std::string port, std::string target)
{
    try {
        if (!SSL_set_tlsext_host_name(impl_->stream.next_layer().native_handle(), host.c_str())) {
            co_return result<void>::failure(
                { error_code::websocket, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
        }

        auto endpoints = co_await impl_->resolver.async_resolve(host, port, boost::cobalt::use_op);

        co_await boost::asio::async_connect(impl_->stream.next_layer().next_layer(), endpoints, boost::cobalt::use_op);

        impl_->stream.control_callback([this](boost::beast::websocket::frame_type kind, boost::beast::string_view) {
            if (kind == boost::beast::websocket::frame_type::ping) {
                boost::system::error_code ignored;
                impl_->stream.pong({}, ignored);
            }
        });

        co_await impl_->stream.next_layer().async_handshake(
            boost::asio::ssl::stream_base::client, boost::cobalt::use_op);

        co_await impl_->stream.async_handshake(host, target, boost::cobalt::use_op);

        impl_->connected = true;
        co_return result<void>::success();
    }
    catch (const boost::system::system_error& e) {
        co_return result<void>::failure({ error_code::websocket, 0, 0, e.what(), {} });
    }
}

boost::cobalt::task<result<void>>
websocket_client::async_write_text(std::string message)
{
    try {
        co_await impl_->stream.async_write(boost::asio::buffer(message), boost::cobalt::use_op);
        co_return result<void>::success();
    }
    catch (const boost::system::system_error& e) {
        co_return result<void>::failure({ error_code::websocket, 0, 0, e.what(), {} });
    }
}

boost::cobalt::task<result<std::string>>
websocket_client::async_read_text()
{
    try {
        impl_->buffer.consume(impl_->buffer.size());
        co_await impl_->stream.async_read(impl_->buffer, boost::cobalt::use_op);
        std::string data = boost::beast::buffers_to_string(impl_->buffer.data());
        co_return result<std::string>::success(std::move(data));
    }
    catch (const boost::system::system_error& e) {
        co_return result<std::string>::failure({ error_code::websocket, 0, 0, e.what(), {} });
    }
}

boost::cobalt::task<result<void>>
websocket_client::async_close()
{
    if (!impl_->connected) {
        co_return result<void>::success();
    }

    try {
        co_await impl_->stream.async_close(boost::beast::websocket::close_code::normal, boost::cobalt::use_op);
        impl_->connected = false;
        co_return result<void>::success();
    }
    catch (const boost::system::system_error& e) {
        impl_->connected = false;
        co_return result<void>::failure({ error_code::websocket, 0, 0, e.what(), {} });
    }
}

// --- Sync (wraps async) ---

result<void>
websocket_client::connect(const std::string& host, const std::string& port, const std::string& target)
{
    return boost::cobalt::run(async_connect(host, port, target));
}

result<void>
websocket_client::write_text(const std::string& message)
{
    return boost::cobalt::run(async_write_text(message));
}

result<std::string>
websocket_client::read_text()
{
    return boost::cobalt::run(async_read_text());
}

result<void>
websocket_client::run_read_loop(message_handler handler)
{
    while (true) {
        auto message = read_text();
        if (!message) {
            return result<void>::failure(message.err);
        }
        if (!handler(*message)) {
            break;
        }
    }
    return result<void>::success();
}

result<void>
websocket_client::close()
{
    return boost::cobalt::run(async_close());
}

} // namespace binapi2::fapi::transport
