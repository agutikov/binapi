// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_streams.hpp
/// @brief Async user data stream client for Binance USD-M Futures account events.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>
#include <binapi2/fapi/types/user_stream_events.hpp>

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <string>
#include <utility>

namespace binapi2::fapi::streams {

/// @brief Typed async event generator for user data streams.
using user_event_generator = boost::cobalt::generator<result<types::user_stream_event_t>>;

// ---------------------------------------------------------------------------
// User event parsing (transport-independent)
// ---------------------------------------------------------------------------

namespace detail {

inline bool match_event(const std::string& payload, const char* event_name)
{
    std::string with_space = std::string("\"e\": \"") + event_name + "\"";
    std::string without_space = std::string("\"e\":\"") + event_name + "\"";
    return payload.find(with_space) != std::string::npos
        || payload.find(without_space) != std::string::npos;
}

template<typename Event>
bool try_parse(const std::string& payload, const char* event_name, types::user_stream_event_t& out)
{
    if (!match_event(payload, event_name)) return false;
    Event event{};
    glz::context ctx{};
    if (glz::read<fapi::detail::json_read_opts>(event, payload, ctx)) return false;
    out = std::move(event);
    return true;
}

} // namespace detail

/// @brief Detect event type string from raw JSON payload and parse into variant.
inline result<types::user_stream_event_t> parse_user_event(const std::string& payload)
{
    types::user_stream_event_t event;

    if (detail::try_parse<types::account_update_event_t>(payload, "ACCOUNT_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::order_trade_update_event_t>(payload, "ORDER_TRADE_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::margin_call_event_t>(payload, "MARGIN_CALL", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::listen_key_expired_event_t>(payload, "listenKeyExpired", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::account_config_update_event_t>(payload, "ACCOUNT_CONFIG_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::trade_lite_event_t>(payload, "TRADE_LITE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::algo_order_update_event_t>(payload, "ALGO_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::conditional_order_trigger_reject_event_t>(payload, "CONDITIONAL_ORDER_TRIGGER_REJECT", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::grid_update_event_t>(payload, "GRID_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (detail::try_parse<types::strategy_update_event_t>(payload, "STRATEGY_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));

    return result<types::user_stream_event_t>::failure(
        { error_code::json, 0, 0, "unknown user stream event type", payload });
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
