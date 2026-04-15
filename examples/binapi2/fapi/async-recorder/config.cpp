// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — institutional-grade recorder example.

#include "config.hpp"

#include <CLI/CLI.hpp>
#include <glaze/yaml.hpp>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace binapi2::examples::async_recorder {

void print_usage(const char* /*prog*/)
{
    // Kept for header compatibility; CLI11 handles help output in parse_args.
}

std::optional<recorder_config> parse_args(int argc, char** argv)
{
    recorder_config cfg;
    bool want_print = false;
    bool full_depth = false;
    bool no_klines = false;
    bool live = false;
    bool testnet = false;
    std::string root_dir_str;
    std::string selector_yaml_str;
    std::string logfile_str;

    CLI::App app{ "async-recorder — institutional-grade recorder example" };
    app.set_help_flag("-h,--help", "Print this help and exit");

    app.add_option("--root", root_dir_str,
        "Output root directory")->default_str("./data");
    app.add_option("--selector", selector_yaml_str,
        "Selector config YAML (default: built-in)");
    app.add_option("--rotate-size", cfg.rotation_size_bytes,
        "Rotation size in bytes")->capture_default_str();
    app.add_option("--rotate-seconds", cfg.rotation_seconds,
        "Rotation time in seconds")->capture_default_str();
    app.add_option("--depth-levels", cfg.depth_levels,
        "Partial-book depth levels")
        ->check(CLI::IsMember({ 5, 10, 20 }))
        ->capture_default_str();
    app.add_flag("--full-depth", full_depth,
        "Use full diff depth stream instead of partial");
    app.add_option("--depth-resnap-seconds", cfg.depth_resnap_seconds,
        "Periodic depth re-snapshot interval (full-depth mode)")->capture_default_str();
    app.add_flag("--with-depth", cfg.with_depth,
        "Record depth at all (default: off)");
    app.add_flag("--no-klines", no_klines,
        "Disable per-symbol 1m kline recording");
    app.add_option("--stats-seconds", cfg.stats_interval_seconds,
        "Stats tick interval")->capture_default_str();
    app.add_option("--loglevel", cfg.loglevel,
        "spdlog level: trace/debug/info/warn/error/critical/off")
        ->check(CLI::IsMember({ "trace","debug","info","warn","error","critical","off" }))
        ->capture_default_str();
    app.add_option("--logfile", logfile_str,
        "Also write logs to this file (truncated on start)");
    app.add_option("--debug-stream", cfg.debug_stream,
        "Run single-stream debug screener: bookTicker|markPriceArr|tickerArr")
        ->check(CLI::IsMember({ "bookTicker","markPriceArr","tickerArr" }));
    app.add_flag("--live", live,    "Use production endpoints");
    app.add_flag("--testnet", testnet, "Use testnet endpoints (default)");
    app.add_flag("--print-config", want_print,
        "Parse flags and YAML, dump config, exit");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));
    }

    if (!root_dir_str.empty())       cfg.root_dir = root_dir_str;
    if (!selector_yaml_str.empty())  cfg.selector_yaml = selector_yaml_str;
    if (!logfile_str.empty())        cfg.logfile = logfile_str;
    if (full_depth) cfg.depth_mode = depth_mode_t::full;
    if (no_klines)  cfg.with_klines = false;
    if (live)       cfg.testnet = false;
    if (testnet)    cfg.testnet = true;

    std::string error;
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
      << "  depth_resnap_secs  = " << cfg.depth_resnap_seconds << "\n"
      << "  with_depth         = " << (cfg.with_depth ? "yes" : "no") << "\n"
      << "  with_klines        = " << (cfg.with_klines ? "yes" : "no") << "\n"
      << "  stats_seconds      = " << cfg.stats_interval_seconds << " s\n"
      << "  loglevel           = " << cfg.loglevel << "\n"
      << "  logfile            = " << (cfg.logfile.empty() ? "<stdout only>" : cfg.logfile.string()) << "\n"
      << "  debug_stream       = " << (cfg.debug_stream.empty() ? "<off>" : cfg.debug_stream) << "\n"
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
