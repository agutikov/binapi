// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/rest/account.hpp>
#include <binapi2/fapi/rest/market_data.hpp>
#include <binapi2/fapi/rest/trade.hpp>
#include <binapi2/fapi/rest/user_data_streams.hpp>
#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>
#include <binapi2/fapi/transport/http_client.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/http/verb.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace binapi2::fapi {

namespace detail {

template<typename T>
result<T>
decode_response(const transport::http_response& response)
{
    if (response.status < 200 || response.status >= 300) {
        types::binance_error_document error_doc{};
        if (!glz::read_json(error_doc, response.body)) {
            return result<T>::failure({ error_code::binance, response.status, error_doc.code, error_doc.msg, response.body });
        }
        return result<T>::failure({ error_code::http_status, response.status, 0, "HTTP request failed", response.body });
    }

    if constexpr (std::is_same_v<T, types::empty_response>) {
        return result<T>::success({});
    } else {
        T value{};
        if (auto ec = glz::read_json(value, response.body)) {
            return result<T>::failure(
                { error_code::json, response.status, 0, glz::format_error(ec, response.body), response.body });
        }
        return result<T>::success(std::move(value));
    }
}

} // namespace detail

class client
{
public:
    template<typename Response>
    using callback_type = std::function<void(result<Response>)>;

    client(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] config& configuration() noexcept;
    [[nodiscard]] const config& configuration() const noexcept;
    [[nodiscard]] boost::asio::io_context& context() noexcept;
    [[nodiscard]] transport::http_client& transport() noexcept;

    template<typename Response>
    [[nodiscard]] result<Response> execute(boost::beast::http::verb method,
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

        auto response = http_.request(method, target, body, "application/x-www-form-urlencoded", cfg_.api_key);
        if (!response) {
            return result<Response>::failure(response.err);
        }
        return detail::decode_response<Response>(*response);
    }

    template<typename Response>
    void async_execute(boost::beast::http::verb method,
                       std::string path,
                       query_map query,
                       bool signed_request,
                       callback_type<Response> callback)
    {
        boost::asio::post(
            io_context_,
            [this,
             method,
             path = std::move(path),
             query = std::move(query),
             signed_request,
             callback = std::move(callback)]() mutable { callback(execute<Response>(method, path, query, signed_request)); });
    }

    rest::account_service account;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

private:
    boost::asio::io_context& io_context_;
    config cfg_;
    transport::http_client http_;
};

} // namespace binapi2::fapi
