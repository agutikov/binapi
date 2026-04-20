// SPDX-License-Identifier: Apache-2.0
//
// Market-stream command registry. Parallel to `rest/commands.hpp` and
// `ws/commands.hpp`, but the action on Enter is start/stop (not one-shot
// run), and every subscription targets the same per-stream `stream_capture`.

#pragma once

#include "../app_state.hpp"
#include "../worker.hpp"
#include "stream_capture.hpp"

#include <boost/cobalt/task.hpp>

#include <memory>
#include <span>
#include <string>

namespace demo_ui::streams {

/// Form-field visibility for streams. Mirrors `rest::form_kind` but only
/// covers the shapes actually used by market-stream subscriptions.
enum class form_kind
{
    no_args,
    symbol,
    kline,        // symbol + interval
    pair_kline,   // pair + interval
    levels,       // symbol + int levels (5 / 10 / 20)
    speed,        // symbol + speed string (100ms / 250ms)
};

/// Sticky input state, one instance per view. Individual streams read
/// only the fields their `form_kind` advertises.
struct form_state
{
    std::string symbol   = "BTCUSDT";
    std::string pair     = "BTCUSDT";
    std::string interval = "1m";
    std::string levels   = "10";
    std::string speed    = "100ms";
};

bool needs_symbol  (form_kind k);
bool needs_pair    (form_kind k);
bool needs_interval(form_kind k);
bool needs_levels  (form_kind k);
bool needs_speed   (form_kind k);

/// Context handed to each command's start function.
struct start_ctx
{
    worker& wrk;
    app_state& state;
    std::shared_ptr<stream_capture> cap;
    const form_state& form;
};

/// Registry entry. Start function spawns the subscription coroutine.
/// Stop is universal — setting `cap->stop = true` breaks the generator
/// loop in `lib::exec_stream`.
struct stream_command
{
    const char* name;
    const char* description;
    form_kind   form;
    void (*start)(const start_ctx&);
};

std::span<const stream_command> stream_commands();

} // namespace demo_ui::streams
