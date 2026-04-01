// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/service.hpp>
#include <binapi2/fapi/types/convert.hpp>

namespace binapi2::fapi::rest {

class convert_service : public service
{
public:
    using service::service;

    using quote_request = types::convert_quote_request;
    using quote_response = types::convert_quote_response;
    using accept_request = types::convert_accept_request;
    using accept_response = types::convert_accept_response;
    using order_status_request = types::convert_order_status_request;
    using order_status_response = types::convert_order_status_response;
};

} // namespace binapi2::fapi::rest
