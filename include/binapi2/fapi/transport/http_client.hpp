// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file http_client.hpp
/// @brief Async HTTP client for the Binance USD-M Futures REST API.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/session_base.hpp>

#include <boost/beast/http/verb.hpp>
#include <boost/cobalt/task.hpp>

#include <string>

namespace binapi2::fapi::transport {

/// @brief Raw HTTP response returned by the transport layer.
struct http_response
{
    int status{ 0 };       ///< HTTP status code (e.g. 200, 400, 429).
    std::string body{};    ///< Response body as raw text (typically JSON).
};

/// @brief Async HTTP client for the Binance Futures REST API.
///
/// Each call to async_request opens a fresh TLS connection, sends the request,
/// reads the response, and closes. The coroutine runs on whatever executor
/// drives it (via co_await this_coro::executor).
class http_client final : public session_base
{
public:
    explicit http_client(config cfg);

    [[nodiscard]] boost::cobalt::task<result<http_response>>
    async_request(boost::beast::http::verb method,
                  std::string target,
                  std::string body,
                  std::string content_type,
                  std::string api_key);
};

} // namespace binapi2::fapi::transport
