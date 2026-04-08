// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the REST pipeline: the async_execute coroutine that signs
/// requests, encodes query parameters, dispatches HTTP, and decodes responses.

#include <binapi2/fapi/rest/pipeline.hpp>

#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>

namespace binapi2::fapi::rest {

pipeline::pipeline(const config& cfg, transport::http_client& http)
    : cfg_(cfg), http_(http)
{
}

} // namespace binapi2::fapi::rest
