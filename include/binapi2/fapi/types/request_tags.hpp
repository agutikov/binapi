// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file request_tags.hpp
/// @brief Empty tag base types for service-level request type categorization.
///
/// Each request type inherits the tag of the service it belongs to.
/// Used by concepts to constrain async_execute on each service.

#pragma once

namespace binapi2::fapi::types {

struct rest_market_data_tag {};
struct rest_account_tag {};
struct rest_trade_tag {};
struct rest_convert_tag {};
struct rest_user_data_stream_tag {};
struct ws_api_tag {};
struct stream_tag {};

} // namespace binapi2::fapi::types
