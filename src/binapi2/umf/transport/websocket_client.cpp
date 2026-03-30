#include <binapi2/umf/transport/websocket_client.hpp>

#include <binapi2/umf/error.hpp>
#include <binapi2/umf/transport/ssl_context.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <openssl/err.h>

namespace binapi2::umf::transport {

struct websocket_client::impl {
    impl(boost::asio::io_context &io_context, config cfg)
        : ssl_ctx(make_ssl_context())
        , resolver(io_context)
        , stream(io_context, ssl_ctx)
        , cfg(std::move(cfg)) {}

    boost::asio::ssl::context ssl_ctx;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> stream;
    boost::beast::flat_buffer buffer;
    config cfg;
    bool connected{false};
};

websocket_client::websocket_client(boost::asio::io_context &io_context, config cfg)
    : session_base(cfg)
    , impl_(std::make_unique<impl>(io_context, std::move(cfg))) {}

websocket_client::~websocket_client() = default;

result<void> websocket_client::connect(const std::string &host, const std::string &port, const std::string &target) {
    if (!SSL_set_tlsext_host_name(impl_->stream.next_layer().native_handle(), host.c_str())) {
        return result<void>::failure({error_code::websocket, 0, 0, ERR_error_string(ERR_get_error(), nullptr), {}});
    }

    boost::system::error_code ec;
    auto endpoints = impl_->resolver.resolve(host, port, ec);
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }

    boost::asio::connect(impl_->stream.next_layer().next_layer(), endpoints, ec);
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }

    impl_->stream.control_callback([this](boost::beast::websocket::frame_type kind, boost::beast::string_view) {
        if (kind == boost::beast::websocket::frame_type::ping) {
            boost::system::error_code ignored;
            impl_->stream.pong({}, ignored);
        }
    });

    impl_->stream.next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }

    impl_->stream.handshake(host, target, ec);
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }

    impl_->connected = true;
    return result<void>::success();
}

result<void> websocket_client::write_text(const std::string &message) {
    boost::system::error_code ec;
    impl_->stream.write(boost::asio::buffer(message), ec);
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }
    return result<void>::success();
}

result<std::string> websocket_client::read_text() {
    impl_->buffer.consume(impl_->buffer.size());
    boost::system::error_code ec;
    impl_->stream.read(impl_->buffer, ec);
    if (ec) {
        return result<std::string>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }

    std::string data = boost::beast::buffers_to_string(impl_->buffer.data());
    return result<std::string>::success(std::move(data));
}

result<void> websocket_client::run_read_loop(message_handler handler) {
    while (true) {
        auto message = read_text();
        if (!message) {
            return result<void>::failure(message.err);
        }
        if (!handler(*message)) {
            break;
        }
    }
    return result<void>::success();
}

result<void> websocket_client::close() {
    if (!impl_->connected) {
        return result<void>::success();
    }

    boost::system::error_code ec;
    impl_->stream.close(boost::beast::websocket::close_code::normal, ec);
    impl_->connected = false;
    if (ec) {
        return result<void>::failure({error_code::websocket, 0, 0, ec.message(), {}});
    }
    return result<void>::success();
}

} // namespace binapi2::umf::transport
