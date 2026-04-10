// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_stream.hpp
/// @brief Async user data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/variant_parse.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/stream_connection.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>
#include <binapi2/fapi/types/event_traits.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>

#include <string>
#include <utility>

namespace binapi2::fapi::streams {

/// @brief Typed async event generator for user data streams.
using user_event_generator = boost::cobalt::generator<result<types::user_stream_event_t>>;

// ---------------------------------------------------------------------------
// User event discriminator mapping
// ---------------------------------------------------------------------------

using user_event_variant = types::user_stream_event_t;

template<typename Event>
constexpr fapi::detail::variant_entry<user_event_variant> user_event_entry()
{
    return fapi::detail::make_entry<Event, user_event_variant>(types::event_traits<Event>::wire_name);
}

inline constexpr fapi::detail::variant_entry<user_event_variant> user_event_mapping[] = {
    user_event_entry<types::account_update_event_t>(),
    user_event_entry<types::order_trade_update_event_t>(),
    user_event_entry<types::margin_call_event_t>(),
    user_event_entry<types::listen_key_expired_event_t>(),
    user_event_entry<types::account_config_update_event_t>(),
    user_event_entry<types::trade_lite_event_t>(),
    user_event_entry<types::algo_order_update_event_t>(),
    user_event_entry<types::conditional_order_trigger_reject_event_t>(),
    user_event_entry<types::grid_update_event_t>(),
    user_event_entry<types::strategy_update_event_t>(),
};

/// @brief Parse a user stream event from raw JSON using discriminator dispatch.
inline result<types::user_stream_event_t> parse_user_event(const std::string& payload)
{
    auto parsed = fapi::detail::parse_variant<user_event_variant>("e", payload, user_event_mapping);
    if (parsed)
        return result<user_event_variant>::success(std::move(*parsed));
    return result<user_event_variant>::failure(
        { error_code::json, 0, 0, "unknown or unparseable user stream event", payload });
}

/// @brief Async client for user data stream events.
///
/// Wraps a stream_connection and adds variant event parsing. Usage:
///
///   auto stream = user_stream.subscribe(listen_key);
///   while (stream) {
///       auto event = co_await stream;
///       if (!event) break;
///       std::visit(overloaded{...}, *event);
///   }
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client>
class basic_user_stream
{
public:
    explicit basic_user_stream(config cfg) :
        conn_(std::move(cfg))
    {
    }

    basic_user_stream(const basic_user_stream&) = delete;
    basic_user_stream& operator=(const basic_user_stream&) = delete;

    // -- Generator --

    user_event_generator subscribe(types::listen_key_t listen_key)
    {
        using Event = types::user_stream_event_t;
        auto conn = co_await conn_.async_connect(
            conn_.configuration().stream_host,
            conn_.configuration().stream_port,
            conn_.configuration().stream_base_target + "/" + listen_key);
        if (!conn) {
            co_yield result<Event>::failure(conn.err);
        } else {
            bool running = true;
            while (running) {
                auto msg = co_await conn_.async_read_text();
                if (!msg) {
                    co_yield result<Event>::failure(msg.err);
                    running = false;
                } else {
                    auto event = parse_user_event(*msg);
                    co_yield std::move(event);
                    if (!event) running = false;
                }
            }
        }
    }

    // -- Accessors --

    [[nodiscard]] Transport& transport() noexcept { return conn_.transport(); }
    [[nodiscard]] basic_stream_connection<Transport>& connection() noexcept { return conn_; }

private:
    basic_stream_connection<Transport> conn_;
};

/// @brief Default user streams using the WebSocket transport.
using user_stream = basic_user_stream<>;

} // namespace binapi2::fapi::streams
