// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/types/detail/enum_set.hpp>
#include <binapi2/fapi/types/enums.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace {

using namespace binapi2::fapi::types;

// Use trading_permission_t (5 values: 0..4) for most tests.
using perm_set = enum_set_t<trading_permission_t>;

// Use order_side_t (2 values: 0..1) for edge cases with small enums.
using side_set = enum_set_t<order_side_t>;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(EnumSet, DefaultConstructEmpty)
{
    perm_set s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(EnumSet, InitializerListConstruct)
{
    perm_set s{ trading_permission_t::grid, trading_permission_t::dca };
    EXPECT_EQ(s.size(), 2u);
    EXPECT_TRUE(s.contains(trading_permission_t::grid));
    EXPECT_TRUE(s.contains(trading_permission_t::dca));
    EXPECT_FALSE(s.contains(trading_permission_t::copy));
}

TEST(EnumSet, InitializerListDuplicates)
{
    perm_set s{ trading_permission_t::grid, trading_permission_t::grid };
    EXPECT_EQ(s.size(), 1u);
}

TEST(EnumSet, InitializerListAll)
{
    perm_set s{
        trading_permission_t::grid,
        trading_permission_t::copy,
        trading_permission_t::dca,
        trading_permission_t::rpi,
        trading_permission_t::psb,
    };
    EXPECT_EQ(s.size(), 5u);
}

// ---------------------------------------------------------------------------
// add / contains / remove
// ---------------------------------------------------------------------------

TEST(EnumSet, AddAndContains)
{
    perm_set s;
    s.add(trading_permission_t::copy);
    EXPECT_TRUE(s.contains(trading_permission_t::copy));
    EXPECT_FALSE(s.contains(trading_permission_t::grid));
    EXPECT_EQ(s.size(), 1u);
    EXPECT_FALSE(s.empty());
}

TEST(EnumSet, AddIdempotent)
{
    perm_set s;
    s.add(trading_permission_t::rpi);
    s.add(trading_permission_t::rpi);
    EXPECT_EQ(s.size(), 1u);
}

TEST(EnumSet, Remove)
{
    perm_set s{ trading_permission_t::grid, trading_permission_t::copy };
    EXPECT_EQ(s.size(), 2u);

    s.remove(trading_permission_t::grid);
    EXPECT_FALSE(s.contains(trading_permission_t::grid));
    EXPECT_TRUE(s.contains(trading_permission_t::copy));
    EXPECT_EQ(s.size(), 1u);
}

TEST(EnumSet, RemoveToEmpty)
{
    perm_set s{ trading_permission_t::psb };
    s.remove(trading_permission_t::psb);
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(EnumSet, ContainsOnEmpty)
{
    perm_set s;
    EXPECT_FALSE(s.contains(trading_permission_t::grid));
    EXPECT_FALSE(s.contains(trading_permission_t::psb));
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST(EnumSet, EqualitySame)
{
    perm_set a{ trading_permission_t::grid, trading_permission_t::dca };
    perm_set b{ trading_permission_t::dca, trading_permission_t::grid };
    EXPECT_EQ(a, b);
}

TEST(EnumSet, EqualityDifferent)
{
    perm_set a{ trading_permission_t::grid };
    perm_set b{ trading_permission_t::copy };
    EXPECT_NE(a, b);
}

TEST(EnumSet, EqualityEmpty)
{
    perm_set a;
    perm_set b;
    EXPECT_EQ(a, b);
}

TEST(EnumSet, EqualityDifferentSize)
{
    perm_set a{ trading_permission_t::grid };
    perm_set b{ trading_permission_t::grid, trading_permission_t::copy };
    EXPECT_NE(a, b);
}

// ---------------------------------------------------------------------------
// Iterator
// ---------------------------------------------------------------------------

TEST(EnumSet, IterateEmpty)
{
    perm_set s;
    EXPECT_EQ(s.begin(), s.end());
    std::vector<trading_permission_t> v(s.begin(), s.end());
    EXPECT_TRUE(v.empty());
}

TEST(EnumSet, IterateAll)
{
    perm_set s{
        trading_permission_t::grid,
        trading_permission_t::copy,
        trading_permission_t::dca,
        trading_permission_t::rpi,
        trading_permission_t::psb,
    };
    std::vector<trading_permission_t> v(s.begin(), s.end());
    ASSERT_EQ(v.size(), 5u);
    // Iterator yields values in ascending enum order.
    EXPECT_EQ(v[0], trading_permission_t::grid);
    EXPECT_EQ(v[1], trading_permission_t::copy);
    EXPECT_EQ(v[2], trading_permission_t::dca);
    EXPECT_EQ(v[3], trading_permission_t::rpi);
    EXPECT_EQ(v[4], trading_permission_t::psb);
}

TEST(EnumSet, IterateSparse)
{
    // Only first and last values set.
    perm_set s{ trading_permission_t::grid, trading_permission_t::psb };
    std::vector<trading_permission_t> v(s.begin(), s.end());
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], trading_permission_t::grid);
    EXPECT_EQ(v[1], trading_permission_t::psb);
}

TEST(EnumSet, IterateSingle)
{
    perm_set s{ trading_permission_t::dca };
    std::vector<trading_permission_t> v(s.begin(), s.end());
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], trading_permission_t::dca);
}

