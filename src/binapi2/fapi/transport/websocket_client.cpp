// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the WebSocket transport using the pimpl idiom to hide
/// Beast/ASIO types from the header. The async_* methods are the primary
/// implementations (coroutine-based); the sync wrappers drive them to
/// completion via io_thread::run_sync. A control_callback is installed during
/// connect to automatically reply with a pong frame whenever a ping is
/// received, satisfying the Binance WebSocket keepalive requirement.

#include <binapi2/fapi/transport/websocket_client.hpp>

#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/transport_logger.hpp>
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

// Pimpl body. Owns the SSL context, resolver, Beast WebSocket stream, and
// read buffer. The SSL context is created once at construction time (unlike
// the HTTP client which creates one per request) because the WebSocket
// connection is long-lived.  The resolver and stream are created lazily in
// async_connect using the coroutine's executor so that io_thread::run_sync()
// can drive completions on its persistent io_context.
struct websocket_client::impl
{
    explicit impl(config cfg) :
        ssl_ctx(make_ssl_context(cfg.ca_cert_file)), cfg(std::move(cfg))
    {
    }

    using ws_stream = boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

    boost::asio::ssl::context ssl_ctx;
    std::optional<boost::asio::ip::tcp::resolver> resolver;
    std::optional<ws_stream> stream;
    boost::beast::flat_buffer buffer;
    config cfg;
    bool connected{ false };
};

websocket_client::websocket_client(config cfg) :
    session_base(cfg), impl_(std::make_unique<impl>(std::move(cfg)))
{
}

websocket_client::~websocket_client() = default;

// --- Async (primary) ---

boost::cobalt::task<result<void>>
websocket_client::async_connect(std::string host, std::string port, std::string target)
{
    try {
        // Create I/O objects on the coroutine's executor so that io_thread::run_sync()
        // (used by sync wrappers) can drive their completions.
        auto executor = co_await boost::cobalt::this_coro::executor;
        impl_->resolver.emplace(executor);
        impl_->stream.emplace(executor, impl_->ssl_ctx);

        if (!SSL_set_tlsext_host_name(impl_->stream->next_layer().native_handle(), host.c_str())) {
            co_return result<void>::failure(
                { error_code::websocket, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "resolve",
                          host + ":" + port, 0, {}, {} });
        }
        auto endpoints = co_await impl_->resolver->async_resolve(host, port, boost::cobalt::use_op);
        if (cfg_.logger) {
            std::string ep_list;
            for (const auto& ep : endpoints) {
                if (!ep_list.empty()) ep_list += ", ";
                ep_list += ep.endpoint().address().to_string() + ":" + std::to_string(ep.endpoint().port());
            }
            cfg_.logger({ transport_direction::received, "CONN", "resolve",
                          host, 0, ep_list, {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "tcp_connect",
                          host + ":" + port, 0, {}, {} });
        }
        co_await boost::asio::async_connect(impl_->stream->next_layer().next_layer(), endpoints, boost::cobalt::use_op);
        if (cfg_.logger) {
            auto& sock = impl_->stream->next_layer().next_layer();
            cfg_.logger({ transport_direction::received, "CONN", "tcp_connect",
                          sock.remote_endpoint().address().to_string() + ":"
                              + std::to_string(sock.remote_endpoint().port()),
                          0, {}, {} });
        }

        // Install a ping/pong handler. Binance sends periodic pings and
        // expects a pong within a timeout window; failure to respond causes
        // the server to drop the connection. The pong is sent synchronously
        // here because we are inside a control callback where async writes
        // are not permitted by Beast.
        impl_->stream->control_callback([this](boost::beast::websocket::frame_type kind, boost::beast::string_view) {
            if (kind == boost::beast::websocket::frame_type::ping) {
                boost::system::error_code ignored;
                impl_->stream->pong({}, ignored);
            }
        });

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "ssl_handshake",
                          host, 0, {}, {} });
        }
        co_await impl_->stream->next_layer().async_handshake(
            boost::asio::ssl::stream_base::client, boost::cobalt::use_op);
        if (cfg_.logger) {
            cfg_.logger({ transport_direction::received, "CONN", "ssl_handshake",
                          SSL_get_version(impl_->stream->next_layer().native_handle()), 0, {}, {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "ws_handshake",
                          host + target, 0, {}, {} });
        }
        co_await impl_->stream->async_handshake(host, target, boost::cobalt::use_op);
        if (cfg_.logger) {
            cfg_.logger({ transport_direction::received, "CONN", "ws_handshake",
                          host + target, 0, {}, {} });
        }

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
        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "WS", "WS", "", 0, message, {} });
        }
        co_await impl_->stream->async_write(boost::asio::buffer(message), boost::cobalt::use_op);
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
        // Drain the buffer before each read to prevent stale data from
        // accumulating across successive reads on the same flat_buffer.
        impl_->buffer.consume(impl_->buffer.size());
        co_await impl_->stream->async_read(impl_->buffer, boost::cobalt::use_op);
        std::string data = boost::beast::buffers_to_string(impl_->buffer.data());
        if (cfg_.stream_recorder) {
            cfg_.stream_recorder(data);
        }
        if (cfg_.logger) {
            cfg_.logger({ transport_direction::received, "WS", "WS", "", 0, data, {} });
        }
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
        co_await impl_->stream->async_close(boost::beast::websocket::close_code::normal, boost::cobalt::use_op);
        impl_->connected = false;
        co_return result<void>::success();
    }
    catch (const boost::system::system_error& e) {
        impl_->connected = false;
        co_return result<void>::failure({ error_code::websocket, 0, 0, e.what(), {} });
    }
}

} // namespace binapi2::fapi::transport
