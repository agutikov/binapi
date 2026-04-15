// SPDX-License-Identifier: Apache-2.0
//
// User data stream commands — listen key management and real-time account events.

#include "cmd_user_stream.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

#include <chrono>
#include <variant>

namespace demo {

namespace types = binapi2::fapi::types;
namespace lib   = binapi2::demo;

namespace {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/// The user-stream "demo loop": start a listen key, subscribe, enable
/// keepalive, then pump events and emit typed summaries via the sink.
boost::cobalt::task<int> run_user_stream(binapi2::futures_usdm_api& c, lib::result_sink& sink)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { sink.on_error(rest.err); sink.on_done(1); co_return 1; }
    auto user_stream = c.create_user_stream();
    if (record_buffer) user_stream->connection().attach_buffer(*record_buffer);

    sink.on_info("requesting listen key...");
    auto key = co_await (*rest)->user_data_streams.async_execute(types::start_listen_key_request_t{});
    if (!key) { sink.on_error(key.err); sink.on_done(1); co_return 1; }
    sink.on_info("listen key: " + key->listenKey);

    sink.on_info("subscribing to user data stream...");
    auto gen = user_stream->subscribe(key->listenKey);

    user_stream->enable_keepalive((*rest)->user_data_streams, std::chrono::minutes(30));
    sink.on_info("keepalive enabled (every 30m)");
    sink.on_info("connected, reading events...");

    while (gen) {
        auto event = co_await gen;
        if (!event) { sink.on_error(event.err); break; }

        bool should_stop = std::visit(overloaded{
            [&sink](const types::account_update_event_t& e) {
                sink.on_info("[account_update] event_time=" + std::to_string(e.event_time.value));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(e); j)
                    sink.on_response_json(*j);
                return false;
            },
            [&sink](const types::order_trade_update_event_t& e) {
                sink.on_info("[order_trade_update] event_time=" + std::to_string(e.event_time.value));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(e); j)
                    sink.on_response_json(*j);
                return false;
            },
            [&sink](const types::margin_call_event_t& e) {
                sink.on_info("[margin_call] event_time=" + std::to_string(e.event_time.value));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(e); j)
                    sink.on_response_json(*j);
                return false;
            },
            [&sink](const types::listen_key_expired_event_t& e) {
                sink.on_info("[listen_key_expired] event_time=" + std::to_string(e.event_time.value));
                return true;
            },
            [&sink](const types::account_config_update_event_t& e) {
                sink.on_info("[account_config_update] event_time=" + std::to_string(e.event_time.value));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(e); j)
                    sink.on_response_json(*j);
                return false;
            },
            [&sink](const types::trade_lite_event_t& e) {
                sink.on_info("[trade_lite] event_time=" + std::to_string(e.event_time.value));
                if (auto j = glz::write<glz::opts{ .prettify = true }>(e); j)
                    sink.on_response_json(*j);
                return false;
            },
            [&sink](const auto& e) {
                sink.on_info("[other event] event_time=" + std::to_string(e.event_time.value));
                return false;
            }
        }, *event);

        if (should_stop) break;
    }

    user_stream->stop_keepalive();
    sink.on_info("user stream ended");
    sink.on_done(0);
    co_return 0;
}

template<typename Request>
CLI::App* add_listen_key(CLI::App& parent, const char* name, const char* desc, selected_cmd& sel)
{
    auto* sub = parent.add_subcommand(name, desc);
    sub->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await lib::exec_user_data_streams(c, Request{}, sink);
        };
    });
    return sub;
}

} // namespace

void register_cmd_user_stream(CLI::App& app, selected_cmd& sel)
{
    constexpr const char* group = "User Data Streams";

    add_listen_key<types::start_listen_key_request_t>    (app, "listen-key-start",     "Start listen key (auth)",     sel)->group(group);
    add_listen_key<types::keepalive_listen_key_request_t>(app, "listen-key-keepalive", "Keepalive listen key (auth)", sel)->group(group);
    add_listen_key<types::close_listen_key_request_t>    (app, "listen-key-close",     "Close listen key (auth)",     sel)->group(group);

    auto* us = app.add_subcommand("user-stream", "User data stream demo (auth)");
    us->group(group);
    us->callback([&sel] {
        sel.factory = [](binapi2::futures_usdm_api& c,
                         lib::result_sink& sink) -> boost::cobalt::task<int> {
            co_return co_await run_user_stream(c, sink);
        };
    });
}

} // namespace demo
