// SPDX-License-Identifier: Apache-2.0
//
// `request_capture` is the shared mutable state filled in by the worker
// thread (via `capture_sink` + `cfg.logger`) and read by the FTXUI render
// thread.
//
// One instance per "in-flight request slot" (currently one per command
// pane). The view holds a `shared_ptr<request_capture>` so the spawned
// coroutine outlives the view rebuild.
//
// Each call has two halves — request and response — and each half has
// three views: a raw HTTP serialization (request line / status line +
// headers + body), a pretty-printed JSON of the typed struct, and a
// parsed `glz::generic` for the tree view.

#pragma once

#include <glaze/glaze.hpp>
#include <glaze/json/generic.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace demo_ui {

/// One side of a request/response pair. Filled by either the cfg.logger
/// (raw HTTP) or by the typed-struct serializer (pretty JSON + parsed).
struct capture_side
{
    std::string raw;          ///< full HTTP message with request/status line + headers + body
    std::string pretty_json;  ///< typed struct serialized as pretty JSON
    std::shared_ptr<glz::generic> parsed_json;  ///< parsed JSON for the tree view
};

/// Capture state for a single REST or WS API call.
///
/// All string fields are guarded by `mtx`. The atomic `state` is the
/// transition flag the render path polls without taking the lock.
struct request_capture
{
    enum state_t { idle, running, done, failed };

    std::atomic<state_t> state{ idle };

    mutable std::mutex mtx;

    /// Free-form info messages from the executor (e.g. "pong",
    /// "status=200"). Newest at the back.
    std::string info_lines;

    capture_side request;
    capture_side response;

    /// Human-readable error message on failure. Empty on success.
    std::string error_message;
    int http_status = 0;
    int binance_code = 0;
};

/// Helper used by both the cfg.logger path (raw HTTP) and the
/// capture_sink path (typed JSON) to populate one side of the capture
/// without duplicating glaze-parse code.
inline void
fill_pretty_and_parsed(capture_side& dest, std::string_view pretty)
{
    dest.pretty_json.assign(pretty);
    auto parsed = std::make_shared<glz::generic>();
    if (glz::read_json(*parsed, pretty)) {
        // Parse failure → leave parsed_json null; the tree view will
        // render "(no parsed JSON)".
        dest.parsed_json.reset();
    } else {
        dest.parsed_json = std::move(parsed);
    }
}

} // namespace demo_ui
