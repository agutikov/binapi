// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/error.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/config.hpp>

#include <gtest/gtest.h>

#include <string>

namespace {

using namespace binapi2::fapi;

// ---------------------------------------------------------------------------
// error
// ---------------------------------------------------------------------------

TEST(FapiError, DefaultConstructedHasNoneCode)
{
    error e;
    EXPECT_EQ(e.code, error_code::none);
}

TEST(FapiError, DefaultConstructedHasZeroHttpStatus)
{
    error e;
    EXPECT_EQ(e.http_status, 0);
}

TEST(FapiError, DefaultConstructedHasZeroBinanceCode)
{
    error e;
    EXPECT_EQ(e.binance_code, 0);
}

TEST(FapiError, DefaultConstructedHasEmptyMessage)
{
    error e;
    EXPECT_TRUE(e.message.empty());
}

TEST(FapiError, DefaultConstructedHasEmptyPayload)
{
    error e;
    EXPECT_TRUE(e.payload.empty());
}

TEST(FapiError, ConstructWithSpecificValues)
{
    error e;
    e.code = error_code::binance;
    e.http_status = 400;
    e.binance_code = -1021;
    e.message = "Timestamp outside recv_window";
    e.payload = R"({"code":-1021,"msg":"Timestamp outside recv_window"})";

    EXPECT_EQ(e.code, error_code::binance);
    EXPECT_EQ(e.http_status, 400);
    EXPECT_EQ(e.binance_code, -1021);
    EXPECT_EQ(e.message, "Timestamp outside recv_window");
    EXPECT_EQ(e.payload, R"({"code":-1021,"msg":"Timestamp outside recv_window"})");
}

TEST(FapiError, AllErrorCodesAreDistinct)
{
    EXPECT_NE(error_code::none, error_code::invalid_argument);
    EXPECT_NE(error_code::none, error_code::transport);
    EXPECT_NE(error_code::none, error_code::http_status);
    EXPECT_NE(error_code::none, error_code::json);
    EXPECT_NE(error_code::none, error_code::binance);
    EXPECT_NE(error_code::none, error_code::websocket);
    EXPECT_NE(error_code::none, error_code::internal);
}

// ---------------------------------------------------------------------------
// result<int>
// ---------------------------------------------------------------------------

TEST(FapiResultInt, SuccessIsTruthy)
{
    auto r = result<int>::success(42);
    EXPECT_TRUE(static_cast<bool>(r));
}

TEST(FapiResultInt, SuccessValueAccessViaDereference)
{
    auto r = result<int>::success(42);
    EXPECT_EQ(*r, 42);
}

TEST(FapiResultInt, SuccessErrorCodeIsNone)
{
    auto r = result<int>::success(7);
    EXPECT_EQ(r.err.code, error_code::none);
}

TEST(FapiResultInt, FailureIsFalsy)
{
    error e;
    e.code = error_code::transport;
    e.message = "connection refused";
    auto r = result<int>::failure(e);
    EXPECT_FALSE(static_cast<bool>(r));
}

TEST(FapiResultInt, FailurePreservesErrorCode)
{
    error e;
    e.code = error_code::transport;
    e.message = "timeout";
    auto r = result<int>::failure(e);
    EXPECT_EQ(r.err.code, error_code::transport);
    EXPECT_EQ(r.err.message, "timeout");
}

TEST(FapiResultInt, DefaultConstructedIsTruthy)
{
    result<int> r;
    EXPECT_TRUE(static_cast<bool>(r));
}

// ---------------------------------------------------------------------------
// result<std::string>
// ---------------------------------------------------------------------------

TEST(FapiResultString, SuccessValueMatches)
{
    auto r = result<std::string>::success("hello");
    EXPECT_TRUE(static_cast<bool>(r));
    EXPECT_EQ(*r, "hello");
}

TEST(FapiResultString, ArrowOperatorWorks)
{
    auto r = result<std::string>::success("hello world");
    EXPECT_EQ(r->size(), 11u);
}

TEST(FapiResultString, FailureIsFalsy)
{
    error e;
    e.code = error_code::json;
    auto r = result<std::string>::failure(e);
    EXPECT_FALSE(static_cast<bool>(r));
    EXPECT_EQ(r.err.code, error_code::json);
}

TEST(FapiResultString, DefaultConstructedIsTruthy)
{
    result<std::string> r;
    EXPECT_TRUE(static_cast<bool>(r));
}

// ---------------------------------------------------------------------------
// result<struct>
// ---------------------------------------------------------------------------

struct point
{
    int x{};
    int y{};
};

TEST(FapiResultStruct, SuccessValueMatches)
{
    auto r = result<point>::success(point{ 3, 4 });
    EXPECT_TRUE(static_cast<bool>(r));
    EXPECT_EQ((*r).x, 3);
    EXPECT_EQ((*r).y, 4);
}

TEST(FapiResultStruct, ArrowOperatorWorks)
{
    auto r = result<point>::success(point{ 10, 20 });
    EXPECT_EQ(r->x, 10);
    EXPECT_EQ(r->y, 20);
}

TEST(FapiResultStruct, FailurePreservesError)
{
    error e;
    e.code = error_code::internal;
    e.message = "unexpected";
    auto r = result<point>::failure(e);
    EXPECT_FALSE(static_cast<bool>(r));
    EXPECT_EQ(r.err.code, error_code::internal);
    EXPECT_EQ(r.err.message, "unexpected");
}

TEST(FapiResultStruct, ConstAccessWorks)
{
    const auto r = result<point>::success(point{ 1, 2 });
    EXPECT_EQ((*r).x, 1);
    EXPECT_EQ(r->y, 2);
}

// ---------------------------------------------------------------------------
// result<void>
// ---------------------------------------------------------------------------

TEST(FapiResultVoid, SuccessIsTruthy)
{
    auto r = result<void>::success();
    EXPECT_TRUE(static_cast<bool>(r));
}

TEST(FapiResultVoid, SuccessErrorCodeIsNone)
{
    auto r = result<void>::success();
    EXPECT_EQ(r.err.code, error_code::none);
}

TEST(FapiResultVoid, FailureIsFalsy)
{
    error e;
    e.code = error_code::http_status;
    e.http_status = 503;
    e.message = "Service Unavailable";
    auto r = result<void>::failure(e);
    EXPECT_FALSE(static_cast<bool>(r));
}

TEST(FapiResultVoid, FailurePreservesAllFields)
{
    error e;
    e.code = error_code::binance;
    e.http_status = 400;
    e.binance_code = -1100;
    e.message = "Illegal characters";
    e.payload = R"({"code":-1100})";
    auto r = result<void>::failure(e);
    EXPECT_EQ(r.err.code, error_code::binance);
    EXPECT_EQ(r.err.http_status, 400);
    EXPECT_EQ(r.err.binance_code, -1100);
    EXPECT_EQ(r.err.message, "Illegal characters");
    EXPECT_EQ(r.err.payload, R"({"code":-1100})");
}

TEST(FapiResultVoid, DefaultConstructedIsTruthy)
{
    result<void> r;
    EXPECT_TRUE(static_cast<bool>(r));
}

// ---------------------------------------------------------------------------
// config
// ---------------------------------------------------------------------------

TEST(FapiConfig, DefaultRestHost)
{
    config cfg;
    EXPECT_EQ(cfg.rest_host, "fapi.binance.com");
}

TEST(FapiConfig, DefaultRestPort)
{
    config cfg;
    EXPECT_EQ(cfg.rest_port, "443");
}

TEST(FapiConfig, DefaultRestBasePathEmpty)
{
    config cfg;
    EXPECT_TRUE(cfg.rest_base_path.empty());
}

TEST(FapiConfig, DefaultWebsocketApiHost)
{
    config cfg;
    EXPECT_EQ(cfg.websocket_api_host, "ws-fapi.binance.com");
}

TEST(FapiConfig, DefaultWebsocketApiPort)
{
    config cfg;
    EXPECT_EQ(cfg.websocket_api_port, "443");
}

TEST(FapiConfig, DefaultWebsocketApiTarget)
{
    config cfg;
    EXPECT_EQ(cfg.websocket_api_target, "/ws-fapi/v1");
}

TEST(FapiConfig, DefaultStreamHost)
{
    config cfg;
    EXPECT_EQ(cfg.stream_host, "fstream.binance.com");
}

TEST(FapiConfig, DefaultStreamPort)
{
    config cfg;
    EXPECT_EQ(cfg.stream_port, "443");
}

TEST(FapiConfig, DefaultStreamBaseTarget)
{
    config cfg;
    EXPECT_EQ(cfg.stream_base_target, "/ws");
}

TEST(FapiConfig, DefaultApiKeyEmpty)
{
    config cfg;
    EXPECT_TRUE(cfg.api_key.empty());
}

TEST(FapiConfig, DefaultSecretKeyEmpty)
{
    config cfg;
    EXPECT_TRUE(cfg.secret_key.empty());
}

TEST(FapiConfig, DefaultRecvWindow)
{
    config cfg;
    EXPECT_EQ(cfg.recv_window, 5000u);
}

TEST(FapiConfig, DefaultUserAgent)
{
    config cfg;
    EXPECT_EQ(cfg.user_agent, "binapi2-fapi/0.1.0");
}

TEST(FapiConfig, DefaultTestnetIsFalse)
{
    config cfg;
    EXPECT_FALSE(cfg.testnet);
}

TEST(FapiConfig, TestnetConfigHasTestnetTrue)
{
    auto cfg = config::testnet_config();
    EXPECT_TRUE(cfg.testnet);
}

TEST(FapiConfig, TestnetConfigRestHost)
{
    auto cfg = config::testnet_config();
    EXPECT_EQ(cfg.rest_host, "testnet.binancefuture.com");
}

TEST(FapiConfig, TestnetConfigWebsocketApiHost)
{
    auto cfg = config::testnet_config();
    EXPECT_EQ(cfg.websocket_api_host, "testnet.binancefuture.com");
}

TEST(FapiConfig, TestnetConfigStreamHost)
{
    auto cfg = config::testnet_config();
    EXPECT_EQ(cfg.stream_host, "fstream.binancefuture.com");
}

TEST(FapiConfig, TestnetConfigWebsocketApiTarget)
{
    auto cfg = config::testnet_config();
    EXPECT_EQ(cfg.websocket_api_target, "/ws-fapi/v1");
}

TEST(FapiConfig, TestnetConfigKeysStillEmpty)
{
    auto cfg = config::testnet_config();
    EXPECT_TRUE(cfg.api_key.empty());
    EXPECT_TRUE(cfg.secret_key.empty());
}

} // anonymous namespace
