// SPDX-License-Identifier: Apache-2.0
//
// `result_sink` implementation that routes output through spdlog, matching
// the original `async-demo-cli` behaviour:
//
//   * `on_info`           → `spdlog::info`     (always visible)
//   * `on_response_json`  → the `"out"` logger (prefix-free stdout),
//                           gated on verbosity ≥ 1 to match the CLI's
//                           pre-refactor `-v` semantics.
//   * `on_error`          → `spdlog::error`    (plus http/binance code
//                           context lines when present; payload on debug).
//
// The `"out"` logger is registered elsewhere (the CLI's `init_logging`);
// this header just looks it up at use time.

#pragma once

#include "result_sink.hpp"

#include <spdlog/spdlog.h>

#include <string_view>

namespace binapi2::demo {

class spdlog_sink final : public result_sink
{
public:
    /// @param verbosity  matches the CLI's `-v`/`-vv`/`-vvv` count.
    ///                   JSON response bodies are emitted only when
    ///                   `verbosity ≥ 1`.
    explicit spdlog_sink(int verbosity) : verbosity_(verbosity) {}

    void on_info(std::string_view message) override
    {
        spdlog::info("{}", message);
    }

    void on_response_json(std::string_view pretty) override
    {
        if (verbosity_ < 1) return;
        if (auto logger = spdlog::get("out")) {
            logger->info("{}", pretty);
        }
    }

    void on_error(const binapi2::fapi::error& err) override
    {
        spdlog::error("{}", err.message);
        if (err.http_status)
            spdlog::error("  http_status: {}", err.http_status);
        if (err.binance_code)
            spdlog::error("  binance_code: {}", err.binance_code);
        if (!err.payload.empty())
            spdlog::debug("  payload: {}", err.payload);
    }

    void on_done(int /*rc*/) override {}

private:
    int verbosity_;
};

} // namespace binapi2::demo
