// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/transport/http_client.hpp>

#include <binapi2/fapi/error.hpp>
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

namespace binapi2::fapi::transport {

http_client::http_client(boost::asio::io_context& io_context, config cfg) :
    session_base(std::move(cfg)), io_context_(io_context)
{
}

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
        asio::ssl::context ssl_ctx = make_ssl_context();
        asio::ip::tcp::resolver resolver{ io_context_ };
        asio::ssl::stream<asio::ip::tcp::socket> stream{ io_context_, ssl_ctx };

        if (!SSL_set_tlsext_host_name(stream.native_handle(), cfg_.rest_host.c_str())) {
            co_return result<http_response>::failure(
                { error_code::transport, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
        }

        auto endpoints = co_await resolver.async_resolve(cfg_.rest_host, cfg_.rest_port, boost::cobalt::use_op);

        co_await asio::async_connect(stream.next_layer(), endpoints, boost::cobalt::use_op);

        co_await stream.async_handshake(asio::ssl::stream_base::client, boost::cobalt::use_op);

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

        co_await http::async_write(stream, req, boost::cobalt::use_op);

        beast::flat_buffer buffer;
        http::response<http::string_body> response;
        co_await http::async_read(stream, buffer, response, boost::cobalt::use_op);

        boost::system::error_code shutdown_ec;
        stream.shutdown(shutdown_ec);
        // Ignore shutdown errors (common with HTTP/1.1)

        co_return result<http_response>::success({ static_cast<int>(response.result_int()), response.body() });
    }
    catch (const boost::system::system_error& e) {
        co_return result<http_response>::failure({ error_code::transport, 0, 0, e.what(), {} });
    }
}

result<http_response>
http_client::request(boost::beast::http::verb method,
                     const std::string& target,
                     const std::string& body,
                     const std::string& content_type,
                     const std::string& api_key)
{
    return boost::cobalt::run(async_request(method, target, body, content_type, api_key));
}

} // namespace binapi2::fapi::transport
