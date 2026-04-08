// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Base class for REST service groups, providing generic async_execute
///        that dispatches through endpoint_traits via the REST pipeline.

#pragma once

#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

/// @brief Base class for domain-specific REST service groups (market_data, trade, etc.).
///
/// Holds a non-owning reference to a rest::pipeline and provides generic
/// async_execute template that forwards to pipeline::async_execute via
/// endpoint_traits lookup.
class service
{
public:
    explicit service(pipeline& p) noexcept : pipeline_(p) {}

    /// @brief Generic async execute for any request type with endpoint_traits.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

protected:
    pipeline& pipeline_;
};

} // namespace binapi2::fapi::rest
