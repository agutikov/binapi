// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for secret_provider using test_provider.

#include <secret_provider/test_provider.hpp>

#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <string>

using namespace secret_provider;

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

TEST(TestProvider, ReturnsStoredSecret)
{
    auto result = run_sync([]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        test_provider provider;
        provider.set("api_key", "my-secret-key");
        co_return co_await provider.async_get_secret("api_key");
    }());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "my-secret-key");
}

TEST(TestProvider, ReturnsErrorForMissingKey)
{
    auto result = run_sync([]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        test_provider provider;
        co_return co_await provider.async_get_secret("nonexistent");
    }());

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("not found"), std::string::npos);
}

TEST(TestProvider, OverwritesExistingKey)
{
    auto result = run_sync([]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        test_provider provider;
        provider.set("key", "value1");
        provider.set("key", "value2");
        co_return co_await provider.async_get_secret("key");
    }());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "value2");
}

TEST(TestProvider, MultipleKeys)
{
    auto result = run_sync([]() -> boost::cobalt::task<int> {
        test_provider provider;
        provider.set("a", "1");
        provider.set("b", "2");

        auto a = co_await provider.async_get_secret("a");
        auto b = co_await provider.async_get_secret("b");

        if (a.has_value() && b.has_value() && *a == "1" && *b == "2")
            co_return 1;
        co_return 0;
    }());

    EXPECT_EQ(result, 1);
}
