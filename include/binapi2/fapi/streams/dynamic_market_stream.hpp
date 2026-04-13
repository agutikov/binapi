// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file dynamic_market_stream.hpp
/// @brief Dynamic market data stream with live subscribe/unsubscribe.
///
/// Connects to the /stream combined endpoint. Subscriptions can be added
/// and removed at any time. Events and control responses are returned
/// through a single async_read_event() call.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/detail/variant_parse.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/detail/stream_connection.hpp>
#include <binapi2/fapi/streams/detail/stream_consumer.hpp>
#include <binapi2/fapi/streams/stream_traits.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>
#include <binapi2/fapi/types/event_traits.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>

#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Control response from SUBSCRIBE/UNSUBSCRIBE/LIST_SUBSCRIPTIONS.
struct control_response_t
{
    std::optional<std::vector<std::string>> result{};
    unsigned int id{};
};

/// @brief Message from a dynamic market stream: either a market event or a control response.
using dynamic_stream_message_t = std::variant<types::market_stream_event_t, control_response_t>;

namespace detail {

/// @brief Wrapper for control responses.
struct dynamic_control_response
{
    std::optional<std::vector<std::string>> result{};
    unsigned int id{};
};

} // namespace detail

} // namespace binapi2::fapi::streams

template<>
struct glz::meta<binapi2::fapi::streams::detail::dynamic_control_response>
{
    using T = binapi2::fapi::streams::detail::dynamic_control_response;
    static constexpr auto value = object("result", &T::result, "id", &T::id);
};

template<>
struct glz::meta<binapi2::fapi::streams::control_response_t>
{
    using T = binapi2::fapi::streams::control_response_t;
    static constexpr auto value = object("result", &T::result, "id", &T::id);
};

namespace binapi2::fapi::streams {

namespace detail {

using market_enum = types::market_event_type_t;
using market_variant = types::market_stream_event_t;

template<typename Event>
constexpr auto market_entry = fapi::detail::make_entry<Event, market_enum, market_variant>(
    types::event_traits<Event>::enum_value);

inline constexpr fapi::detail::variant_entry<market_enum, market_variant> market_event_mapping[] = {
    market_entry<types::book_ticker_stream_event_t>,
    market_entry<types::aggregate_trade_stream_event_t>,
    market_entry<types::mark_price_stream_event_t>,
    market_entry<types::depth_stream_event_t>,
    market_entry<types::mini_ticker_stream_event_t>,
    market_entry<types::ticker_stream_event_t>,
    market_entry<types::liquidation_order_stream_event_t>,
    market_entry<types::kline_stream_event_t>,
    market_entry<types::continuous_contract_kline_stream_event_t>,
    market_entry<types::composite_index_stream_event_t>,
    market_entry<types::contract_info_stream_event_t>,
    market_entry<types::asset_index_stream_event_t>,
    market_entry<types::trading_session_stream_event_t>,
};

} // namespace detail

/// @brief Dynamic market data stream with live subscribe/unsubscribe.
///
/// Connects to the /stream combined endpoint. async_subscribe() and
/// async_unsubscribe() send control messages (write-only). async_read_event()
/// reads the next frame and returns either a typed market event or a control
/// response.
///
/// Usage:
///   dynamic_market_stream dyn(cfg);
///   co_await dyn.async_connect();
///   co_await dyn.async_subscribe(book_ticker_subscription{.symbol = "BTCUSDT"});
///
///   while (true) {
///       auto msg = co_await dyn.async_read_event();
///       if (!msg) break;
///       std::visit(overloaded{
///           [](const market_stream_event_t& ev) { std::visit(..., ev); },
///           [](const control_response_t& cr) { /* ack */ },
///       }, *msg);
///   }
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
template<transport::websocket_transport Transport = transport::websocket_client,
         stream_consumer Consumer = inline_consumer>
class basic_dynamic_market_stream
{
public:
    explicit basic_dynamic_market_stream(config cfg) :
        conn_(std::move(cfg))
    {
    }

    basic_dynamic_market_stream(config cfg, Consumer consumer) :
        conn_(std::move(cfg), std::move(consumer))
    {
    }

