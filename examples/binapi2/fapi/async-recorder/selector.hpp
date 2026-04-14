// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 2: selector.

/// @file selector.hpp
/// @brief Instrument selector: scores aggregates, applies hysteresis +
///        min-hold + bounds + mandatory, emits add/remove signals.

#pragma once

#include "aggregates.hpp"
#include "config.hpp"

#include <boost/cobalt/task.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace binapi2::examples::async_recorder {

class status_reporter;

/// @brief Direction of a selector signal — symbol admitted to or evicted
///        from the active set.
enum class selector_action
{
    add,
    remove,
};

/// @brief Emitted signal. Serialised to `selector/signals.jsonl`.
struct selector_event
{
    std::chrono::system_clock::time_point ts;
    std::string symbol;
    selector_action action;
    double score{ 0.0 };
    std::string reason;
};

/// @brief Append-only JSONL writer for `selector/signals.jsonl`.
///        Synchronous; the file is tiny (one line per add/remove).
class signals_writer
{
public:
    explicit signals_writer(const std::filesystem::path& path);

    void write(const selector_event& ev);

    [[nodiscard]] std::size_t lines_written() const noexcept { return lines_; }

private:
    std::ofstream out_;
    std::size_t lines_{ 0 };
};

/// @brief Snapshot of per-symbol selector state, maintained across ticks.
struct selector_symbol_state
{
    bool active{ false };
    double last_score{ 0.0 };
    /// @brief Time the symbol was (most recently) admitted. Used by
    ///        the min-hold rule.
    std::chrono::steady_clock::time_point admitted_at{};
    /// @brief When the symbol first dropped below `remove_score`, or
    ///        nullopt if it currently is above. Used by the cooloff rule.
    std::optional<std::chrono::steady_clock::time_point> below_since{};
};

/// @brief Pure logic core. Fed an aggregates snapshot, yields the
///        add/remove diff for this tick. Separated from the coroutine
///        driver so unit tests can feed it synthetic inputs with a
///        virtual clock.
class selector
{
public:
    explicit selector(selector_config cfg);

    /// @brief Compute the new active set from `aggs` and emit the
    ///        difference against the previous tick as a vector of
    ///        selector_events. `now` is the logical clock — pass
    ///        `steady_clock::now()` in the live path, or a stepped
    ///        value in tests.
    std::vector<selector_event>
    tick(const aggregates_map& aggs,
         std::chrono::steady_clock::time_point now);

    [[nodiscard]] const std::unordered_set<std::string>& active() const noexcept
    {
        return active_;
    }

    [[nodiscard]] const selector_config& cfg() const noexcept { return cfg_; }

private:
    double score_symbol(const symbol_agg& agg) const;

    selector_config cfg_;
    std::unordered_map<std::string, selector_symbol_state> state_{};
    std::unordered_set<std::string> active_{};
};

/// @brief Coroutine driver: on an interval timer, take an aggregates
///        snapshot, run one selector tick, and write each emitted
///        event to the signals file. Returns when the status_reporter
///        is closed (via its timer cancellation path). Registers a
///        "selector" status source.
boost::cobalt::task<void>
selector_run(const recorder_config& cfg,
             const aggregates_map& aggs,
             selector& sel,
             signals_writer& signals,
             status_reporter& status);

} // namespace binapi2::examples::async_recorder
