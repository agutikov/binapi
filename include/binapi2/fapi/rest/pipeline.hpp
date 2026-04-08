// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file pipeline.hpp
/// @brief REST request pipeline: signing, query encoding, HTTP transport, response decoding.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/decode.hpp>
#include <binapi2/fapi/query.hpp>
#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>
#include <binapi2/fapi/transport/http_client.hpp>

#include <boost/beast/http/verb.hpp>
#include <boost/cobalt/task.hpp>

#include <string>

namespace binapi2::fapi::rest {

/// @brief Async REST request pipeline.
///
/// Combines signing, query encoding, HTTP transport, and JSON response decoding
/// into a single async_execute coroutine. This is the primary (and only) execution
/// path for REST API calls.
///
/// Does not own an executor — the coroutine runs on whatever executor drives it.
class pipeline
{
public:
    pipeline(const config& cfg, transport::http_client& http);

    /// @brief Low-level async execute: caller provides HTTP verb, path, query, signing flag.
    template<typename Response>
    [[nodiscard]] boost::cobalt::task<result<Response>>
    async_execute(boost::beast::http::verb method, std::string path,
                  query_map query, bool signed_request);

    /// @brief Generic async execute: request struct → endpoint_traits → low-level async_execute.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        using traits = endpoint_traits<Request>;
        return async_execute<typename traits::response_type_t>(
            traits::endpoint.method,
            std::string{ traits::endpoint.path },
            to_query_map(request),
            traits::endpoint.signed_request);
    }

    /// @brief Access the underlying config.
    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }

private:
    const config& cfg_;
    transport::http_client& http_;
};

// --- Template implementation ---

template<typename Response>
boost::cobalt::task<result<Response>>
pipeline::async_execute(boost::beast::http::verb method, std::string path,
                        query_map query, bool signed_request)
{
    query_map final_query = std::move(query);
    if (signed_request) {
        inject_auth_query(final_query, cfg_.recv_window, current_timestamp_ms());
        sign_query(final_query, cfg_.secret_key);
    }

    const auto query_string = build_query_string(final_query);
    std::string target = cfg_.rest_base_path + path;
    std::string body;
    if (!query_string.empty()) {
        if (method == boost::beast::http::verb::get || method == boost::beast::http::verb::delete_) {
            target += "?" + query_string;
        } else {
            body = query_string;
        }
    }

    auto response = co_await http_.async_request(
        method, std::move(target), std::move(body), "application/x-www-form-urlencoded", cfg_.api_key);
    if (!response) {
        co_return result<Response>::failure(response.err);
    }
    co_return detail::decode_response<Response>(*response);
}

} // namespace binapi2::fapi::rest
