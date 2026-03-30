#pragma once

#include <boost/asio/ssl/context.hpp>

namespace binapi2::umf::transport {

boost::asio::ssl::context
make_ssl_context();

} // namespace binapi2::umf::transport
