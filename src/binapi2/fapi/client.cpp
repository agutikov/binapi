// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the top-level fapi::client facade. Provides prepare_and_send
/// (and its async counterpart) which centralise the shared query-building logic
/// used by every REST endpoint: auth injection, signing, query-string assembly,
/// and method-dependent placement of the query (URL for GET/DELETE, body for
/// POST/PUT).

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {

client::client(config cfg) :
    account(*this), convert(*this), market_data(*this), trade(*this), user_data_streams(*this),
    io_thread_(std::make_unique<detail::io_thread>()),
    cfg_(std::move(cfg)),
    http_(*io_thread_, cfg_)
{
}

client::client(config cfg, async_mode_t) :
    account(*this), convert(*this), market_data(*this), trade(*this), user_data_streams(*this),
    cfg_(std::move(cfg)),
    http_(cfg_)
{
}

client::~client() = default;

config&
client::configuration() noexcept
{
    return cfg_;
}

const config&
client::configuration() const noexcept
{
    return cfg_;
}

bool
client::has_io_thread() const noexcept
{
    return io_thread_ != nullptr;
}

transport::http_client&
client::transport() noexcept
{
    return http_;
}

websocket_api::client&
client::ws_api()
{
    if (!ws_api_) {
        if (io_thread_)
            ws_api_ = std::make_unique<websocket_api::client>(*io_thread_, cfg_);
        else
            ws_api_ = std::make_unique<websocket_api::client>(cfg_);
    }
    return *ws_api_;
}

streams::market_streams&
client::streams()
{
    if (!streams_) {
        if (io_thread_)
            streams_ = std::make_unique<streams::market_streams>(*io_thread_, cfg_);
        else
            streams_ = std::make_unique<streams::market_streams>(cfg_);
    }
    return *streams_;
}

streams::user_streams&
client::user_streams()
{
    if (!user_streams_) {
        if (io_thread_)
            user_streams_ = std::make_unique<streams::user_streams>(*io_thread_, cfg_);
        else
            user_streams_ = std::make_unique<streams::user_streams>(cfg_);
    }
    return *user_streams_;
}

// Shared query pipeline for synchronous REST calls. Copies the incoming query
// so the caller's map is not mutated, then optionally injects recvWindow +
// timestamp and appends the HMAC-SHA256 signature. GET and DELETE encode
// parameters into the URL; POST and PUT send them as a form-encoded body, as
// required by the Binance API.
result<transport::http_response>
client::prepare_and_send(boost::beast::http::verb method,
                         const std::string& path,
                         const query_map& query,
                         bool signed_request)
{
    query_map final_query = query;
    if (signed_request) {
        inject_auth_query(final_query, cfg_.recv_window, current_timestamp_ms());
        sign_query(final_query, cfg_.secret_key);
    }

    const auto query_string = build_query_string(final_query);
    std::string target = cfg_.rest_base_path + path;
    std::string body;
    if (!query_string.empty()) {
        if (method == boost::beast::http::verb::get || method == boost::beast::http::verb::delete_) {
            target += "?" + query_string;
        } else {
            body = query_string;
        }
    }

    return http_.request(method, target, body, "application/x-www-form-urlencoded", cfg_.api_key);
}

// Async variant of prepare_and_send. Takes parameters by value so they can be
// safely moved into the coroutine frame. Uses co_await on the transport layer's
// async_request, allowing the io_context to service other work while the
// network I/O is in flight.
boost::cobalt::task<result<transport::http_response>>
client::async_prepare_and_send(boost::beast::http::verb method,
                               std::string path,
                               query_map query,
                               bool signed_request)
{
    query_map final_query = std::move(query);
    if (signed_request) {
        inject_auth_query(final_query, cfg_.recv_window, current_timestamp_ms());
        sign_query(final_query, cfg_.secret_key);
    }

    const auto query_string = build_query_string(final_query);
    std::string target = cfg_.rest_base_path + path;
    std::string body;
    if (!query_string.empty()) {
        if (method == boost::beast::http::verb::get || method == boost::beast::http::verb::delete_) {
            target += "?" + query_string;
        } else {
            body = query_string;
        }
    }

    co_return co_await http_.async_request(method, std::move(target), std::move(body), "application/x-www-form-urlencoded", cfg_.api_key);
}

} // namespace binapi2::fapi
