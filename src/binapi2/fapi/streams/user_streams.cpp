// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/streams/user_streams.hpp>

#include <boost/asio/post.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::fapi::streams {

user_streams::user_streams(boost::asio::io_context& io_context, config cfg) :
    io_context_(io_context), transport_(io_context, cfg), cfg_(std::move(cfg))
{
}

result<void>
user_streams::connect(const std::string& listen_key)
{
    const auto target = cfg_.stream_base_target + "/" + listen_key;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
user_streams::connect(const std::string& listen_key, void_callback callback)
{
    boost::asio::post(io_context_,
                      [this, listen_key, callback = std::move(callback)]() mutable { callback(connect(listen_key)); });
}

result<void>
user_streams::read_loop(account_update_handler account_handler,
                        order_trade_update_handler order_handler,
                        margin_call_handler margin_handler,
                        listen_key_expired_handler listen_key_expired)
{
    return transport_.run_read_loop([account_handler = std::move(account_handler),
                                     order_handler = std::move(order_handler),
                                     margin_handler = std::move(margin_handler),
                                     listen_key_expired = std::move(listen_key_expired)](const std::string& payload) {
        if (payload.find("\"e\": \"ACCOUNT_UPDATE\"") != std::string::npos ||
            payload.find("\"e\":\"ACCOUNT_UPDATE\"") != std::string::npos) {
            types::account_update_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return account_handler(event);
        }
        if (payload.find("\"e\": \"ORDER_TRADE_UPDATE\"") != std::string::npos ||
            payload.find("\"e\":\"ORDER_TRADE_UPDATE\"") != std::string::npos) {
            types::order_trade_update_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return order_handler(event);
        }
        if (margin_handler && (payload.find("\"e\": \"MARGIN_CALL\"") != std::string::npos ||
                               payload.find("\"e\":\"MARGIN_CALL\"") != std::string::npos)) {
            types::margin_call_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return margin_handler(event);
        }
        if (listen_key_expired && (payload.find("\"e\": \"listenKeyExpired\"") != std::string::npos ||
                                   payload.find("\"e\":\"listenKeyExpired\"") != std::string::npos)) {
            types::listen_key_expired_event event{};
            if (glz::read_json(event, payload)) {
                return false;
            }
            return listen_key_expired(event);
        }
        return true;
    });
}

void
user_streams::read_loop(account_update_handler account_handler,
                        order_trade_update_handler order_handler,
                        margin_call_handler margin_handler,
                        listen_key_expired_handler listen_key_expired,
                        void_callback callback)
{
    boost::asio::post(io_context_,
                      [this,
                       account_handler = std::move(account_handler),
                       order_handler = std::move(order_handler),
                       margin_handler = std::move(margin_handler),
                       listen_key_expired = std::move(listen_key_expired),
                       callback = std::move(callback)]() mutable {
                          callback(read_loop(std::move(account_handler),
                                             std::move(order_handler),
                                             std::move(margin_handler),
                                             std::move(listen_key_expired)));
                      });
}

namespace {

bool
match_event(const std::string& payload, const char* event_name)
{
    std::string with_space = std::string("\"e\": \"") + event_name + "\"";
    std::string without_space = std::string("\"e\":\"") + event_name + "\"";
    return payload.find(with_space) != std::string::npos || payload.find(without_space) != std::string::npos;
}

template<typename Event, typename Handler>
bool
try_dispatch(const std::string& payload, const char* event_name, const Handler& handler)
{
    if (!handler) {
        return false;
    }
    if (!match_event(payload, event_name)) {
        return false;
    }
    Event event{};
    if (glz::read_json(event, payload)) {
        return false;
    }
    handler(event);
    return true;
}

} // namespace

result<void>
user_streams::read_loop(const handlers& h)
{
    return transport_.run_read_loop([h](const std::string& payload) {
        if (try_dispatch<types::account_update_event>(payload, "ACCOUNT_UPDATE", h.on_account_update)) {
            return true;
        }
        if (try_dispatch<types::order_trade_update_event>(payload, "ORDER_TRADE_UPDATE", h.on_order_trade_update)) {
            return true;
        }
        if (try_dispatch<types::margin_call_event>(payload, "MARGIN_CALL", h.on_margin_call)) {
            return true;
        }
        if (try_dispatch<types::listen_key_expired_event>(payload, "listenKeyExpired", h.on_listen_key_expired)) {
            return true;
        }
        if (try_dispatch<types::account_config_update_event>(payload, "ACCOUNT_CONFIG_UPDATE", h.on_account_config_update)) {
            return true;
        }
        if (try_dispatch<types::trade_lite_event>(payload, "TRADE_LITE", h.on_trade_lite)) {
            return true;
        }
        if (try_dispatch<types::algo_order_update_event>(payload, "ALGO_UPDATE", h.on_algo_order_update)) {
            return true;
        }
        if (try_dispatch<types::conditional_order_trigger_reject_event>(
                payload, "CONDITIONAL_ORDER_TRIGGER_REJECT", h.on_conditional_order_reject)) {
            return true;
        }
        if (try_dispatch<types::grid_update_event>(payload, "GRID_UPDATE", h.on_grid_update)) {
            return true;
        }
        if (try_dispatch<types::strategy_update_event>(payload, "STRATEGY_UPDATE", h.on_strategy_update)) {
            return true;
        }
        return true;
    });
}

void
user_streams::read_loop(const handlers& h, void_callback callback)
{
    boost::asio::post(io_context_,
                      [this, h, callback = std::move(callback)]() mutable { callback(read_loop(h)); });
}

result<void>
user_streams::close()
{
    return transport_.close();
}

void
user_streams::close(void_callback callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(close()); });
}

} // namespace binapi2::fapi::streams
