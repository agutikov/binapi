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
