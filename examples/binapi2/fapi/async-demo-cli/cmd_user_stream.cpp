// SPDX-License-Identifier: Apache-2.0
//
// User data stream commands — listen key management and real-time account events.
// Demonstrates: async_execute() for listen key lifecycle, and user_streams
// subscribe() returning a variant generator for all event types.

#include "cmd_user_stream.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

#include <variant>

namespace demo {

namespace types = binapi2::fapi::types;

boost::cobalt::task<int> cmd_listen_key_start(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    spdlog::debug("requesting new listen key");
    auto r = co_await (*rest)->user_data_streams.async_execute(types::start_listen_key_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("listenKey: {}", r->listenKey);
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_listen_key_keepalive(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    spdlog::debug("sending listen key keepalive");
    auto r = co_await (*rest)->user_data_streams.async_execute(types::keepalive_listen_key_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("keepalive ok");
    co_return 0;
}

boost::cobalt::task<int> cmd_listen_key_close(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    spdlog::debug("closing listen key");
    auto r = co_await (*rest)->user_data_streams.async_execute(types::close_listen_key_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("listen key closed");
    co_return 0;
}

namespace {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace

boost::cobalt::task<int> cmd_user_stream(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto user_streams = c.create_user_streams();
    spdlog::info("requesting listen key...");
    auto key = co_await (*rest)->user_data_streams.async_execute(types::start_listen_key_request_t{});
    if (!key) { print_error(key.err); co_return 1; }
    spdlog::info("listen key: {}", key->listenKey);

    spdlog::info("subscribing to user data stream...");
    auto gen = user_streams->subscribe(key->listenKey);

    spdlog::info("connected, reading events...");
    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }

        bool should_stop = std::visit(overloaded{
            [](const types::account_update_event_t& e) {
                spdlog::info("[account_update] event_time={}", e.event_time);
                if (verbosity >= 1) print_json(e);
                return false;
            },
            [](const types::order_trade_update_event_t& e) {
                spdlog::info("[order_trade_update] event_time={}", e.event_time);
                if (verbosity >= 1) print_json(e);
                return false;
            },
            [](const types::margin_call_event_t& e) {
                spdlog::warn("[margin_call] event_time={}", e.event_time);
                if (verbosity >= 1) print_json(e);
                return false;
            },
            [](const types::listen_key_expired_event_t& e) {
                spdlog::error("[listen_key_expired] event_time={}", e.event_time);
                return true;
            },
            [](const types::account_config_update_event_t& e) {
                spdlog::info("[account_config_update] event_time={}", e.event_time);
                if (verbosity >= 1) print_json(e);
                return false;
            },
            [](const types::trade_lite_event_t& e) {
                spdlog::info("[trade_lite] event_time={}", e.event_time);
                if (verbosity >= 1) print_json(e);
                return false;
            },
            [](const auto& e) {
                spdlog::info("[other event] event_time={}", e.event_time);
                return false;
            }
        }, *event);

        if (should_stop) break;
    }

    spdlog::info("user stream ended");
    co_return 0;
}

} // namespace demo
