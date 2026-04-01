// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/rest/convert.hpp>

#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <binapi2/fapi/query.hpp>

#include "common.hpp"

namespace binapi2::fapi::rest {

convert_service::convert_service(binapi2::fapi::client& owner) noexcept : owner_(owner) {}

result<types::convert_quote_response>
convert_service::get_quote(const types::convert_quote_request& request)
{
    return owner_.execute<types::convert_quote_response>(convert_get_quote_endpoint.method,
                                                          std::string{ convert_get_quote_endpoint.path },
                                                          to_query_map(request),
                                                          convert_get_quote_endpoint.signed_request);
}

void
convert_service::get_quote(const types::convert_quote_request& request,
                           callback_type<types::convert_quote_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(get_quote(request)); });
}

result<types::convert_accept_response>
convert_service::accept_quote(const types::convert_accept_request& request)
{
    return owner_.execute<types::convert_accept_response>(convert_accept_quote_endpoint.method,
                                                           std::string{ convert_accept_quote_endpoint.path },
                                                           to_query_map(request),
                                                           convert_accept_quote_endpoint.signed_request);
}

void
convert_service::accept_quote(const types::convert_accept_request& request,
                              callback_type<types::convert_accept_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(accept_quote(request)); });
}

result<types::convert_order_status_response>
convert_service::order_status(const types::convert_order_status_request& request)
{
    return owner_.execute<types::convert_order_status_response>(convert_order_status_endpoint.method,
                                                                 std::string{ convert_order_status_endpoint.path },
                                                                 to_query_map(request),
                                                                 convert_order_status_endpoint.signed_request);
}

void
convert_service::order_status(const types::convert_order_status_request& request,
                              callback_type<types::convert_order_status_response> callback)
{
    detail::post_callback(owner_.context(),
                          [this, request, callback = std::move(callback)]() mutable { callback(order_status(request)); });
}

} // namespace binapi2::fapi::rest