    basic_dynamic_market_stream(const basic_dynamic_market_stream&) = delete;
    basic_dynamic_market_stream& operator=(const basic_dynamic_market_stream&) = delete;

    // -- Connection --

    [[nodiscard]] boost::cobalt::task<result<void>> async_connect()
    {
        co_return co_await conn_.async_connect(
            conn_.configuration().stream_host,
            conn_.configuration().stream_port,
            conn_.configuration().combined_stream_target);
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await conn_.async_close();
    }

    // -- Subscription control (send only, no read) --

    template<class... Subscriptions>
        requires (has_stream_traits<Subscriptions> && ...)
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_subscribe(const Subscriptions&... subs)
    {
        co_return co_await send_control("SUBSCRIBE", make_topics(subs...), next_id());
    }

    template<class... Subscriptions>
        requires (has_stream_traits<Subscriptions> && ...)
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_unsubscribe(const Subscriptions&... subs)
    {
        co_return co_await send_control("UNSUBSCRIBE", make_topics(subs...), next_id());
    }

    [[nodiscard]] boost::cobalt::task<result<void>>
    async_list_subscriptions()
    {
        co_return co_await send_control("LIST_SUBSCRIPTIONS", {}, next_id());
    }

    // -- Read next message (event or control response) --

    [[nodiscard]] boost::cobalt::task<result<dynamic_stream_message_t>>
    async_read_event()
    {
        auto msg = co_await conn_.async_read_text();
        if (!msg)
            co_return result<dynamic_stream_message_t>::failure(msg.err);

        // Try as data frame: {"stream": "...", "data": {...}}
        types::combined_stream_frame_t frame{};
        glz::context ctx{};
        if (!glz::read<fapi::detail::json_read_opts>(frame, *msg, ctx) && !frame.stream.empty()) {
            // Parse the "data" field by "e" discriminator (enum-based)
            auto parsed = fapi::detail::parse_variant<detail::market_enum, detail::market_variant>(
                "e", frame.data.str, detail::market_event_mapping);
            if (parsed)
                co_return result<dynamic_stream_message_t>::success(
                    dynamic_stream_message_t{std::move(*parsed)});
            co_return result<dynamic_stream_message_t>::failure(
                {error_code::json, 0, 0, "unknown market event in topic: " + frame.stream, std::string(frame.data.str)});
        }

        // Try as control response: {"result": ..., "id": ...}
        detail::dynamic_control_response ctrl{};
        glz::context ctx2{};
        if (!glz::read<fapi::detail::json_read_opts>(ctrl, *msg, ctx2)) {
            co_return result<dynamic_stream_message_t>::success(
                dynamic_stream_message_t{control_response_t{std::move(ctrl.result), ctrl.id}});
        }

        co_return result<dynamic_stream_message_t>::failure(
            {error_code::json, 0, 0, "unrecognized frame", *msg});
    }

    // -- Accessors --

    [[nodiscard]] Transport& transport() noexcept { return conn_.transport(); }
    [[nodiscard]] basic_stream_connection<Transport, Consumer>& connection() noexcept { return conn_; }

private:
    unsigned int next_id() { return ++id_counter_; }

    template<class... Subscriptions>
    std::vector<std::string> make_topics(const Subscriptions&... subs)
    {
        std::vector<std::string> topics;
        topics.reserve(sizeof...(subs));
        (topics.push_back(stream_traits<Subscriptions>::topic(conn_.configuration(), subs)), ...);
        return topics;
    }

    [[nodiscard]] boost::cobalt::task<result<void>>
    send_control(const char* method, std::vector<std::string> topics, unsigned int id)
    {
        detail::stream_control_request req{ .method = method, .params = std::move(topics), .id = id };
        auto json = glz::write_json(req);
        if (!json)
            co_return result<void>::failure({error_code::json, 0, 0, "failed to serialize control request", {}});
        co_return co_await conn_.async_write_text(*json);
    }

    basic_stream_connection<Transport, Consumer> conn_;
    unsigned int id_counter_{0};
};

using dynamic_market_stream = basic_dynamic_market_stream<>;

} // namespace binapi2::fapi::streams
