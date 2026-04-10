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

#include <memory>
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
/// Maintains a persistent TLS connection to rest_host:rest_port and reuses
/// it across requests (HTTP/1.1 keep-alive). If the connection drops or
/// a request fails, the client reconnects transparently and retries once.
///
/// The SSL context and resolver are created once. The TCP+TLS stream is
/// established on the first request and reused for subsequent requests.
class http_client final : public session_base
{
public:
    explicit http_client(config cfg);
    ~http_client();

    /// @brief Establish the TLS connection. If already connected, no-op.
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect();

    [[nodiscard]] boost::cobalt::task<result<http_response>>
    async_request(boost::beast::http::verb method,
                  std::string target,
                  std::string body,
                  std::string content_type,
                  std::string api_key);

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace binapi2::fapi::transport
