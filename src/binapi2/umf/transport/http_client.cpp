#include <binapi2/umf/transport/http_client.hpp>

#include <binapi2/umf/error.hpp>
#include <binapi2/umf/transport/ssl_context.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <openssl/err.h>

namespace binapi2::umf::transport {

http_client::http_client(boost::asio::io_context& io_context, config cfg) :
    session_base(std::move(cfg)), io_context_(io_context)
{
}

result<http_response>
http_client::request(boost::beast::http::verb method,
                     const std::string& target,
                     const std::string& body,
                     const std::string& content_type,
                     const std::string& api_key)
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;

    asio::ssl::context ssl_ctx = make_ssl_context();
    asio::ip::tcp::resolver resolver{ io_context_ };
    asio::ssl::stream<asio::ip::tcp::socket> stream{ io_context_, ssl_ctx };

    if (!SSL_set_tlsext_host_name(stream.native_handle(), cfg_.rest_host.c_str())) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {} });
    }

    boost::system::error_code ec;
    auto endpoints = resolver.resolve(cfg_.rest_host, cfg_.rest_port, ec);
    if (ec) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ec.message(), {} });
    }

    asio::connect(stream.next_layer(), endpoints, ec);
    if (ec) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ec.message(), {} });
    }

    stream.handshake(asio::ssl::stream_base::client, ec);
    if (ec) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ec.message(), {} });
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
        req.body() = body;
        req.prepare_payload();
    }

    http::write(stream, req, ec);
    if (ec) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ec.message(), {} });
    }

    beast::flat_buffer buffer;
    http::response<http::string_body> response;
    http::read(stream, buffer, response, ec);
    if (ec) {
        return result<http_response>::failure({ error_code::transport, 0, 0, ec.message(), {} });
    }

    stream.shutdown(ec);

    return result<http_response>::success({ static_cast<int>(response.result_int()), response.body() });
}

void
http_client::async_request(boost::beast::http::verb method,
                           std::string target,
                           std::string body,
                           std::string content_type,
                           std::string api_key,
                           callback_type callback)
{
    boost::asio::post(
        io_context_,
        [this,
         method,
         target = std::move(target),
         body = std::move(body),
         content_type = std::move(content_type),
         api_key = std::move(api_key),
         callback = std::move(callback)]() mutable { callback(request(method, target, body, content_type, api_key)); });
}

} // namespace binapi2::umf::transport
