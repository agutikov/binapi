#include <binapi2/umf/client.hpp>

#include <binapi2/umf/signing.hpp>
#include <binapi2/umf/time.hpp>
#include <binapi2/umf/types/account.hpp>
#include <binapi2/umf/types/common.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::umf {

namespace {

template <typename T>
result<T> decode_response(const transport::http_response &response) {
    if (response.status < 200 || response.status >= 300) {
        types::binance_error_document error_doc{};
        if (!glz::read_json(error_doc, response.body)) {
            return result<T>::failure({error_code::binance, response.status, error_doc.code, error_doc.msg, response.body});
        }
        return result<T>::failure({error_code::http_status, response.status, 0, "HTTP request failed", response.body});
    }

    if constexpr (std::is_same_v<T, types::empty_response>) {
        return result<T>::success({});
    } else {
        T value{};
        if (auto ec = glz::read_json(value, response.body)) {
            return result<T>::failure({error_code::json, response.status, 0, glz::format_error(ec, response.body), response.body});
        }
        return result<T>::success(std::move(value));
    }
}

} // namespace

client::client(boost::asio::io_context &io_context, config cfg)
    : account(*this)
    , market_data(*this)
    , trade(*this)
    , user_data_streams(*this)
    , io_context_(io_context)
    , cfg_(std::move(cfg))
    , http_(io_context_, cfg_) {}

config &client::configuration() noexcept { return cfg_; }
const config &client::configuration() const noexcept { return cfg_; }
transport::http_client &client::transport() noexcept { return http_; }

template <typename Response>
result<Response> client::execute(
    boost::beast::http::verb method,
    const std::string &path,
    const query_map &initial_query,
    bool signed_request
) {
    query_map query = initial_query;
    if (signed_request) {
        inject_auth_query(query, cfg_.recv_window, current_timestamp_ms());
        sign_query(query, cfg_.secret_key);
    }

    const auto query_string = build_query_string(query);
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
    return decode_response<Response>(*response);
}

template result<types::empty_response> client::execute<types::empty_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::server_time_response> client::execute<types::server_time_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::exchange_info_response> client::execute<types::exchange_info_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::order_book_response> client::execute<types::order_book_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::recent_trade>> client::execute<std::vector<types::recent_trade>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::aggregate_trade>> client::execute<std::vector<types::aggregate_trade>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::kline>> client::execute<std::vector<types::kline>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::book_ticker> client::execute<types::book_ticker>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::book_ticker>> client::execute<std::vector<types::book_ticker>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::price_ticker> client::execute<types::price_ticker>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::price_ticker>> client::execute<std::vector<types::price_ticker>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::ticker_24hr> client::execute<types::ticker_24hr>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::ticker_24hr>> client::execute<std::vector<types::ticker_24hr>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::mark_price> client::execute<types::mark_price>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::mark_price>> client::execute<std::vector<types::mark_price>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::funding_rate_history_entry>> client::execute<std::vector<types::funding_rate_history_entry>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::funding_rate_info>> client::execute<std::vector<types::funding_rate_info>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::open_interest> client::execute<types::open_interest>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::account_information> client::execute<types::account_information>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::futures_account_balance>> client::execute<std::vector<types::futures_account_balance>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<std::vector<types::position_risk>> client::execute<std::vector<types::position_risk>>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::listen_key_response> client::execute<types::listen_key_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);
template result<types::order_response> client::execute<types::order_response>(boost::beast::http::verb, const std::string &, const query_map &, bool);

} // namespace binapi2::umf
