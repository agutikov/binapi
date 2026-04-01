// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class service
{
public:
    explicit service(client& owner) noexcept : owner_(owner) {}

    // Sync execute.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request) -> result<typename endpoint_traits<Request>::response_type>;

    // Async execute.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type>>;

protected:
    client& owner_;
};

} // namespace binapi2::fapi::rest
