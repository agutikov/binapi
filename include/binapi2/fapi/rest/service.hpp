// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file
/// @brief Base class for REST service groups with generic constrained async_execute.

#pragma once

#include <binapi2/fapi/rest/endpoint_traits.hpp>
#include <binapi2/fapi/rest/pipeline.hpp>
#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

/// @brief Base class for REST services.
///
/// Provides the pipeline reference. Derived services add a constrained
/// async_execute that only accepts their own request types.
class service
{
public:
    explicit service(pipeline& p) noexcept : pipeline_(p) {}

protected:
    pipeline& pipeline_;
};

} // namespace binapi2::fapi::rest
