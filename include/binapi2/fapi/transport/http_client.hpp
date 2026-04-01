// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file http_client.hpp
/// @brief Asynchronous-primary HTTP client for the Binance USD-M Futures REST API.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/session_base.hpp>

#include <boost/asio/io_context.hpp>
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

/// @brief HTTP client for the Binance Futures REST API.
///
/// This client follows an async-primary design: the coroutine-based
/// @ref async_request is the primary implementation, while @ref request is a
/// synchronous convenience wrapper that drives the io_context to completion.
class http_client final : public session_base
{
public:
    /// @brief Construct an HTTP client.
    /// @param io_context Boost.Asio I/O context used for async operations.
    /// @param cfg        Configuration containing the REST endpoint URL and credentials.
    http_client(boost::asio::io_context& io_context, config cfg);

    /// @brief Send an HTTP request asynchronously (primary implementation).
    ///
    /// Establishes a TLS connection, sends the request, and co_returns the
    /// response. This is a Boost.Cobalt coroutine task.
    ///
    /// @param method       HTTP verb (GET, POST, PUT, DELETE, etc.).
    /// @param target       Request target path including query string.
    /// @param body         Request body (empty for GET requests).
    /// @param content_type MIME content type for the request body.
    /// @param api_key      Binance API key sent in the X-MBX-APIKEY header.
    /// @return A cobalt task yielding a result containing the HTTP response.
    [[nodiscard]] boost::cobalt::task<result<http_response>> async_request(boost::beast::http::verb method,
                                                                           std::string target,
                                                                           std::string body,
                                                                           std::string content_type,
                                                                           std::string api_key);

    /// @brief Send an HTTP request synchronously (wraps @ref async_request).
    ///
    /// Blocks the calling thread by running the io_context until the async
    /// operation completes.
    ///
    /// @param method       HTTP verb (GET, POST, PUT, DELETE, etc.).
    /// @param target       Request target path including query string.
    /// @param body         Request body (empty for GET requests).
    /// @param content_type MIME content type for the request body.
    /// @param api_key      Binance API key sent in the X-MBX-APIKEY header.
    /// @return A result containing the HTTP response or an error.
    [[nodiscard]] result<http_response> request(boost::beast::http::verb method,
                                                const std::string& target,
                                                const std::string& body,
                                                const std::string& content_type,
                                                const std::string& api_key);

private:
    boost::asio::io_context& io_context_;
};

} // namespace binapi2::fapi::transport
