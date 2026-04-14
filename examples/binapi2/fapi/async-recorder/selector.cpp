// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — Stage 2: selector.

#include "selector.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <utility>

namespace binapi2::examples::async_recorder {

// ---------------------------------------------------------------------------
// signals_writer
// ---------------------------------------------------------------------------

signals_writer::signals_writer(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path.parent_path());
    out_.open(path, std::ios::app);
    if (!out_)
        spdlog::error("signals_writer: cannot open {}", path.string());
}

namespace {

const char* action_str(selector_action a)
{
    return a == selector_action::add ? "add" : "remove";
}

std::string to_iso(std::chrono::system_clock::time_point tp)
{
    const auto t = std::chrono::system_clock::to_time_t(tp);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        tp.time_since_epoch())
                        .count() %
                    1000;
    std::tm tm{};
    ::gmtime_r(&t, &tm);
    char buf[80];
    std::snprintf(buf, sizeof(buf),
                  "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long>(ms));
    return buf;
}

std::string escape_json(std::string_view s)
{
    std::string o;
    o.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:   o += c;
        }
    }
    return o;
}

} // namespace

void signals_writer::write(const selector_event& ev)
{
    if (!out_) return;
    std::ostringstream j;
    j << "{\"ts\":\"" << to_iso(ev.ts) << "\""
      << ",\"symbol\":\"" << escape_json(ev.symbol) << "\""
      << ",\"action\":\"" << action_str(ev.action) << "\""
      << ",\"score\":" << ev.score
      << ",\"reason\":\"" << escape_json(ev.reason) << "\"}\n";
    out_ << j.str();
    out_.flush();
    ++lines_;
}

// ---------------------------------------------------------------------------
// selector — pure logic
// ---------------------------------------------------------------------------

selector::selector(selector_config cfg) : cfg_(std::move(cfg))
{
    // Mandatory symbols are pre-admitted so they are never subject to
    // the remove rules. Their admitted_at is epoch so min-hold doesn't
    // matter for them (they're excluded from eviction anyway).
    for (const auto& s : cfg_.mandatory) {
        state_[s].active = true;
        active_.insert(s);
    }
}

double selector::score_symbol(const symbol_agg& agg) const
{
    // F5 scoring: weighted blend of rolling-window volume + trades on
    // 1m/5m/1h, with `events_total` retained as a low-cost liveness
    // fallback (matters during the warm-up window before the first
    // !ticker@arr tick has filled the windows).
    //
    // The two configured weights — `w_volume` and `w_trades` — are
    // multiplied across all three windows. `w_range` and `w_natr`
    // are not yet populated (no OHLC source on the live feed); they
    // contribute nothing until a future step plumbs them through.
    const double vol_score =
        cfg_.w_volume * (agg.vol_1m() + agg.vol_5m() + agg.vol_1h());
    const double trade_score =
        cfg_.w_trades * (agg.trades_1m() + agg.trades_5m() + agg.trades_1h());
    const double liveness =
        cfg_.w_volume * static_cast<double>(agg.events_total);

    return vol_score + trade_score + liveness;
}

std::vector<selector_event>
selector::tick(const aggregates_map& aggs,
               std::chrono::steady_clock::time_point now)
{
    std::vector<selector_event> diffs;

    // -- 1. Score every symbol we know about --------------------------------
    struct scored { std::string sym; double score; };
    std::vector<scored> scored_list;
    scored_list.reserve(aggs.size());
    for (const auto& [sym, agg] : aggs) {
        scored_list.push_back({ sym, score_symbol(agg) });
        state_[sym].last_score = scored_list.back().score;
    }

    std::sort(scored_list.begin(), scored_list.end(),
              [](const auto& a, const auto& b) { return a.score > b.score; });

    const std::unordered_set<std::string> mandatory_set(
        cfg_.mandatory.begin(), cfg_.mandatory.end());

    auto is_mandatory = [&](const std::string& s) {
        return mandatory_set.count(s) > 0;
    };

    // -- 2. Apply eviction rules -------------------------------------------
    // Walk the currently-active set. Mandatory symbols are never evicted.
    // Others check remove_score + cooloff + min_hold.
    std::vector<std::string> to_evict;
    for (const auto& sym : active_) {
        if (is_mandatory(sym)) continue;

        auto& st = state_[sym];
        const auto held_for = std::chrono::duration_cast<std::chrono::seconds>(
                                  now - st.admitted_at).count();
        if (static_cast<std::uint64_t>(held_for) < cfg_.min_hold_seconds)
            continue;  // min-hold still running, not eligible for eviction

        if (st.last_score >= cfg_.remove_score) {
            st.below_since.reset();
            continue;
        }

        // Score is below the remove threshold — start or continue cooloff.
        if (!st.below_since) st.below_since = now;
        const auto below_for = std::chrono::duration_cast<std::chrono::seconds>(
                                   now - *st.below_since).count();
        if (static_cast<std::uint64_t>(below_for) >= cfg_.cooloff_seconds) {
            to_evict.push_back(sym);
        }
    }
    for (const auto& sym : to_evict) {
        active_.erase(sym);
        state_[sym].active = false;
        state_[sym].below_since.reset();
        diffs.push_back({ std::chrono::system_clock::now(),
                          sym, selector_action::remove,
                          state_[sym].last_score,
                          "score<remove_score for cooloff" });
    }

    // -- 3. Apply admission rules ------------------------------------------
    // Walk scored_list top-down, admitting symbols with score >= add_score
    // until we reach max_active.
    for (const auto& s : scored_list) {
        if (active_.size() >= cfg_.max_active) break;
        if (active_.count(s.sym)) continue;
        if (s.score < cfg_.add_score) break;  // sorted, rest will also fail

        active_.insert(s.sym);
        auto& st = state_[s.sym];
        st.active = true;
        st.admitted_at = now;
        st.below_since.reset();
        diffs.push_back({ std::chrono::system_clock::now(),
                          s.sym, selector_action::add,
                          s.score,
                          "score>=add_score" });
    }

    // -- 4. Enforce min_active ---------------------------------------------
    // If the active set is below min_active and more candidates exist,
    // promote the top-scored non-active symbols even if they are below
    // add_score. This keeps the detail monitor fed when the market is
    // quiet.
    for (const auto& s : scored_list) {
        if (active_.size() >= cfg_.min_active) break;
        if (active_.count(s.sym)) continue;
        active_.insert(s.sym);
        auto& st = state_[s.sym];
        st.active = true;
        st.admitted_at = now;
        st.below_since.reset();
        diffs.push_back({ std::chrono::system_clock::now(),
                          s.sym, selector_action::add,
                          s.score,
                          "min_active floor" });
    }

    return diffs;
}

} // namespace binapi2::examples::async_recorder
