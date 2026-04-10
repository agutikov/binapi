// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Explicit template instantiation for the default market_streams transport.

#include <binapi2/fapi/streams/market_streams.hpp>

namespace binapi2::fapi::streams {

template class basic_market_streams<transport::websocket_client>;

} // namespace binapi2::fapi::streams
