// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for the factory's built-in dispatch and custom registration.

#include <secret_provider/factory.hpp>
#include <secret_provider/test_provider.hpp>

#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace secret_provider;

namespace {

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

bool listed(const std::vector<std::string>& v, std::string_view needle)
{
    return std::any_of(v.begin(), v.end(), [&](const auto& s) { return s == needle; });
}

} // namespace

TEST(Factory, BuiltInTestProvider)
{
    auto p = create("test");
    ASSERT_NE(p, nullptr);
}

TEST(Factory, BuiltInDirProvider)
{
    auto p = create("dir:/tmp");
    ASSERT_NE(p, nullptr);
}

TEST(Factory, UnknownNameReturnsNull)
{
    auto p = create("does-not-exist");
    EXPECT_EQ(p, nullptr);
}

TEST(Factory, AvailableListsBuiltIns)
{
    auto names = available();
    EXPECT_TRUE(listed(names, "env"));
    EXPECT_TRUE(listed(names, "test"));
    EXPECT_TRUE(listed(names, "dir:<dir>"));
    EXPECT_TRUE(listed(names, "systemd-creds:<dir>"));
}

TEST(Factory, RegisterCustomProviderExactMatch)
{
    bool called = false;
    register_provider("custom_a", [&](std::string_view arg) -> std::unique_ptr<::secret_provider::secret_provider> {
        called = true;
        EXPECT_TRUE(arg.empty());
        auto tp = std::make_unique<test_provider>();
        tp->set("k", "v");
        return tp;
    });

    auto p = create("custom_a");
    EXPECT_TRUE(called);
    ASSERT_NE(p, nullptr);

    auto v = run_sync([&]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        co_return co_await p->async_get_secret("k");
    }());
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "v");
}

TEST(Factory, RegisterCustomProviderPrefixMatch)
{
    std::string seen_arg;
    register_provider("custom_b", [&](std::string_view arg) -> std::unique_ptr<::secret_provider::secret_provider> {
        seen_arg = std::string(arg);
        return std::make_unique<test_provider>();
    });

    auto p = create("custom_b:hello/world");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(seen_arg, "hello/world");
}

TEST(Factory, RegisterCustomProviderListedInAvailable)
{
    register_provider("custom_c", [](std::string_view) {
        return std::make_unique<test_provider>();
    });

    auto names = available();
    EXPECT_TRUE(listed(names, "custom_c"));
}

TEST(Factory, BuiltInsAreNotOverridden)
{
    bool called = false;
    register_provider("test", [&](std::string_view) -> std::unique_ptr<::secret_provider::secret_provider> {
        called = true;
        return nullptr;
    });

    auto p = create("test");
    ASSERT_NE(p, nullptr);
    EXPECT_FALSE(called);
}
