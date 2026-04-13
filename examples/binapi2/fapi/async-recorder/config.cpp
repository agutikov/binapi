// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — institutional-grade recorder example.

#include "config.hpp"

#include <glaze/yaml.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace binapi2::examples::async_recorder {

namespace {

bool parse_int(const char* s, int& out)
{
    try {
        out = std::stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_size(const char* s, std::size_t& out)
{
    try {
        out = static_cast<std::size_t>(std::stoull(s));
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_u64(const char* s, std::uint64_t& out)
{
    try {
        out = static_cast<std::uint64_t>(std::stoull(s));
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

void print_usage(const char* prog)
{
    std::fprintf(stdout,
        "Usage: %s [flags]\n"
        "\n"
        "Flags:\n"
        "  --root <dir>              Output root directory (default: ./data)\n"
        "  --selector <file.yaml>    Selector config YAML (default: built-in)\n"
        "  --rotate-size <bytes>     Rotation size in bytes (default: 512 MiB)\n"
        "  --rotate-seconds <s>      Rotation time in seconds (default: 3600)\n"
        "  --depth-levels {5,10,20}  Partial-book depth levels (default: 20)\n"
        "  --full-depth              Use full diff depth stream instead of partial\n"
        "  --with-depth              Record depth at all (default: off)\n"
        "  --no-klines               Disable per-symbol 1m kline recording\n"
        "  --stats-seconds <s>       Stats tick interval (default: 10)\n"
        "  --live                    Use production endpoints (default: testnet)\n"
        "  --testnet                 Use testnet endpoints (default)\n"
        "  --print-config            Parse flags and YAML, dump config, exit\n"
        "  -h, --help                Print this help and exit\n",
        prog);
}

std::optional<recorder_config> parse_args(int argc, char** argv)
{
    recorder_config cfg;
    bool want_print = false;
    std::string error;

    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        auto need_arg = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "error: %s requires an argument\n", name);
                std::exit(2);
            }
            return argv[++i];
        };

        if (a == "-h" || a == "--help") {
            print_usage(argv[0]);
            return std::nullopt;
        }
        else if (a == "--root") { cfg.root_dir = need_arg("--root"); }
        else if (a == "--selector") { cfg.selector_yaml = need_arg("--selector"); }
        else if (a == "--rotate-size") {
            if (!parse_size(need_arg("--rotate-size"), cfg.rotation_size_bytes)) {
                std::fprintf(stderr, "error: --rotate-size needs an integer\n");
                std::exit(2);
            }
        }
        else if (a == "--rotate-seconds") {
            if (!parse_u64(need_arg("--rotate-seconds"), cfg.rotation_seconds)) {
                std::fprintf(stderr, "error: --rotate-seconds needs an integer\n");
                std::exit(2);
            }
        }
        else if (a == "--depth-levels") {
            int n = 0;
            if (!parse_int(need_arg("--depth-levels"), n) ||
                (n != 5 && n != 10 && n != 20)) {
                std::fprintf(stderr, "error: --depth-levels must be 5, 10, or 20\n");
                std::exit(2);
            }
            cfg.depth_levels = n;
            cfg.depth_mode = depth_mode_t::partial;
        }
        else if (a == "--full-depth") { cfg.depth_mode = depth_mode_t::full; }
        else if (a == "--with-depth") { cfg.with_depth = true; }
        else if (a == "--no-klines") { cfg.with_klines = false; }
        else if (a == "--stats-seconds") {
            if (!parse_u64(need_arg("--stats-seconds"), cfg.stats_interval_seconds)) {
                std::fprintf(stderr, "error: --stats-seconds needs an integer\n");
                std::exit(2);
            }
        }
        else if (a == "--live") { cfg.testnet = false; }
        else if (a == "--testnet") { cfg.testnet = true; }
        else if (a == "--print-config") { want_print = true; }
        else {
            std::fprintf(stderr, "error: unknown flag: %.*s\n",
                         static_cast<int>(a.size()), a.data());
            print_usage(argv[0]);
            std::exit(2);
        }
    }

    if (!load_selector_yaml(cfg, error)) {
        std::fprintf(stderr, "error: %s\n", error.c_str());
        std::exit(2);
    }

    if (want_print) {
        std::fputs(dump_config(cfg).c_str(), stdout);
        return std::nullopt;
    }

    return cfg;
}

bool load_selector_yaml(recorder_config& cfg, std::string& error)
{
    if (cfg.selector_yaml.empty()) return true;

    std::ifstream f(cfg.selector_yaml);
    if (!f) {
        error = "cannot open selector YAML: " + cfg.selector_yaml.string();
        return false;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string buf = ss.str();

    // Glaze's read_yaml appends to existing vectors instead of replacing.
    // Start from a fresh object with empty collections; if the YAML omits a
    // collection, restore the built-in default afterwards.
    selector_config fresh{};
    fresh.mandatory.clear();

    auto ec = glz::read_yaml(fresh, buf);
    if (ec) {
        error = "YAML parse error: " + glz::format_error(ec, buf);
        return false;
    }
    if (fresh.mandatory.empty())
        fresh.mandatory = selector_config{}.mandatory;

    cfg.selector = std::move(fresh);
    return true;
}

std::string dump_config(const recorder_config& cfg)
{
    std::ostringstream o;
    o << "async-recorder config:\n"
      << "  root_dir           = " << cfg.root_dir.string() << "\n"
      << "  selector_yaml      = " << (cfg.selector_yaml.empty() ? "<defaults>" : cfg.selector_yaml.string()) << "\n"
      << "  rotation_size      = " << cfg.rotation_size_bytes << " B\n"
      << "  rotation_seconds   = " << cfg.rotation_seconds << " s\n"
      << "  depth_mode         = " << (cfg.depth_mode == depth_mode_t::partial ? "partial" : "full") << "\n"
      << "  depth_levels       = " << cfg.depth_levels << "\n"
      << "  with_depth         = " << (cfg.with_depth ? "yes" : "no") << "\n"
      << "  with_klines        = " << (cfg.with_klines ? "yes" : "no") << "\n"
      << "  stats_seconds      = " << cfg.stats_interval_seconds << " s\n"
      << "  network            = " << (cfg.testnet ? "testnet" : "live") << "\n"
      << "  selector:\n"
      << "    w_volume         = " << cfg.selector.w_volume << "\n"
      << "    w_trades         = " << cfg.selector.w_trades << "\n"
      << "    w_range          = " << cfg.selector.w_range << "\n"
      << "    w_natr           = " << cfg.selector.w_natr << "\n"
      << "    add_score        = " << cfg.selector.add_score << "\n"
      << "    remove_score     = " << cfg.selector.remove_score << "\n"
      << "    min_hold_seconds = " << cfg.selector.min_hold_seconds << "\n"
      << "    cooloff_seconds  = " << cfg.selector.cooloff_seconds << "\n"
      << "    min_active       = " << cfg.selector.min_active << "\n"
      << "    max_active       = " << cfg.selector.max_active << "\n"
      << "    mandatory        = [";
    for (std::size_t i = 0; i < cfg.selector.mandatory.size(); ++i) {
        if (i) o << ", ";
        o << cfg.selector.mandatory[i];
    }
    o << "]\n";
    return o.str();
}

} // namespace binapi2::examples::async_recorder
