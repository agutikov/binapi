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

boost::cobalt::task<int> cmd_listen_key_start(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_user_data_streams(c, types::start_listen_key_request_t{});
}

boost::cobalt::task<int> cmd_listen_key_keepalive(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_user_data_streams(c, types::keepalive_listen_key_request_t{});
}

boost::cobalt::task<int> cmd_listen_key_close(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_user_data_streams(c, types::close_listen_key_request_t{});
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
    auto user_stream = c.create_user_stream();
    if (record_buffer) user_stream->connection().attach_buffer(*record_buffer);
    spdlog::info("requesting listen key...");
    auto key = co_await (*rest)->user_data_streams.async_execute(types::start_listen_key_request_t{});
    if (!key) { print_error(key.err); co_return 1; }
    spdlog::info("listen key: {}", key->listenKey);

    spdlog::info("subscribing to user data stream...");
    auto gen = user_stream->subscribe(key->listenKey);

    user_stream->enable_keepalive((*rest)->user_data_streams,
                                  std::chrono::minutes(30));
    spdlog::info("keepalive enabled (every 30m)");

    spdlog::info("connected, reading events...");
    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); break; }

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

    user_stream->stop_keepalive();
    spdlog::info("user stream ended");
    co_return 0;
}

} // namespace demo
