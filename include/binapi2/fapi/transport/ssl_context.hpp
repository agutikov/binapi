// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <boost/asio/ssl/context.hpp>

namespace binapi2::fapi::transport {

boost::asio::ssl::context
make_ssl_context();

} // namespace binapi2::fapi::transport
