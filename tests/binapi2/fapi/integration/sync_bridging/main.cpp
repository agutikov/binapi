// SPDX-License-Identifier: Apache-2.0
//
// Sync bridging integration test: verifies all non-async call models work
// correctly against the Postman mock server.
//
// Tests: io_thread::run_sync, future via cobalt::spawn, callback via cobalt::spawn.
//
// Prerequisites: same as postman_mock integration test.

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <cstdlib>
#include <future>
#include <iostream>
#include <string>

using namespace binapi2::fapi;

static std::string env_or(const char* name, const char* fallback)
{
    const char* v = std::getenv(name);
    return v ? v : fallback;
}

static int failures = 0;
static int passed = 0;

template<typename T>
static bool check(const char* name, const result<T>& r)
{
    if (!r) {
        std::cerr << "FAIL " << name << ": " << r.err.message << "\n";
        ++failures;
        return false;
    }
    std::cout << "  OK " << name << "\n";
    ++passed;
    return true;
}

// The core async operation used by all bridging tests.
static boost::cobalt::task<result<types::server_time_response_t>>
get_server_time(binapi2::futures_usdm_api& c)
{
    auto rest = co_await c.create_rest_client();
    if (!rest)
        co_return result<types::server_time_response_t>::failure(rest.err);
    co_return co_await (*rest)->market_data.async_execute(types::server_time_request_t{});
}

// --- Test 1: io_thread::run_sync ---

static void test_run_sync(binapi2::futures_usdm_api& c)
{
    std::cout << "--- run_sync ---\n";
    detail::io_thread io;
    auto r = io.run_sync(get_server_time(c));
    check("run_sync", r);
    if (r && r->serverTime.value == 0) {
        std::cerr << "FAIL run_sync: serverTime is 0\n";
        ++failures; --passed;
    }
}

// --- Test 2: future via cobalt::spawn ---

static void test_future(binapi2::futures_usdm_api& c)
{
    std::cout << "--- future ---\n";
    detail::io_thread io;
    auto future = boost::cobalt::spawn(io.context(), get_server_time(c), boost::asio::use_future);
    auto r = future.get();
    check("future", r);
    if (r && r->serverTime.value == 0) {
        std::cerr << "FAIL future: serverTime is 0\n";
        ++failures; --passed;
    }
}

// --- Test 3: callback via cobalt::spawn ---

static void test_callback(binapi2::futures_usdm_api& c)
{
    std::cout << "--- callback ---\n";
    detail::io_thread io;
    std::promise<result<types::server_time_response_t>> promise;
    auto cb_future = promise.get_future();

    boost::cobalt::spawn(io.context(), get_server_time(c),
        [&promise](std::exception_ptr ep, result<types::server_time_response_t> r) {
            if (ep) {
                promise.set_value(result<types::server_time_response_t>::failure(
                    { error_code::internal, 0, 0, "exception in callback", {} }));
            } else {
                promise.set_value(std::move(r));
            }
        });

    auto r = cb_future.get();
    check("callback", r);
    if (r && r->serverTime.value == 0) {
        std::cerr << "FAIL callback: serverTime is 0\n";
        ++failures; --passed;
    }
}

// --- Test 4: manual io_context ---

static void test_manual_io_context(binapi2::futures_usdm_api& c)
{
    std::cout << "--- manual_io_context ---\n";
    boost::asio::io_context io;
    auto future = boost::cobalt::spawn(io, get_server_time(c), boost::asio::use_future);
    io.run();
    auto r = future.get();
    check("manual_io_context", r);
}

int main()
{
    const auto host = env_or("MOCK_HOST", "localhost");
    const auto port = env_or("MOCK_PORT", "8443");

    std::cout << "sync bridging test against " << host << ":" << port << "\n\n";

    config cfg;
    cfg.rest_host = host;
    cfg.rest_port = port;
    cfg.api_key = "test-api-key";
    cfg.secret_key = "test-secret-key";
    cfg.sign_method = sign_method_t::hmac;
    cfg.ca_cert_file = env_or("SSL_CERT_FILE", "");

    binapi2::futures_usdm_api c(cfg);

    test_run_sync(c);
    test_future(c);
    test_callback(c);
    test_manual_io_context(c);

    std::cout << "\n========================================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failures << "\n";
    return failures > 0 ? 1 : 0;
}
