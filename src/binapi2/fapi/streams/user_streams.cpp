// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Explicit template instantiation for the default user_streams transport.

#include <binapi2/fapi/streams/user_streams.hpp>

namespace binapi2::fapi::streams {

template class basic_user_streams<transport::websocket_client>;

} // namespace binapi2::fapi::streams
