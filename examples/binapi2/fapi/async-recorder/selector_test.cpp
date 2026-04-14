// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — selector unit tests.

#include "aggregates.hpp"
#include "selector.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <unordered_set>

using namespace binapi2::examples::async_recorder;
using clk = std::chrono::steady_clock;

namespace {

selector_config make_cfg()
{
    selector_config c;
    c.w_volume = 1.0;
    c.add_score = 100.0;
    c.remove_score = 50.0;
    c.min_hold_seconds = 60;
    c.cooloff_seconds = 30;
    c.min_active = 2;
    c.max_active = 5;
    c.mandatory = { "BTCUSDT", "ETHUSDT" };
    return c;
}

void set_events(aggregates_map& m, const std::string& sym, std::size_t n)
{
    m[sym].events_total = n;
}

} // namespace

TEST(Selector, MandatoryAlwaysActive)
{
    selector s(make_cfg());
    EXPECT_TRUE(s.active().count("BTCUSDT"));
    EXPECT_TRUE(s.active().count("ETHUSDT"));
    EXPECT_EQ(s.active().size(), 2u);

    aggregates_map empty;
    auto diffs = s.tick(empty, clk::now());
    EXPECT_TRUE(diffs.empty());  // no add/remove; mandatory already in
    EXPECT_TRUE(s.active().count("BTCUSDT"));
    EXPECT_TRUE(s.active().count("ETHUSDT"));
}

TEST(Selector, AdmitsAboveThreshold)
{
    selector s(make_cfg());
    aggregates_map m;
    set_events(m, "SOLUSDT", 150);
    set_events(m, "ADAUSDT", 200);
    set_events(m, "DOTUSDT", 50);  // below add_score

    auto diffs = s.tick(m, clk::now());

    EXPECT_TRUE(s.active().count("SOLUSDT"));
    EXPECT_TRUE(s.active().count("ADAUSDT"));
    EXPECT_FALSE(s.active().count("DOTUSDT"));

    // Active set: BTCUSDT, ETHUSDT (mandatory) + SOL + ADA = 4.
    EXPECT_EQ(s.active().size(), 4u);

    // Both adds were reported.
    int adds = 0;
    for (const auto& d : diffs)
        if (d.action == selector_action::add) ++adds;
    EXPECT_EQ(adds, 2);
}

TEST(Selector, MaxActiveCap)
{
    auto cfg = make_cfg();
    cfg.max_active = 3;  // tight cap
    selector s(cfg);

    aggregates_map m;
    set_events(m, "AAA", 500);
    set_events(m, "BBB", 400);
    set_events(m, "CCC", 300);
    set_events(m, "DDD", 200);

    s.tick(m, clk::now());
    // BTCUSDT + ETHUSDT are mandatory (pre-admitted, 2). max_active=3 means
    // only one more slot is free, so only the top-scored AAA should be
    // admitted on top of the mandatory pair.
    EXPECT_EQ(s.active().size(), 3u);
    EXPECT_TRUE(s.active().count("AAA"));
    EXPECT_FALSE(s.active().count("BBB"));
}

TEST(Selector, MinHoldPreventsImmediateEviction)
{
    selector s(make_cfg());
    aggregates_map m;
    set_events(m, "SOLUSDT", 200);

    auto t0 = clk::time_point(std::chrono::seconds(0));
    s.tick(m, t0);
    EXPECT_TRUE(s.active().count("SOLUSDT"));

    // Score drops below remove_score immediately.
    set_events(m, "SOLUSDT", 10);

    // Tick 10 s later — still within min_hold (60s) + cooloff (30s). No evict.
    s.tick(m, t0 + std::chrono::seconds(10));
    EXPECT_TRUE(s.active().count("SOLUSDT"));

    // Tick 70 s later — past min_hold but only 10 s of cooloff. Still held.
    s.tick(m, t0 + std::chrono::seconds(70));
    EXPECT_TRUE(s.active().count("SOLUSDT"));

    // Tick 100 s later — past both min_hold (60) and cooloff (30). Evicted.
    s.tick(m, t0 + std::chrono::seconds(100));
    EXPECT_FALSE(s.active().count("SOLUSDT"));
}

TEST(Selector, HysteresisNoFlappingInDeadband)
{
    selector s(make_cfg());
    aggregates_map m;

    auto t0 = clk::time_point(std::chrono::seconds(0));

    // Admit above add_score.
    set_events(m, "SOLUSDT", 200);
    s.tick(m, t0);
    EXPECT_TRUE(s.active().count("SOLUSDT"));

    // Score drops into the deadband (remove < score < add). No flap.
    set_events(m, "SOLUSDT", 75);
    for (int i = 0; i < 20; ++i) {
        s.tick(m, t0 + std::chrono::seconds(10 + i));
    }
    EXPECT_TRUE(s.active().count("SOLUSDT"));
}

