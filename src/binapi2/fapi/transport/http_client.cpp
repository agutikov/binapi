// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the HTTP transport for REST API calls. Each request opens a
/// fresh TLS connection (resolve -> TCP connect -> SSL handshake -> write ->
/// read -> shutdown). The primary implementation is the coroutine-based
/// async_request; the synchronous request() is a thin wrapper that drives the
/// coroutine to completion via io_thread::run_sync.

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
#include <boost/cobalt/run.hpp>

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
    // Beast's http::serializer can write the header portion.
    // For simplicity use the operator<< overload which writes the full message.
    oss << msg;
    return oss.str();
}

http_client::http_client(detail::io_thread& io, config cfg) :
    session_base(std::move(cfg)), io_(&io)
{
}

http_client::http_client(config cfg) :
    session_base(std::move(cfg))
{
}

// Coroutine-based async HTTP request. Each call creates its own SSL context,
// resolver, and socket so requests are independent and thread-safe.
// Parameters are taken by value to ensure they survive across suspension
// points. The X-MBX-APIKEY header is required by Binance for authenticated
// endpoints. SSL shutdown errors are intentionally ignored -- truncated
// closes are common with HTTP/1.1 servers and do not affect the response.
boost::cobalt::task<result<http_response>>
http_client::async_request(boost::beast::http::verb method,
                           std::string target,
                           std::string body,
                           std::string content_type,
                           std::string api_key)
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;

    try {
        // Use the coroutine's own executor for all I/O objects.  This is
        // Use the coroutine's own executor for all I/O objects so their
        // completions are dispatched by the io_thread's io_context.
        auto executor = co_await boost::cobalt::this_coro::executor;
        asio::ssl::context ssl_ctx = make_ssl_context(cfg_.ca_cert_file);
        asio::ip::tcp::resolver resolver{ executor };
        asio::ssl::stream<asio::ip::tcp::socket> stream{ executor, ssl_ctx };

        if (!SSL_set_tlsext_host_name(stream.native_handle(), cfg_.rest_host.c_str())) {
            co_return result<http_response>::failure(
                { error_code::transport, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "resolve",
                          cfg_.rest_host + ":" + cfg_.rest_port, 0, {}, {} });
        }
        auto endpoints = co_await resolver.async_resolve(cfg_.rest_host, cfg_.rest_port, boost::cobalt::use_op);

        if (cfg_.logger) {
            std::string ep_list;
            for (const auto& ep : endpoints) {
                if (!ep_list.empty()) ep_list += ", ";
                ep_list += ep.endpoint().address().to_string() + ":" + std::to_string(ep.endpoint().port());
            }
            cfg_.logger({ transport_direction::received, "CONN", "resolve",
                          cfg_.rest_host, 0, ep_list, {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "tcp_connect",
                          cfg_.rest_host + ":" + cfg_.rest_port, 0, {}, {} });
        }
        co_await asio::async_connect(stream.next_layer(), endpoints, boost::cobalt::use_op);
        if (cfg_.logger) {
            auto& sock = stream.next_layer();
            cfg_.logger({ transport_direction::received, "CONN", "tcp_connect",
                          sock.remote_endpoint().address().to_string() + ":"
                              + std::to_string(sock.remote_endpoint().port()),
                          0, {}, {} });
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "CONN", "ssl_handshake",
                          cfg_.rest_host, 0, {}, {} });
        }
        co_await stream.async_handshake(asio::ssl::stream_base::client, boost::cobalt::use_op);
        if (cfg_.logger) {
            cfg_.logger({ transport_direction::received, "CONN", "ssl_handshake",
                          SSL_get_version(stream.native_handle()), 0, {}, {} });
        }

        http::request<http::string_body> req{ method, target, 11 };
        req.set(http::field::host, cfg_.rest_host);
        req.set(http::field::user_agent, cfg_.user_agent);
        if (!content_type.empty()) {
            req.set(http::field::content_type, content_type);
        }
        if (!api_key.empty()) {
            req.set("X-MBX-APIKEY", api_key);
        }
        if (method != http::verb::get && method != http::verb::delete_) {
            req.body() = std::move(body);
            req.prepare_payload();
        }

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::sent, "HTTP",
                          std::string(http::to_string(method)), target, 0,
                          req.body(), serialize_message(req) });
        }

        co_await http::async_write(stream, req, boost::cobalt::use_op);

        beast::flat_buffer buffer;
        http::response<http::string_body> response;
        co_await http::async_read(stream, buffer, response, boost::cobalt::use_op);

        if (cfg_.logger) {
            cfg_.logger({ transport_direction::received, "HTTP",
                          std::string(http::to_string(method)), target,
                          static_cast<int>(response.result_int()),
                          response.body(), serialize_message(response) });
        }

        boost::system::error_code shutdown_ec;
        stream.shutdown(shutdown_ec);
        // Ignore shutdown errors (common with HTTP/1.1)

        co_return result<http_response>::success({ static_cast<int>(response.result_int()), response.body() });
    }
    catch (const boost::system::system_error& e) {
        co_return result<http_response>::failure({ error_code::transport, 0, 0, e.what(), {} });
    }
}

// Synchronous wrapper: posts the async coroutine to the io_thread and
// blocks until it completes. In async-only mode (no io_thread), returns an error.
result<http_response>
http_client::request(boost::beast::http::verb method,
                     const std::string& target,
                     const std::string& body,
                     const std::string& content_type,
                     const std::string& api_key)
{
    if (!io_) return result<http_response>::failure({ error_code::internal, 0, 0, "sync methods require io_thread", {} });
    return io_->run_sync(async_request(method, target, body, content_type, api_key));
}

} // namespace binapi2::fapi::transport
