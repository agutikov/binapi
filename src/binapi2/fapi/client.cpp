// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {

client::client(boost::asio::io_context& io_context, config cfg) :
    account(*this), convert(*this), market_data(*this), trade(*this), user_data_streams(*this), io_context_(io_context), cfg_(std::move(cfg)),
    http_(io_context_, cfg_)
{
}

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

boost::asio::io_context&
client::context() noexcept
{
    return io_context_;
}

transport::http_client&
client::transport() noexcept
{
    return http_;
}

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
