// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Explicit template instantiation for the default stream_connection transport.

#include <binapi2/fapi/streams/stream_connection.hpp>

namespace binapi2::fapi::streams {

template class basic_stream_connection<transport::websocket_client>;

} // namespace binapi2::fapi::streams
