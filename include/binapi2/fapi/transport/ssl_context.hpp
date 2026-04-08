// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file ssl_context.hpp
/// @brief Factory for pre-configured Boost.Asio SSL contexts.

#pragma once

#include <boost/asio/ssl/context.hpp>

#include <string>

namespace binapi2::fapi::transport {

/// @brief Create a TLS/SSL context suitable for connecting to the Binance API.
///
/// Returns a context configured with appropriate TLS version constraints and
/// certificate verification settings for secure HTTPS and WSS connections.
///
/// @param ca_cert_file  Optional path to a PEM CA certificate file. When
///                      non-empty, the file is loaded as a trusted authority
///                      in addition to system defaults.
/// @return A ready-to-use Boost.Asio SSL context.
boost::asio::ssl::context
make_ssl_context(const std::string& ca_cert_file);

} // namespace binapi2::fapi::transport