TEST(EnumSet, RangeFor)
{
    perm_set s{ trading_permission_t::copy, trading_permission_t::rpi };
    std::size_t count = 0;
    for (auto val : s) {
        EXPECT_TRUE(s.contains(val));
        ++count;
    }
    EXPECT_EQ(count, 2u);
}

TEST(EnumSet, PostIncrement)
{
    perm_set s{ trading_permission_t::grid, trading_permission_t::copy };
    auto it = s.begin();
    auto prev = it++;
    EXPECT_EQ(*prev, trading_permission_t::grid);
    EXPECT_EQ(*it, trading_permission_t::copy);
}

// ---------------------------------------------------------------------------
// Small enum (order_side_t: 2 values)
// ---------------------------------------------------------------------------

TEST(EnumSet, SmallEnumEmpty)
{
    side_set s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(EnumSet, SmallEnumFull)
{
    side_set s{ order_side_t::buy, order_side_t::sell };
    EXPECT_EQ(s.size(), 2u);
    EXPECT_TRUE(s.contains(order_side_t::buy));
    EXPECT_TRUE(s.contains(order_side_t::sell));
}

TEST(EnumSet, SmallEnumIterate)
{
    side_set s{ order_side_t::sell };
    std::vector<order_side_t> v(s.begin(), s.end());
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], order_side_t::sell);
}

// ---------------------------------------------------------------------------
// enum_capacity trait
// ---------------------------------------------------------------------------

TEST(EnumSet, CapacityTradingPermission)
{
    // trading_permission_t has 5 glaze entries, no aliases.
    EXPECT_EQ(enum_capacity<trading_permission_t>, 5u);
}

TEST(EnumSet, CapacityOrderSide)
{
    EXPECT_EQ(enum_capacity<order_side_t>, 2u);
}

TEST(EnumSet, CapacityOrderType)
{
    // order_type_t has 7 values, no aliases.
    EXPECT_EQ(enum_capacity<order_type_t>, 7u);
}

// ---------------------------------------------------------------------------
// JSON serialization / deserialization via glaze
// ---------------------------------------------------------------------------

TEST(EnumSet, JsonDeserialize)
{
    perm_set s;
    auto ec = glz::read_json(s, R"(["GRID","DCA","PSB"])");
    ASSERT_FALSE(ec) << glz::format_error(ec);
    EXPECT_EQ(s.size(), 3u);
    EXPECT_TRUE(s.contains(trading_permission_t::grid));
    EXPECT_TRUE(s.contains(trading_permission_t::dca));
    EXPECT_TRUE(s.contains(trading_permission_t::psb));
    EXPECT_FALSE(s.contains(trading_permission_t::copy));
}

TEST(EnumSet, JsonDeserializeEmpty)
{
    perm_set s{ trading_permission_t::grid };
    auto ec = glz::read_json(s, R"([])");
    ASSERT_FALSE(ec) << glz::format_error(ec);
    EXPECT_TRUE(s.empty());
}

TEST(EnumSet, JsonDeserializeDuplicates)
{
    perm_set s;
    auto ec = glz::read_json(s, R"(["COPY","COPY","COPY"])");
    ASSERT_FALSE(ec) << glz::format_error(ec);
    EXPECT_EQ(s.size(), 1u);
    EXPECT_TRUE(s.contains(trading_permission_t::copy));
}

TEST(EnumSet, JsonSerialize)
{
    perm_set s{ trading_permission_t::grid, trading_permission_t::psb };
    std::string json;
    (void)glz::write_json(s, json);
    // Values are iterated in enum order (grid=0, psb=4).
    EXPECT_EQ(json, R"(["GRID","PSB"])");
}

TEST(EnumSet, JsonSerializeEmpty)
{
    perm_set s;
    std::string json;
    (void)glz::write_json(s, json);
    EXPECT_EQ(json, "[]");
}

TEST(EnumSet, JsonRoundTrip)
{
    perm_set original{
        trading_permission_t::copy,
        trading_permission_t::dca,
        trading_permission_t::rpi,
    };
    std::string json;
    (void)glz::write_json(original, json);

    perm_set parsed;
    auto ec = glz::read_json(parsed, json);
    ASSERT_FALSE(ec) << glz::format_error(ec);
    EXPECT_EQ(original, parsed);
}

TEST(EnumSet, JsonDeserializeOrderTypes)
{
    enum_set_t<order_type_t> s;
    auto ec = glz::read_json(s, R"(["LIMIT","MARKET","STOP"])");
    ASSERT_FALSE(ec) << glz::format_error(ec);
    EXPECT_EQ(s.size(), 3u);
    EXPECT_TRUE(s.contains(order_type_t::limit));
    EXPECT_TRUE(s.contains(order_type_t::market));
    EXPECT_TRUE(s.contains(order_type_t::stop));
}

} // anonymous namespace
