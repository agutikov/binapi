// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the async user data stream client with variant-based generator.

#include <binapi2/fapi/streams/user_streams.hpp>

#include <binapi2/fapi/detail/json_opts.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::fapi::streams {

user_streams::user_streams(config cfg) :
    transport_(cfg), cfg_(std::move(cfg))
{
}

boost::cobalt::task<result<void>>
user_streams::async_connect(std::string listen_key)
{
    const auto target = cfg_.stream_base_target + "/" + listen_key;
    co_return co_await transport_.async_connect(cfg_.stream_host, cfg_.stream_port, target);
}

boost::cobalt::task<result<std::string>>
user_streams::async_read_text()
{
    co_return co_await transport_.async_read_text();
}

boost::cobalt::task<result<void>>
user_streams::async_close()
{
    co_return co_await transport_.async_close();
}

// --- Event detection and parsing ---

namespace {

bool match_event(const std::string& payload, const char* event_name)
{
    // Binance may or may not include space after colon: "e":"X" or "e": "X"
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

} // namespace

result<types::user_stream_event_t>
user_streams::parse_event(const std::string& payload)
{
    types::user_stream_event_t event;

    if (try_parse<types::account_update_event_t>(payload, "ACCOUNT_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::order_trade_update_event_t>(payload, "ORDER_TRADE_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::margin_call_event_t>(payload, "MARGIN_CALL", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::listen_key_expired_event_t>(payload, "listenKeyExpired", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::account_config_update_event_t>(payload, "ACCOUNT_CONFIG_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::trade_lite_event_t>(payload, "TRADE_LITE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::algo_order_update_event_t>(payload, "ALGO_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::conditional_order_trigger_reject_event_t>(payload, "CONDITIONAL_ORDER_TRIGGER_REJECT", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::grid_update_event_t>(payload, "GRID_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));
    if (try_parse<types::strategy_update_event_t>(payload, "STRATEGY_UPDATE", event)) return result<types::user_stream_event_t>::success(std::move(event));

    return result<types::user_stream_event_t>::failure(
        { error_code::json, 0, 0, "unknown user stream event type", payload });
}

// --- Generator ---

user_event_generator
user_streams::subscribe(std::string listen_key)
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
                auto event = parse_event(*msg);
                co_yield std::move(event);
                if (!event) running = false;
            }
        }
    }
}

} // namespace binapi2::fapi::streams
