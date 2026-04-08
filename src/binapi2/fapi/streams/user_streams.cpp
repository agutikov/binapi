// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the user data WebSocket stream consumer. Unlike market
/// streams (one event type per connection), the user data stream multiplexes
/// many event types on a single connection, so incoming messages must be
/// dispatched by inspecting the "e" (event type) field in the raw JSON before
/// deserialising into the correct type. Two read_loop overloads are provided:
///   - A legacy variant accepting individual handler callbacks for the most
///     common events (ACCOUNT_UPDATE, ORDER_TRADE_UPDATE, MARGIN_CALL,
///     listenKeyExpired).
///   - A handlers-struct variant that supports all event types including
///     ACCOUNT_CONFIG_UPDATE, TRADE_LITE, ALGO_UPDATE,
///     CONDITIONAL_ORDER_TRIGGER_REJECT, GRID_UPDATE, and STRATEGY_UPDATE.
///
/// Event type detection uses a string::find on the raw payload rather than a
/// full parse, avoiding the cost of deserialising events the caller did not
/// register a handler for.

#include <binapi2/fapi/streams/user_streams.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>

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

// Legacy read_loop dispatching on the four most common user data event types.
// Each incoming JSON frame is tested with string::find for both whitespace
// variants ("e": "..." and "e":"...") because the Binance server does not
// guarantee consistent whitespace in its JSON output. Unrecognised events
// are silently skipped (returns true to keep reading).
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
            types::account_update_event_t event{};
            glz::context glz_ctx{};
            if (glz::read<fapi::detail::json_read_opts>(event, payload, glz_ctx)) {
                return false;
            }
            return account_handler(event);
        }
        if (payload.find("\"e\": \"ORDER_TRADE_UPDATE\"") != std::string::npos ||
            payload.find("\"e\":\"ORDER_TRADE_UPDATE\"") != std::string::npos) {
            types::order_trade_update_event_t event{};
            glz::context glz_ctx{};
            if (glz::read<fapi::detail::json_read_opts>(event, payload, glz_ctx)) {
                return false;
            }
            return order_handler(event);
        }
        if (margin_handler && (payload.find("\"e\": \"MARGIN_CALL\"") != std::string::npos ||
                               payload.find("\"e\":\"MARGIN_CALL\"") != std::string::npos)) {
            types::margin_call_event_t event{};
            glz::context glz_ctx{};
            if (glz::read<fapi::detail::json_read_opts>(event, payload, glz_ctx)) {
                return false;
            }
            return margin_handler(event);
        }
        if (listen_key_expired && (payload.find("\"e\": \"listenKeyExpired\"") != std::string::npos ||
                                   payload.find("\"e\":\"listenKeyExpired\"") != std::string::npos)) {
            types::listen_key_expired_event_t event{};
            glz::context glz_ctx{};
            if (glz::read<fapi::detail::json_read_opts>(event, payload, glz_ctx)) {
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

// Matches the "e" (event type) field in raw JSON without a full parse.
// Checks both with and without a space after the colon to handle
// inconsistent Binance JSON formatting.
bool
match_event(const std::string& payload, const char* event_name)
{
    std::string with_space = std::string("\"e\": \"") + event_name + "\"";
    std::string without_space = std::string("\"e\":\"") + event_name + "\"";
    return payload.find(with_space) != std::string::npos || payload.find(without_space) != std::string::npos;
}

// Attempts to match and dispatch a single event type. Skips if no handler
// is registered (null check) or the event name does not match. Returns true
// if the event was consumed, false otherwise so the caller can try the
// next event type in the dispatch chain.
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
    glz::context glz_ctx{};
    if (glz::read<fapi::detail::json_read_opts>(event, payload, glz_ctx)) {
        return false;
    }
    handler(event);
    return true;
}

} // namespace

// Full-featured read_loop using the handlers struct. Dispatches each incoming
// frame through a chain of try_dispatch calls. The dispatch order matches
// event frequency in typical usage (account/order updates first). Unmatched
// events are silently ignored (returns true) so the stream continues.
result<void>
user_streams::read_loop(const handlers& h)
{
    return transport_.run_read_loop([h](const std::string& payload) {
        if (try_dispatch<types::account_update_event_t>(payload, "ACCOUNT_UPDATE", h.on_account_update)) {
            return true;
        }
        if (try_dispatch<types::order_trade_update_event_t>(payload, "ORDER_TRADE_UPDATE", h.on_order_trade_update)) {
            return true;
        }
        if (try_dispatch<types::margin_call_event_t>(payload, "MARGIN_CALL", h.on_margin_call)) {
            return true;
        }
        if (try_dispatch<types::listen_key_expired_event_t>(payload, "listenKeyExpired", h.on_listen_key_expired)) {
            return true;
        }
        if (try_dispatch<types::account_config_update_event_t>(payload, "ACCOUNT_CONFIG_UPDATE", h.on_account_config_update)) {
            return true;
        }
        if (try_dispatch<types::trade_lite_event_t>(payload, "TRADE_LITE", h.on_trade_lite)) {
            return true;
        }
        if (try_dispatch<types::algo_order_update_event_t>(payload, "ALGO_UPDATE", h.on_algo_order_update)) {
            return true;
        }
        if (try_dispatch<types::conditional_order_trigger_reject_event_t>(
                payload, "CONDITIONAL_ORDER_TRIGGER_REJECT", h.on_conditional_order_reject)) {
            return true;
        }
        if (try_dispatch<types::grid_update_event_t>(payload, "GRID_UPDATE", h.on_grid_update)) {
            return true;
        }
        if (try_dispatch<types::strategy_update_event_t>(payload, "STRATEGY_UPDATE", h.on_strategy_update)) {
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