TEST(Selector, MinActiveFloorPromotesWeakCandidates)
{
    auto cfg = make_cfg();
    cfg.mandatory = {};            // disable mandatory for this test
    cfg.min_active = 4;
    cfg.add_score = 1000.0;        // unreachable
    selector s(cfg);

    aggregates_map m;
    set_events(m, "AAA", 10);
    set_events(m, "BBB", 5);
    set_events(m, "CCC", 3);
    set_events(m, "DDD", 1);
    set_events(m, "EEE", 0);

    s.tick(m, clk::now());
    EXPECT_EQ(s.active().size(), 4u);
    EXPECT_TRUE(s.active().count("AAA"));
    EXPECT_TRUE(s.active().count("BBB"));
    EXPECT_TRUE(s.active().count("CCC"));
    EXPECT_TRUE(s.active().count("DDD"));
    EXPECT_FALSE(s.active().count("EEE"));
}

// =====================================================================
// F5 — tf_window mechanics + delta-based scoring
// =====================================================================

TEST(TfWindow, AccumulatesAcrossBuckets)
{
    tf_window w(60, std::chrono::seconds(1));    // 60 × 1 s = 1 min

    auto t0 = clk::time_point(std::chrono::seconds(1000));
    w.update(t0,                                10.0, 5.0);
    w.update(t0 + std::chrono::seconds(1),      20.0, 7.0);
    w.update(t0 + std::chrono::seconds(2),      30.0, 9.0);

    // All three buckets are inside the 1-minute window; all contribute.
    EXPECT_DOUBLE_EQ(w.vol_sum(), 60.0);
    EXPECT_DOUBLE_EQ(w.trades_sum(), 21.0);
}

TEST(TfWindow, SlidesOutOldBuckets)
{
    tf_window w(5, std::chrono::seconds(1));     // 5 × 1 s = 5 s window

    auto t0 = clk::time_point(std::chrono::seconds(1000));
    w.update(t0 + std::chrono::seconds(0), 10.0, 0.0);
    w.update(t0 + std::chrono::seconds(1), 10.0, 0.0);
    w.update(t0 + std::chrono::seconds(2), 10.0, 0.0);
    w.update(t0 + std::chrono::seconds(3), 10.0, 0.0);
    w.update(t0 + std::chrono::seconds(4), 10.0, 0.0);
    EXPECT_DOUBLE_EQ(w.vol_sum(), 50.0);

    // 6 seconds later — every original bucket has slid out.
    w.update(t0 + std::chrono::seconds(11), 1.0, 0.0);
    EXPECT_DOUBLE_EQ(w.vol_sum(), 1.0);
}

TEST(TfWindow, BigGapWipesWindow)
{
    tf_window w(60, std::chrono::seconds(1));
    auto t0 = clk::time_point(std::chrono::seconds(1000));

    w.update(t0,                                  100.0, 0.0);
    w.update(t0 + std::chrono::seconds(1),         50.0, 0.0);
    EXPECT_DOUBLE_EQ(w.vol_sum(), 150.0);

    // Skip ahead by far more than n_buckets * bucket_seconds — every
    // old bucket should be gone, leaving only the new sample.
    w.update(t0 + std::chrono::hours(1), 7.0, 0.0);
    EXPECT_DOUBLE_EQ(w.vol_sum(), 7.0);
}

TEST(Selector, ScoresFromTfWindows)
{
    auto cfg = make_cfg();
    cfg.mandatory = {};
    cfg.w_volume = 1.0;
    cfg.w_trades = 0.0;
    cfg.add_score = 50.0;
    cfg.remove_score = 25.0;
    cfg.max_active = 5;
    cfg.min_active = 0;
    selector s(cfg);

    aggregates_map m;
    auto t0 = clk::time_point(std::chrono::seconds(1000));

    // SOLUSDT — drip 100 units of volume per second for 5 seconds.
    // After 5 ticks, vol_1m = 500, vol_5m = 500, vol_1h = 500 → total
    // 1500. With w_volume=1 the score is 1500 → above add_score.
    auto& sol = m["SOLUSDT"];
    for (int i = 0; i < 5; ++i)
        sol.win_1m.update(t0 + std::chrono::seconds(i), 100.0, 0.0),
        sol.win_5m.update(t0 + std::chrono::seconds(i), 100.0, 0.0),
        sol.win_1h.update(t0 + std::chrono::seconds(i), 100.0, 0.0);

    // QUIETUSDT — no volume, no events, no admission.
    m["QUIETUSDT"];

    s.tick(m, t0 + std::chrono::seconds(5));
    EXPECT_TRUE(s.active().count("SOLUSDT"));
    EXPECT_FALSE(s.active().count("QUIETUSDT"));
}

// =====================================================================
// F6 — synthetic 1m klines + NATR
// =====================================================================

