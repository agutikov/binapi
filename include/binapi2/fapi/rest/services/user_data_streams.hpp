// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief User data stream REST service for managing WebSocket listen keys.

#pragma once

#include <binapi2/fapi/rest/services/service.hpp>
#include <binapi2/fapi/types/account.hpp>

namespace binapi2::fapi::rest {

class user_data_stream_service : public service
{
public:
    using service::service;

    template<class Request>
        requires is_user_data_stream_request<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

    using start_request = types::start_listen_key_request_t;
    using keepalive_request = types::keepalive_listen_key_request_t;
    using close_request = types::close_listen_key_request_t;
};

} // namespace binapi2::fapi::rest
