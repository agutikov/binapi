// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Factory for TLS client contexts used by both the HTTP and WebSocket
/// transports. Configures TLS with system-default CA paths and peer
/// verification enabled.

#include <binapi2/fapi/transport/ssl_context.hpp>

namespace binapi2::fapi::transport {

boost::asio::ssl::context
make_ssl_context(const std::string& ca_cert_file)
{
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tls_client };
    ctx.set_default_verify_paths();
    if (!ca_cert_file.empty()) {
        ctx.load_verify_file(ca_cert_file);
    }
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    return ctx;
}

} // namespace binapi2::fapi::transport
