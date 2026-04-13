// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — institutional-grade recorder example.

/// @file config.hpp
/// @brief Runtime configuration for the async-recorder example.

#pragma once

#include <glaze/glaze.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::examples::async_recorder {

/// @brief Selector weights and rules. Loaded from YAML.
struct selector_config
{
    // -- scoring weights (per main timeframe) --------------------------------
    double w_volume{ 1.0 };
    double w_trades{ 1.0 };
    double w_range{ 1.0 };
    double w_natr{ 1.0 };

    // -- hysteresis / stability ----------------------------------------------
    double add_score{ 1.0 };     ///< score to admit a new symbol
    double remove_score{ 0.5 };  ///< score to evict an admitted symbol
    std::uint64_t min_hold_seconds{ 1800 };  ///< 30 min
    std::uint64_t cooloff_seconds{ 600 };    ///< 10 min

    // -- set bounds ----------------------------------------------------------
    std::size_t min_active{ 5 };
    std::size_t max_active{ 25 };

    // -- mandatory members ---------------------------------------------------
    std::vector<std::string> mandatory{ "BTCUSDT", "ETHUSDT" };
};

/// @brief Depth recording mode (see §6 of docs/binapi2/plans/async_recorder.md).
enum class depth_mode_t
{
    partial,  ///< @depth<N>@100ms (default, N ∈ {5,10,20})
    full      ///< @depth@100ms + local book reconstruction
};

/// @brief Root configuration.
struct recorder_config
{
    // -- layout --------------------------------------------------------------
    std::filesystem::path root_dir{ "./data" };
    std::filesystem::path selector_yaml{};  ///< path to selector YAML, empty = defaults

    // -- rotation ------------------------------------------------------------
    std::size_t rotation_size_bytes{ 512ULL * 1024 * 1024 };  ///< 512 MiB
    std::uint64_t rotation_seconds{ 3600 };                    ///< 1 h

    // -- depth ---------------------------------------------------------------
    depth_mode_t depth_mode{ depth_mode_t::partial };
    int depth_levels{ 20 };  ///< 5, 10, or 20 when mode == partial

    // -- feature flags -------------------------------------------------------
    bool with_depth{ false };   ///< record depth stream at all (off by default; storage-heavy)
    bool with_klines{ true };   ///< record per-symbol 1m klines

    // -- stats ---------------------------------------------------------------
    std::uint64_t stats_interval_seconds{ 10 };

    // -- network -------------------------------------------------------------
    bool testnet{ true };  ///< default to testnet for safety

    // -- loaded selector (populated by load_yaml) ----------------------------
    selector_config selector{};
};

/// @brief Parse CLI arguments into a recorder_config. Returns nullopt on --help.
std::optional<recorder_config> parse_args(int argc, char** argv);

/// @brief Print CLI usage to stdout.
void print_usage(const char* prog);

/// @brief Load selector YAML into cfg.selector. No-op if path is empty.
[[nodiscard]] bool load_selector_yaml(recorder_config& cfg, std::string& error);

/// @brief Serialize config to a human-readable string for --print-config.
std::string dump_config(const recorder_config& cfg);

} // namespace binapi2::examples::async_recorder

// Glaze reflection for YAML read/write of the selector config.
template<>
struct glz::meta<binapi2::examples::async_recorder::selector_config>
{
    using T = binapi2::examples::async_recorder::selector_config;
    static constexpr auto value = object(
        "w_volume", &T::w_volume,
        "w_trades", &T::w_trades,
        "w_range", &T::w_range,
        "w_natr", &T::w_natr,
        "add_score", &T::add_score,
        "remove_score", &T::remove_score,
        "min_hold_seconds", &T::min_hold_seconds,
        "cooloff_seconds", &T::cooloff_seconds,
        "min_active", &T::min_active,
        "max_active", &T::max_active,
        "mandatory", &T::mandatory);
};
