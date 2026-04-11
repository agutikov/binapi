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
#include <binapi2/fapi/streams/stream_consumer.hpp>
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
using user_enum = types::user_event_type_t;

template<typename Event>
constexpr auto user_entry = fapi::detail::make_entry<Event, user_enum, user_event_variant>(
    types::event_traits<Event>::enum_value);

inline constexpr fapi::detail::variant_entry<user_enum, user_event_variant> user_event_mapping[] = {
    user_entry<types::account_update_event_t>,
    user_entry<types::order_trade_update_event_t>,
    user_entry<types::margin_call_event_t>,
    user_entry<types::listen_key_expired_event_t>,
    user_entry<types::account_config_update_event_t>,
    user_entry<types::trade_lite_event_t>,
    user_entry<types::algo_order_update_event_t>,
    user_entry<types::conditional_order_trigger_reject_event_t>,
    user_entry<types::grid_update_event_t>,
    user_entry<types::strategy_update_event_t>,
};

/// @brief Parse a user stream event from raw JSON using discriminator dispatch.
inline result<types::user_stream_event_t> parse_user_event(const std::string& payload)
{
    auto parsed = fapi::detail::parse_variant<user_enum, user_event_variant>("e", payload, user_event_mapping);
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
template<transport::websocket_transport Transport = transport::websocket_client,
         stream_consumer Consumer = inline_consumer>
class basic_user_stream
{
public:
    explicit basic_user_stream(config cfg) :
        conn_(std::move(cfg))
    {
    }

    basic_user_stream(config cfg, Consumer consumer) :
        conn_(std::move(cfg), std::move(consumer))
    {
    }

    basic_user_stream(const basic_user_stream&) = delete;
    basic_user_stream& operator=(const basic_user_stream&) = delete;

    // -- Generator --

    /// @brief Subscribe with auto-reconnect.
    ///
    /// On connection drop, reconnects with the same listen key.
    /// Note: if the listen key has expired, the reconnect will fail and
    /// the generator yields the error. The caller must manage listen key
    /// keepalive separately.
    user_event_generator subscribe(types::listen_key_t listen_key)
    {
        using Event = types::user_stream_event_t;
        const auto host = conn_.configuration().stream_host;
        const auto port = conn_.configuration().stream_port;
        const ws_target_t target = conn_.configuration().stream_base_target + "/" + listen_key;

        error last_err;
        int failures = 0;

        while (failures <= max_reconnects_) {
            auto conn = co_await conn_.async_connect(host, port, target);
            if (!conn) {
                last_err = conn.err;
                ++failures;
                continue;
            }

            failures = 0;

            while (true) {
                auto msg = co_await conn_.async_read_text();
                if (!msg) {
                    last_err = msg.err;
                    co_await conn_.async_close();
                    ++failures;
                    break;
                }

                auto event = parse_user_event(*msg);
                co_yield std::move(event);
                if (!event) {
                    failures = max_reconnects_ + 1;
                    break;
                }
            }
        }

        co_yield result<Event>::failure(last_err);
    }

    /// @brief Set maximum consecutive reconnect attempts (default: 3).
    void set_max_reconnects(int n) { max_reconnects_ = n; }

    // -- Imperative API --

    /// @brief Connect to the user data stream endpoint.
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect(types::listen_key_t listen_key)
    {
        const auto host = conn_.configuration().stream_host;
        const auto port = conn_.configuration().stream_port;
        const ws_target_t target = conn_.configuration().stream_base_target + "/" + listen_key;
        co_return co_await conn_.async_connect(host, port, target);
    }

    /// @brief Read the next user event from the stream.
    ///
    /// Returns a parsed user_stream_event_t variant, or an error if the
    /// connection is closed or the JSON is unrecognized.
    [[nodiscard]] boost::cobalt::task<result<types::user_stream_event_t>>
    async_read_event()
    {
        auto msg = co_await conn_.async_read_text();
        if (!msg)
            co_return result<types::user_stream_event_t>::failure(msg.err);
        co_return parse_user_event(*msg);
    }

    /// @brief Close the connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await conn_.async_close();
    }

    // -- Accessors --

    [[nodiscard]] Transport& transport() noexcept { return conn_.transport(); }
    [[nodiscard]] basic_stream_connection<Transport, Consumer>& connection() noexcept { return conn_; }

private:
    basic_stream_connection<Transport, Consumer> conn_;
    int max_reconnects_{3};
};

/// @brief Default user streams using the WebSocket transport.
using user_stream = basic_user_stream<>;

} // namespace binapi2::fapi::streams
