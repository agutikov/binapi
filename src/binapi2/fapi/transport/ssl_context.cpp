// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Factory for TLS client contexts used by both the HTTP and WebSocket
/// transports. Configures TLS with system-default CA paths and peer
/// verification enabled.

#include <binapi2/fapi/transport/ssl_context.hpp>

namespace binapi2::fapi::transport {

boost::asio::ssl::context
make_ssl_context()
{
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tls_client };
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    return ctx;
}

} // namespace binapi2::fapi::transport
