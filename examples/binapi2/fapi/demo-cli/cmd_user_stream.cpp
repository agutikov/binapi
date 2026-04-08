// SPDX-License-Identifier: Apache-2.0
//
// User data stream commands — listen key management and real-time account events.
// Demonstrates: user_data_stream_service for listen key lifecycle, and
// user_streams with the handlers struct for receiving all event types.

#include "cmd_user_stream.hpp"

#include <binapi2/fapi/client.hpp>

#include <spdlog/spdlog.h>

namespace demo {

int cmd_listen_key_start(const args_t& /*args*/)
{
    binapi2::fapi::client client{ make_config() };

    spdlog::debug("requesting new listen key");
    auto r = client.user_data_streams.start();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("listenKey: {}", r->listenKey);
    if (verbosity >= 1) print_json(*r);
    return 0;
}

int cmd_listen_key_keepalive(const args_t& /*args*/)
{
    binapi2::fapi::client client{ make_config() };

    spdlog::debug("sending listen key keepalive");
    auto r = client.user_data_streams.keepalive();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("keepalive ok");
    return 0;
}

int cmd_listen_key_close(const args_t& /*args*/)
{
    binapi2::fapi::client client{ make_config() };

    spdlog::debug("closing listen key");
    auto r = client.user_data_streams.close();
    if (!r) { print_error(r.err); return 1; }

    spdlog::info("listen key closed");
    return 0;
}

int cmd_user_stream(const args_t& /*args*/)
{
    binapi2::fapi::client client{ make_config() };

    spdlog::info("requesting listen key...");
    auto key = client.user_data_streams.start();
    if (!key) { print_error(key.err); return 1; }
    spdlog::info("listen key: {}", key->listenKey);

    auto& stream = client.user_streams();
    spdlog::info("connecting to user data stream...");
    if (auto c = stream.connect(key->listenKey); !c) {
        print_error(c.err); return 1;
    }
    spdlog::info("connected, reading events...");

    binapi2::fapi::streams::user_streams::handlers h;
    h.on_account_update = [](const auto& e) {
        spdlog::info("[account_update] event_time={}", e.event_time);
        if (verbosity >= 1) print_json(e);
        return false;
    };
    h.on_order_trade_update = [](const auto& e) {
        spdlog::info("[order_trade_update] event_time={}", e.event_time);
        if (verbosity >= 1) print_json(e);
        return false;
    };
    h.on_margin_call = [](const auto& e) {
        spdlog::warn("[margin_call] event_time={}", e.event_time);
        if (verbosity >= 1) print_json(e);
        return false;
    };
    h.on_listen_key_expired = [](const auto& e) {
        spdlog::error("[listen_key_expired] event_time={}", e.event_time);
        return true;
    };
    h.on_account_config_update = [](const auto& e) {
        spdlog::info("[account_config_update] event_time={}", e.event_time);
        if (verbosity >= 1) print_json(e);
        return false;
    };
    h.on_trade_lite = [](const auto& e) {
        spdlog::info("[trade_lite] event_time={}", e.event_time);
        if (verbosity >= 1) print_json(e);
        return false;
    };

    auto loop = stream.read_loop(h);
    (void)stream.close();
    if (!loop) { print_error(loop.err); return 1; }
    return 0;
}

} // namespace demo