TEST(Bar1m, ReopenExtendRange)
{
    bar_1m bar;
    bar.reopen(1000, 100.0);
    EXPECT_TRUE(bar.primed);
    EXPECT_DOUBLE_EQ(bar.open, 100.0);
    EXPECT_DOUBLE_EQ(bar.close, 100.0);

    // Intra-bar observations via extend_range (bookTicker mid path):
    // should move high/low but not open/close.
    bar.extend_range(105.0);
    bar.extend_range(95.0);
    EXPECT_DOUBLE_EQ(bar.high, 105.0);
    EXPECT_DOUBLE_EQ(bar.low, 95.0);
    EXPECT_DOUBLE_EQ(bar.open, 100.0);
    EXPECT_DOUBLE_EQ(bar.close, 100.0);

    // Intra-bar trade print (ticker last_price path): must move close
    // and may extend high/low.
    bar.update_from_trade(108.0);
    EXPECT_DOUBLE_EQ(bar.close, 108.0);
    EXPECT_DOUBLE_EQ(bar.high, 108.0);
    EXPECT_DOUBLE_EQ(bar.low, 95.0);
    EXPECT_DOUBLE_EQ(bar.open, 100.0);
}

TEST(TrWindow, PrimesAfterNSamples)
{
    tr_window trw(3);
    EXPECT_FALSE(trw.primed());
    EXPECT_DOUBLE_EQ(trw.atr(), 0.0);

    trw.push(1.0);
    EXPECT_FALSE(trw.primed());
    EXPECT_DOUBLE_EQ(trw.atr(), 1.0);   // mean so far

    trw.push(2.0);
    EXPECT_FALSE(trw.primed());
    EXPECT_DOUBLE_EQ(trw.atr(), 1.5);

    trw.push(3.0);
    EXPECT_TRUE(trw.primed());
    EXPECT_DOUBLE_EQ(trw.atr(), 2.0);

    // Ring wrap: oldest (1.0) gets replaced by 4.0 → mean(2,3,4) = 3.
    trw.push(4.0);
    EXPECT_TRUE(trw.primed());
    EXPECT_DOUBLE_EQ(trw.atr(), 3.0);
}

TEST(SymbolAgg, NatrFromSyntheticBars)
{
    // Drive 4 closed bars with deterministic OHLC into a fresh
    // symbol_agg's bar pipeline, then assert the computed NATR matches
    // the hand-calculated value from the bar history.
    symbol_agg agg;

    // Minute 0 bar: open/high/low/close = 100 / 102 / 98 / 101
    agg.current_bar.reopen(0, 100.0);
    agg.current_bar.extend_range(102.0);
    agg.current_bar.extend_range(98.0);
    agg.current_bar.update_from_trade(101.0);

    // Close bar 0 → open bar 1 at 101.
    agg.close_bar_and_open_next(1, 101.0);
    // Shape bar 1: O/H/L/C = 101 / 104 / 100 / 103
    agg.current_bar.extend_range(104.0);
    agg.current_bar.extend_range(100.0);
    agg.current_bar.update_from_trade(103.0);

    // Close bar 1 → bar 2 at 103.
    agg.close_bar_and_open_next(2, 103.0);
    // Bar 2: O/H/L/C = 103 / 105 / 102 / 104
    agg.current_bar.extend_range(105.0);
    agg.current_bar.extend_range(102.0);
    agg.current_bar.update_from_trade(104.0);

    // Close bar 2 → bar 3 at 104.
    agg.close_bar_and_open_next(3, 104.0);
    // Bar 3: O/H/L/C = 104 / 107 / 103 / 106
    agg.current_bar.extend_range(107.0);
    agg.current_bar.extend_range(103.0);
    agg.current_bar.update_from_trade(106.0);

    // Close bar 3 → bar 4 at 106. This is the moment NATR reflects
    // the true-range history of bars 0..3.
    agg.close_bar_and_open_next(4, 106.0);

    // Hand-calculated TRs:
    //   bar 0 has no prev_close, so TR = high-low = 4.0
    //   bar 1: range = 4.0;  |104-101|=3; |100-101|=1  → TR = 4.0
    //   bar 2: range = 3.0;  |105-103|=2; |102-103|=1  → TR = 3.0
    //   bar 3: range = 4.0;  |107-104|=3; |103-104|=1  → TR = 4.0
    // ATR(14) over the 4 values we have so far = (4+4+3+4)/4 = 3.75
    // NATR = atr / close_of_last_closed_bar * 100
    //      = 3.75 / 106 * 100 = ~3.537735...
    EXPECT_NEAR(agg.trw.atr(), 3.75, 1e-9);
    EXPECT_NEAR(agg.natr, 3.75 / 106.0 * 100.0, 1e-9);
}

TEST(Selector, ScoresFromNatr)
{
    auto cfg = make_cfg();
    cfg.mandatory = {};
    cfg.w_volume = 0.0;       // isolate NATR contribution
    cfg.w_trades = 0.0;
    cfg.w_natr   = 10.0;
    cfg.add_score = 5.0;      // score = w_natr * natr, NATR ~3.5 → 35
    cfg.remove_score = 2.0;
    cfg.max_active = 5;
    cfg.min_active = 0;
    selector s(cfg);

    aggregates_map m;
    auto& sym = m["VOLATILEUSDT"];
    sym.natr = 3.5;            // synthetic — bypass the bar pipeline

    // Low-volatility competitor: NATR = 0, everything else zero.
    m["QUIETUSDT"];

    s.tick(m, clk::now());
    EXPECT_TRUE(s.active().count("VOLATILEUSDT"));
    EXPECT_FALSE(s.active().count("QUIETUSDT"));
}
