// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Base class for REST service groups, providing generic execute methods
///        that dispatch through endpoint_traits.

#pragma once

#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

/// @brief Base class for domain-specific REST service groups (market_data, trade, etc.).
///
/// Holds a non-owning reference to the parent client and provides generic
/// execute/async_execute template methods that forward to client::execute via
/// endpoint_traits lookup. Derived services inherit these for any request type
/// that has a 1:1 endpoint mapping, and add named methods for shared request types
/// or parameterless endpoints.
///
/// Template method bodies are defined in client.hpp because they require the
/// full definition of the client class.
class service
{
public:
    explicit service(client& owner) noexcept : owner_(owner) {}

    /// @brief Synchronous generic execute for any request type with endpoint_traits.
    ///
    /// Blocks the calling thread. Must not be called from within a coroutine context.
    ///
    /// @tparam Request  A request struct satisfying has_endpoint_traits. The response
    ///                  type is deduced from endpoint_traits<Request>::response_type.
    /// @param request   The populated request struct.
    /// @return Typed result with the deduced response type, or an error.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto execute(const Request& request) -> result<typename endpoint_traits<Request>::response_type>;

    /// @brief Asynchronous generic execute (boost::cobalt coroutine).
    ///
    /// Coroutine counterpart of execute. This is the primary execution path.
    ///
    /// @tparam Request  A request struct satisfying has_endpoint_traits.
    /// @param request   The populated request struct.
    /// @return Coroutine yielding a typed result with the deduced response type.
    template<class Request>
        requires has_endpoint_traits<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type>>;

protected:
    client& owner_;
};

} // namespace binapi2::fapi::rest
