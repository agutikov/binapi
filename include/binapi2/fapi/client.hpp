// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Top-level USD-M Futures API client, combining all REST service groups
///        and providing both synchronous and asynchronous (boost::cobalt) execution.

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

/// JSON parse options:
///  - error_on_unknown_keys = false: skip unknown keys so that new fields
///    added by Binance don't break deserialization of existing response types.
///  - error_on_missing_keys = true: require all non-optional fields to be
///    present in the JSON, catching silent zero/empty defaults on renamed or
///    removed fields.
inline constexpr glz::opts json_read_opts{
    .error_on_unknown_keys = false,
    .error_on_missing_keys = true,
};

/// @brief Decode an HTTP response body into a typed result.
///
/// Checks the HTTP status code first; on non-2xx responses, attempts to parse
/// a Binance error document from the body. On success, deserializes the body
/// as JSON into @p T using glaze.
///
/// For @c types::empty_response_t, skips JSON parsing and returns an empty value.
///
/// @tparam T  The response type to deserialize into. Must be default-constructible
///            and supported by glz::read_json, or types::empty_response_t.
/// @param response  The raw HTTP response from the transport layer.
/// @return A result containing either the deserialized value or an error with
///         error_code::binance, error_code::http_status, or error_code::json.
template<typename T>
result<T>
decode_response(const transport::http_response& response)
{
    if (response.status < 200 || response.status >= 300) {
        types::binance_error_document_t error_doc{};
        glz::context ctx{};
        if (!glz::read<json_read_opts>(error_doc, response.body, ctx)) {
            return result<T>::failure({ error_code::binance, response.status, error_doc.code, error_doc.msg, response.body });
        }
        return result<T>::failure({ error_code::http_status, response.status, 0, "HTTP request failed", response.body });
    }

    if constexpr (std::is_same_v<T, types::empty_response_t>) {
        return result<T>::success({});
    } else {
        T value{};
        glz::context ctx{};
        if (auto ec = glz::read<json_read_opts>(value, response.body, ctx)) {
            return result<T>::failure(
                { error_code::json, response.status, 0, glz::format_error(ec, response.body), response.body });
        }
        return result<T>::success(std::move(value));
    }
}

} // namespace detail

/// @brief USD-M Futures API client.
///
/// Owns the HTTP transport and configuration, and exposes domain-specific
/// service objects (market_data, trade, account, convert, user_data_streams)
/// for organized access to Binance endpoints.
///
/// All network I/O is implemented as boost::cobalt coroutines (async_execute).
/// The synchronous execute methods wrap the coroutine via cobalt::run and block
/// until completion, so they must not be called from within a coroutine context.
///
/// Two execution styles are available:
///   - **Low-level**: caller specifies HTTP verb, path, query map, and signing flag.
///   - **Generic**: caller passes a request struct; endpoint metadata and response
///     type are resolved automatically via endpoint_traits.
class client
{
public:
    client(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] config& configuration() noexcept;
    [[nodiscard]] const config& configuration() const noexcept;
    [[nodiscard]] boost::asio::io_context& context() noexcept;
    [[nodiscard]] transport::http_client& transport() noexcept;

    /// @brief Low-level synchronous request execution.
    ///
    /// Prepares the query (signing, timestamp injection), sends the HTTP request,
    /// and deserializes the response body into @p Response.
    /// Blocks the calling thread; must not be called from a coroutine.
    ///
    /// @tparam Response     The type to deserialize the response body into.
    /// @param method        HTTP verb (GET, POST, PUT, DELETE).
    /// @param path          API endpoint path (e.g. "/fapi/v1/order").
    /// @param query         Query parameters as key-value pairs.
    /// @param signed_request  Whether to HMAC-sign the request.
    /// @return Typed result with the deserialized response or an error.
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

    /// @brief Low-level asynchronous request execution (boost::cobalt coroutine).
    ///
    /// Same behavior as the synchronous overload, but suspends rather than blocks.
    /// This is the primary execution path; the synchronous variant delegates here.
    ///
    /// @tparam Response     The type to deserialize the response body into.
    /// @param method        HTTP verb.
    /// @param path          API endpoint path (taken by value for coroutine safety).
    /// @param query         Query parameters (taken by value for coroutine safety).
    /// @param signed_request  Whether to HMAC-sign the request.
    /// @return Typed result with the deserialized response or an error.
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

    /// @brief Generic synchronous execute that derives endpoint metadata from the request type.
    ///
    /// Looks up the HTTP method, path, signing requirement, and response type
    /// via endpoint_traits<Request>, then delegates to the low-level execute.
    ///
    /// @tparam Request  A request struct for which endpoint_traits is specialized.
    ///                  Must satisfy the has_endpoint_traits concept.
    /// @param request   The populated request struct; converted to a query_map via to_query_map.
    /// @return Typed result with the endpoint's associated response type.
    template<class Request>
        requires rest::has_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request)
    {
        using traits = rest::endpoint_traits<Request>;
        return execute<typename traits::response_type_t>(
            traits::endpoint.method, std::string{ traits::endpoint.path }, to_query_map(request), traits::endpoint.signed_request);
    }

    /// @brief Generic asynchronous execute (boost::cobalt coroutine).
    ///
    /// Asynchronous counterpart of the generic synchronous execute.
    ///
    /// @tparam Request  A request struct satisfying has_endpoint_traits.
    /// @param request   The populated request struct.
    /// @return Coroutine yielding a typed result with the endpoint's response type.
    template<class Request>
        requires rest::has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
    {
        using traits = rest::endpoint_traits<Request>;
        return async_execute<typename traits::response_type_t>(
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

/// @brief Deferred definition of rest::service::execute.
///
/// Defined here (not in service.hpp) because the full client definition is
/// required to delegate the call.
template<class Request>
    requires rest::has_endpoint_traits<Request>
auto
rest::service::execute(const Request& request) -> result<typename rest::endpoint_traits<Request>::response_type_t>
{
    return owner_.execute(request);
}

/// @brief Deferred definition of rest::service::async_execute.
///
/// Defined here for the same reason as execute above.
template<class Request>
    requires rest::has_endpoint_traits<Request>
auto
rest::service::async_execute(const Request& request)
    -> boost::cobalt::task<result<typename rest::endpoint_traits<Request>::response_type_t>>
{
    co_return co_await owner_.async_execute(request);
}

} // namespace binapi2::fapi
