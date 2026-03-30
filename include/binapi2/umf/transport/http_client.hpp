#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/transport/session_base.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>

#include <functional>
#include <string>

namespace binapi2::umf::transport {

struct http_response {
    int status{0};
    std::string body{};
};

class http_client final : public session_base {
  public:
    http_client(boost::asio::io_context &io_context, config cfg);

    [[nodiscard]] result<http_response> request(
        boost::beast::http::verb method,
        const std::string &target,
        const std::string &body,
        const std::string &content_type,
        const std::string &api_key
    );

    using callback_type = std::function<void(result<http_response>)>;
    void async_request(
        boost::beast::http::verb method,
        std::string target,
        std::string body,
        std::string content_type,
        std::string api_key,
        callback_type callback
    );

  private:
    boost::asio::io_context &io_context_;
};

} // namespace binapi2::umf::transport
