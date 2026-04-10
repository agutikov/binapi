// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Explicit template instantiation for the default user_stream transport.

#include <binapi2/fapi/streams/user_stream.hpp>

namespace binapi2::fapi::streams {

template class basic_user_stream<transport::websocket_client>;

} // namespace binapi2::fapi::streams
