// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the HTTP transport with persistent connection reuse.
///
/// The SSL context is created once at construction. The TCP+TLS stream is
/// established lazily on the first request and reused for subsequent ones
/// (HTTP/1.1 keep-alive). On write/read failure the client reconnects and
/// retries the request once.

#include <binapi2/fapi/transport/http_client.hpp>

#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/transport_logger.hpp>
#include <binapi2/fapi/transport/ssl_context.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/cobalt/op.hpp>

#include <openssl/err.h>

#include <sstream>

namespace binapi2::fapi::transport {

// Serialize a Beast HTTP message (request or response) to a string
// including the start line and all headers.
template<bool IsRequest, class Body, class Fields>
static std::string
serialize_message(const boost::beast::http::message<IsRequest, Body, Fields>& msg)
{
    std::ostringstream oss;
    oss << msg;
    return oss.str();
}

struct http_client::impl
{
    explicit impl(config cfg) :
        ssl_ctx(make_ssl_context(cfg.ca_cert_file)), cfg(std::move(cfg))
    {
    }

    boost::asio::ssl::context ssl_ctx;
    std::optional<boost::asio::ip::tcp::resolver> resolver;
    std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> stream;
    bool connected{ false };
    config cfg;

    boost::cobalt::task<result<void>> async_connect();
    void disconnect();
    boost::cobalt::task<result<http_response>> async_send_receive(
        boost::beast::http::verb method, const std::string& target,
        const std::string& body, const std::string& content_type,
        const std::string& api_key);
};

http_client::http_client(config cfg) :
    session_base(cfg), impl_(std::make_unique<impl>(std::move(cfg)))
{
}

http_client::~http_client() = default;

// Establish a fresh TCP+TLS connection.
boost::cobalt::task<result<void>>
http_client::impl::async_connect()
{
    namespace asio = boost::asio;

    try {
        auto executor = co_await boost::cobalt::this_coro::executor;
        resolver.emplace(executor);
        stream.emplace(executor, ssl_ctx);

        if (!SSL_set_tlsext_host_name(stream->native_handle(), cfg.rest_host.c_str())) {
            co_return result<void>::failure(
                { error_code::transport, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
        }

        if (cfg.logger) {
            cfg.logger({ transport_direction::sent, "CONN", "resolve",
                         cfg.rest_host + ":" + cfg.rest_port, 0, {}, {} });
        }
        auto endpoints = co_await resolver->async_resolve(
            cfg.rest_host, cfg.rest_port, boost::cobalt::use_op);
        if (cfg.logger) {
            std::string ep_list;
            for (const auto& ep : endpoints) {
                if (!ep_list.empty()) ep_list += ", ";
                ep_list += ep.endpoint().address().to_string() + ":" + std::to_string(ep.endpoint().port());
            }
            cfg.logger({ transport_direction::received, "CONN", "resolve",
                         cfg.rest_host, 0, ep_list, {} });
        }

        if (cfg.logger) {
            cfg.logger({ transport_direction::sent, "CONN", "tcp_connect",
                         cfg.rest_host + ":" + cfg.rest_port, 0, {}, {} });
        }
        co_await asio::async_connect(stream->next_layer(), endpoints, boost::cobalt::use_op);
        if (cfg.logger) {
            auto& sock = stream->next_layer();
            cfg.logger({ transport_direction::received, "CONN", "tcp_connect",
                         sock.remote_endpoint().address().to_string() + ":"
                             + std::to_string(sock.remote_endpoint().port()),
                         0, {}, {} });
        }

        if (cfg.logger) {
            cfg.logger({ transport_direction::sent, "CONN", "ssl_handshake",
                         cfg.rest_host, 0, {}, {} });
        }
        co_await stream->async_handshake(asio::ssl::stream_base::client, boost::cobalt::use_op);
        if (cfg.logger) {
            cfg.logger({ transport_direction::received, "CONN", "ssl_handshake",
                         SSL_get_version(stream->native_handle()), 0, {}, {} });
        }

        connected = true;
        co_return result<void>::success();
    }
    catch (const boost::system::system_error& e) {
        connected = false;
        co_return result<void>::failure({ error_code::transport, 0, 0, e.what(), {} });
    }
}

void http_client::impl::disconnect()
{
    if (stream) {
        boost::system::error_code ec;
        stream->shutdown(ec);
    }
    stream.reset();
    resolver.reset();
    connected = false;
}

boost::cobalt::task<result<http_response>>
http_client::impl::async_send_receive(boost::beast::http::verb method,
                                      const std::string& target,
                                      const std::string& body,
                                      const std::string& content_type,
                                      const std::string& api_key)
{
    namespace beast = boost::beast;
    namespace http = beast::http;

    http::request<http::string_body> req{ method, target, 11 };
    req.set(http::field::host, cfg.rest_host);
    req.set(http::field::user_agent, cfg.user_agent);
    req.set(http::field::connection, "keep-alive");
    if (!content_type.empty()) {
        req.set(http::field::content_type, content_type);
    }
    if (!api_key.empty()) {
        req.set("X-MBX-APIKEY", api_key);
    }
    if (method != http::verb::get && method != http::verb::delete_) {
        req.body() = body;
        req.prepare_payload();
    }

    if (cfg.logger) {
        cfg.logger({ transport_direction::sent, "HTTP",
                     std::string(http::to_string(method)), target, 0,
                     req.body(), serialize_message(req) });
    }

    co_await http::async_write(*stream, req, boost::cobalt::use_op);

    beast::flat_buffer buffer;
    http::response<http::string_body> response;
    co_await http::async_read(*stream, buffer, response, boost::cobalt::use_op);

    if (cfg.logger) {
        cfg.logger({ transport_direction::received, "HTTP",
                     std::string(http::to_string(method)), target,
                     static_cast<int>(response.result_int()),
                     response.body(), serialize_message(response) });
    }

    co_return result<http_response>::success(
        { static_cast<int>(response.result_int()), response.body() });
}

boost::cobalt::task<result<void>>
http_client::async_connect()
{
    if (impl_->connected) co_return result<void>::success();
    co_return co_await impl_->async_connect();
}

boost::cobalt::task<result<http_response>>
http_client::async_request(boost::beast::http::verb method,
                           std::string target,
                           std::string body,
                           std::string content_type,
                           std::string api_key)
{
    if (!impl_->connected) {
        co_return result<http_response>::failure(
            { error_code::transport, 0, 0, "not connected — call async_connect() first", {} });
    }

    // Try to send the request on the existing connection.
    try {
        co_return co_await impl_->async_send_receive(method, target, body, content_type, api_key);
    }
    catch (const boost::system::system_error&) {
        // Connection stale — reconnect and retry once.
    }

    impl_->disconnect();
    auto conn = co_await impl_->async_connect();
    if (!conn) co_return result<http_response>::failure(conn.err);

    try {
        co_return co_await impl_->async_send_receive(method, target, body, content_type, api_key);
    }
    catch (const boost::system::system_error& e) {
        impl_->disconnect();
        co_return result<http_response>::failure({ error_code::transport, 0, 0, e.what(), {} });
    }
}

} // namespace binapi2::fapi::transport
