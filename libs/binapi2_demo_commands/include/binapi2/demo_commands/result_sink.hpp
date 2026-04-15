// SPDX-License-Identifier: Apache-2.0
//
// Abstract sink interface for command output. The sink is how shared
// executors (`exec_market_data` / `exec_ws_signed` / `exec_stream` / …)
// talk back to their host surface.
//
// Each executor calls the sink in a strict order:
//
//   [on_info()*]                   — any number of free-form summary lines
//   on_response_json(pretty)       — exactly once on success
//   on_error(err)                  — exactly once on failure (instead of
//                                     on_response_json)
//   on_done(rc)                    — exactly once, at the very end
//
// Stream executors call `on_response_json` repeatedly (once per event) and
// `on_done(0)` when the generator is exhausted, or `on_error` + `on_done(1)`
// if the stream errors out.

#pragma once

#include <binapi2/fapi/error.hpp>

#include <string_view>

namespace binapi2::demo {

/// Abstract sink consumed by the shared executors.
///
/// The CLI implements this as `spdlog_sink` (prints via spdlog, gated on a
/// `--verbose` level). The UI implements it as a capture sink writing into
/// a `request_capture` / `stream_capture` struct that an FTXUI component
/// then renders.
///
/// Sinks are invoked from the cobalt coroutine thread. Implementations that
/// are read concurrently by a different thread (the UI's) must synchronise
/// internally; implementations with no cross-thread reader (the CLI's
/// spdlog sink) can be trivial.
class result_sink
{
public:
    virtual ~result_sink() = default;

    /// Short info / summary message from commands that print extra context
    /// beyond the raw JSON. Examples: `cmd_ping` emits `"pong"`,
    /// `cmd_time` emits `"server time: …"`, the WS API executors emit
    /// `"status=200"`.
    virtual void on_info(std::string_view message) = 0;

    /// Pretty-printed JSON of a successful response. Called exactly once
    /// by REST/WS executors and once per event by the stream executor.
    virtual void on_response_json(std::string_view pretty) = 0;

    /// Error details on failure. Called at most once per execution; the
    /// executor then calls `on_done(1)`.
    virtual void on_error(const binapi2::fapi::error& err) = 0;

    /// Final callback — always called exactly once, with the return code
    /// the executor intends to pass back (0 success / 1 failure).
    virtual void on_done(int rc) = 0;
};

} // namespace binapi2::demo
