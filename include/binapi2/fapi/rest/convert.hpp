// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Convert service for USD-M Futures asset conversion endpoints.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/convert.hpp>

namespace binapi2::fapi::rest {

/// @brief Service group for futures asset conversion (quote, accept, status).
///
/// This service is fully generic: all three request types (quote_request,
/// accept_request, order_status_request) have endpoint_traits specializations
/// and are dispatched entirely through the inherited execute/async_execute
/// template methods. No named methods are needed.
///
/// The using declarations provide convenient short aliases for the request
/// and response types within the convert_service scope.
class convert_service : public service
{
public:
    using service::service;

    using quote_request = types::convert_quote_request_t;
    using quote_response = types::convert_quote_response_t;
    using accept_request = types::convert_accept_request_t;
    using accept_response = types::convert_accept_response_t;
    using order_status_request = types::convert_order_status_request_t;
    using order_status_response = types::convert_order_status_response_t;
};

} // namespace binapi2::fapi::rest
