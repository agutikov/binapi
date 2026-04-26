// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for dir_provider.

#include <secret_provider/dir_provider.hpp>

#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

using namespace secret_provider;

namespace {

template<typename T>
T run_sync(boost::cobalt::task<T> t)
{
    return boost::cobalt::run(std::move(t));
}

class DirProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        dir_ = std::filesystem::temp_directory_path() /
               ("secret_provider_dir_test_" + std::to_string(stamp));
        std::filesystem::create_directories(dir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(dir_, ec);
    }

    void write(const std::string& key, const std::string& value)
    {
        auto p = dir_ / key;
        std::filesystem::create_directories(p.parent_path());
        std::ofstream out(p, std::ios::binary);
        out << value;
    }

    std::filesystem::path dir_;
};

} // namespace

TEST_F(DirProviderTest, ReadsFileContents)
{
    write("api_key", "super-secret-value");

    auto result = run_sync([this]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        dir_provider provider(dir_);
        co_return co_await provider.async_get_secret("api_key");
    }());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "super-secret-value");
}

TEST_F(DirProviderTest, StripsSingleTrailingNewline)
{
    write("api_key", "value\n");

    auto result = run_sync([this]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        dir_provider provider(dir_);
        co_return co_await provider.async_get_secret("api_key");
    }());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "value");
}

TEST_F(DirProviderTest, StripsTrailingCRLF)
{
    write("api_key", "value\r\n");

    auto result = run_sync([this]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        dir_provider provider(dir_);
        co_return co_await provider.async_get_secret("api_key");
    }());

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "value");
}

TEST_F(DirProviderTest, ReturnsErrorForMissingKey)
{
    auto result = run_sync([this]() -> boost::cobalt::task<std::expected<std::string, std::string>> {
        dir_provider provider(dir_);
        co_return co_await provider.async_get_secret("nonexistent");
    }());

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("not found"), std::string::npos);
}

TEST_F(DirProviderTest, SupportsHierarchicalKeys)
{
    write("binance/api_key", "k");
    write("binance/api_secret", "s");

    auto result = run_sync([this]() -> boost::cobalt::task<std::pair<std::string, std::string>> {
        dir_provider provider(dir_);
        auto k = co_await provider.async_get_secret("binance/api_key");
        auto s = co_await provider.async_get_secret("binance/api_secret");
        co_return std::pair{*k, *s};
    }());

    EXPECT_EQ(result.first, "k");
    EXPECT_EQ(result.second, "s");
}
