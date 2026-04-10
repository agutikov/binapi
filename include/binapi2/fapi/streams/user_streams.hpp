// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_streams.hpp
/// @brief Async user data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/variant_parse.hpp>
#include <binapi2/fapi/result.hpp>
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

inline constexpr fapi::detail::variant_entry<user_event_variant> user_event_mapping[] = {
    fapi::detail::make_entry<types::account_update_event_t, user_event_variant>(
        types::event_traits<types::account_update_event_t>::wire_name),
    fapi::detail::make_entry<types::order_trade_update_event_t, user_event_variant>(
        types::event_traits<types::order_trade_update_event_t>::wire_name),
    fapi::detail::make_entry<types::margin_call_event_t, user_event_variant>(
        types::event_traits<types::margin_call_event_t>::wire_name),
    fapi::detail::make_entry<types::listen_key_expired_event_t, user_event_variant>(
        types::event_traits<types::listen_key_expired_event_t>::wire_name),
    fapi::detail::make_entry<types::account_config_update_event_t, user_event_variant>(
        types::event_traits<types::account_config_update_event_t>::wire_name),
    fapi::detail::make_entry<types::trade_lite_event_t, user_event_variant>(
        types::event_traits<types::trade_lite_event_t>::wire_name),
    fapi::detail::make_entry<types::algo_order_update_event_t, user_event_variant>(
        types::event_traits<types::algo_order_update_event_t>::wire_name),
    fapi::detail::make_entry<types::conditional_order_trigger_reject_event_t, user_event_variant>(
        types::event_traits<types::conditional_order_trigger_reject_event_t>::wire_name),
    fapi::detail::make_entry<types::grid_update_event_t, user_event_variant>(
        types::event_traits<types::grid_update_event_t>::wire_name),
    fapi::detail::make_entry<types::strategy_update_event_t, user_event_variant>(
        types::event_traits<types::strategy_update_event_t>::wire_name),
};

/// @brief Parse a user stream event from raw JSON using discriminator dispatch.
///
/// Extracts the "e" field value, maps it to the correct variant alternative,
/// and parses the JSON exactly once.
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
/// Usage (generator — recommended):
///   auto stream = user_streams.subscribe(listen_key);
///   while (stream) {
///       auto event = co_await stream;
///       if (!event) break;
///       std::visit(overloaded{
///           [](const types::order_trade_update_event_t& e) { ... },
///           [](const auto&) {}
///       }, *event);
///   }
///
/// Usage (low-level):
///   co_await user_streams.async_connect(listen_key);
///   auto msg = co_await user_streams.async_read_text();
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client>
class basic_user_streams
{
public:
    explicit basic_user_streams(config cfg) :
        transport_(cfg), cfg_(std::move(cfg))
    {
    }

    basic_user_streams(const basic_user_streams&) = delete;
    basic_user_streams& operator=(const basic_user_streams&) = delete;

    // -- Generator --

    /// @brief Subscribe and return a typed async generator yielding user_stream_event_t variants.
    user_event_generator subscribe(std::string listen_key)
    {
        using Event = types::user_stream_event_t;
        auto conn = co_await async_connect(std::move(listen_key));
        if (!conn) {
            co_yield result<Event>::failure(conn.err);
        } else {
            bool running = true;
            while (running) {
                auto msg = co_await transport_.async_read_text();
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

    // -- Low-level --

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string listen_key)
    {
        const auto target = cfg_.stream_base_target + "/" + listen_key;
        co_return co_await transport_.async_connect(cfg_.stream_host, cfg_.stream_port, target);
    }

    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text()
    {
        co_return co_await transport_.async_read_text();
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await transport_.async_close();
    }

    /// @brief Access the underlying transport.
    [[nodiscard]] Transport& transport() noexcept { return transport_; }

private:
    Transport transport_;
    config cfg_;
};

/// @brief Default user streams using the WebSocket transport.
using user_streams = basic_user_streams<>;

} // namespace binapi2::fapi::streams
