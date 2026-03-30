#include <binapi2/umf/transport/ssl_context.hpp>

namespace binapi2::umf::transport {

boost::asio::ssl::context make_ssl_context() {
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    return ctx;
}

} // namespace binapi2::umf::transport
