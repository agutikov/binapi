// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/query.hpp>
#include <binapi2/fapi/rest/account.hpp>
#include <binapi2/fapi/rest/convert.hpp>
#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/rest/market_data.hpp>
#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/rest/trade.hpp>
#include <binapi2/fapi/rest/user_data_streams.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>
#include <binapi2/fapi/transport/http_client.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace binapi2::fapi {

namespace detail {

template<typename T>
result<T>
decode_response(const transport::http_response& response)
{
    if (response.status < 200 || response.status >= 300) {
        types::binance_error_document error_doc{};
        if (!glz::read_json(error_doc, response.body)) {
            return result<T>::failure({ error_code::binance, response.status, error_doc.code, error_doc.msg, response.body });
        }
        return result<T>::failure({ error_code::http_status, response.status, 0, "HTTP request failed", response.body });
    }

    if constexpr (std::is_same_v<T, types::empty_response>) {
        return result<T>::success({});
    } else {
        T value{};
        if (auto ec = glz::read_json(value, response.body)) {
            return result<T>::failure(
                { error_code::json, response.status, 0, glz::format_error(ec, response.body), response.body });
        }
        return result<T>::success(std::move(value));
    }
}

} // namespace detail

class client
{
public:
    client(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] config& configuration() noexcept;
    [[nodiscard]] const config& configuration() const noexcept;
    [[nodiscard]] boost::asio::io_context& context() noexcept;
    [[nodiscard]] transport::http_client& transport() noexcept;

    // Low-level sync execute.
    template<typename Response>
    [[nodiscard]] result<Response> execute(boost::beast::http::verb method,
                                           const std::string& path,
                                           const query_map& query,
                                           bool signed_request)
    {
        auto response = prepare_and_send(method, path, query, signed_request);
        if (!response) {
            return result<Response>::failure(response.err);
        }
        return detail::decode_response<Response>(*response);
    }

    // Low-level async execute.
    template<typename Response>
    [[nodiscard]] boost::cobalt::task<result<Response>> async_execute(boost::beast::http::verb method,
                                                                      std::string path,
                                                                      query_map query,
                                                                      bool signed_request)
    {
        auto response = co_await async_prepare_and_send(method, std::move(path), std::move(query), signed_request);
        if (!response) {
            co_return result<Response>::failure(response.err);
        }
        co_return detail::decode_response<Response>(*response);
    }

    // Generic sync execute: derives endpoint and response type from request type.
    template<class Request>
        requires rest::has_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request)
    {
        using traits = rest::endpoint_traits<Request>;
        return execute<typename traits::response_type>(
            traits::endpoint.method, std::string{ traits::endpoint.path }, to_query_map(request), traits::endpoint.signed_request);
    }

    // Generic async execute.
    template<class Request>
        requires rest::has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
    {
        using traits = rest::endpoint_traits<Request>;
        return async_execute<typename traits::response_type>(
            traits::endpoint.method,
            std::string{ traits::endpoint.path },
            to_query_map(request),
            traits::endpoint.signed_request);
    }

    rest::account_service account;
    rest::convert_service convert;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::http_client http_;

    // Shared query preparation + transport call.
    result<transport::http_response> prepare_and_send(boost::beast::http::verb method,
                                                      const std::string& path,
                                                      const query_map& query,
                                                      bool signed_request);

    boost::cobalt::task<result<transport::http_response>> async_prepare_and_send(boost::beast::http::verb method,
                                                                                  std::string path,
                                                                                  query_map query,
                                                                                  bool signed_request);
};

// Template definitions for rest::service (needs full client definition).
template<class Request>
    requires rest::has_endpoint_traits<Request>
auto
rest::service::execute(const Request& request) -> result<typename rest::endpoint_traits<Request>::response_type>
{
    return owner_.execute(request);
}

template<class Request>
    requires rest::has_endpoint_traits<Request>
auto
rest::service::async_execute(const Request& request)
    -> boost::cobalt::task<result<typename rest::endpoint_traits<Request>::response_type>>
{
    co_return co_await owner_.async_execute(request);
}

} // namespace binapi2::fapi
