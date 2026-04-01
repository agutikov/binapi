// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/result.hpp>

#include <functional>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class service
{
public:
    template<typename T>
    using callback_type = std::function<void(result<T>)>;

    explicit service(client& owner) noexcept : owner_(owner) {}

    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request) -> result<typename endpoint_traits<Request>::response_type>;

    template<class Request>
        requires has_endpoint_traits<Request>
    void async_execute(const Request& request, callback_type<typename endpoint_traits<Request>::response_type> callback);

protected:
    client& owner_;
};

} // namespace binapi2::fapi::rest
