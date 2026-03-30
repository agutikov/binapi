#include <binapi2/umf/streams/user_streams.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::umf::streams {

user_streams::user_streams(boost::asio::io_context &io_context, config cfg)
    : transport_(io_context, cfg)
    , cfg_(std::move(cfg)) {}

result<void> user_streams::connect(const std::string &listen_key) {
    const auto target = cfg_.stream_base_target + "/" + listen_key;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

result<void> user_streams::read_loop(
    account_update_handler account_handler,
    order_trade_update_handler order_handler,
    margin_call_handler margin_handler,
    listen_key_expired_handler listen_key_expired
) {
    return transport_.run_read_loop([
        account_handler = std::move(account_handler),
        order_handler = std::move(order_handler),
        margin_handler = std::move(margin_handler),
        listen_key_expired = std::move(listen_key_expired)](const std::string &payload) {
        if (payload.find("\"e\": \"ACCOUNT_UPDATE\"") != std::string::npos || payload.find("\"e\":\"ACCOUNT_UPDATE\"") != std::string::npos) {
            types::account_update_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return account_handler(event);
        }
        if (payload.find("\"e\": \"ORDER_TRADE_UPDATE\"") != std::string::npos || payload.find("\"e\":\"ORDER_TRADE_UPDATE\"") != std::string::npos) {
            types::order_trade_update_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return order_handler(event);
        }
        if (margin_handler && (payload.find("\"e\": \"MARGIN_CALL\"") != std::string::npos || payload.find("\"e\":\"MARGIN_CALL\"") != std::string::npos)) {
            types::margin_call_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return margin_handler(event);
        }
        if (listen_key_expired && (payload.find("\"e\": \"listenKeyExpired\"") != std::string::npos || payload.find("\"e\":\"listenKeyExpired\"") != std::string::npos)) {
            types::listen_key_expired_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return listen_key_expired(event);
        }
        return true;
    });
}

result<void> user_streams::close() {
    return transport_.close();
}

} // namespace binapi2::umf::streams
