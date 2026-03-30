#pragma once

#include <binapi2/umf/types/enums.hpp>

#include <boost/beast/http/verb.hpp>

#include <string_view>

namespace binapi2::umf::rest {

struct endpoint_metadata {
    std::string_view name;
    boost::beast::http::verb method;
    std::string_view path;
    types::security_type security;
    bool signed_request;
    bool requires_timestamp;
    bool allows_recv_window;
};

inline constexpr endpoint_metadata ping_endpoint{"ping", boost::beast::http::verb::get, "/fapi/v1/ping", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata server_time_endpoint{"server_time", boost::beast::http::verb::get, "/fapi/v1/time", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata exchange_info_endpoint{"exchange_info", boost::beast::http::verb::get, "/fapi/v1/exchangeInfo", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata order_book_endpoint{"order_book", boost::beast::http::verb::get, "/fapi/v1/depth", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata recent_trades_endpoint{"recent_trades", boost::beast::http::verb::get, "/fapi/v1/trades", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata aggregate_trades_endpoint{"aggregate_trades", boost::beast::http::verb::get, "/fapi/v1/aggTrades", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata klines_endpoint{"klines", boost::beast::http::verb::get, "/fapi/v1/klines", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata continuous_klines_endpoint{"continuous_klines", boost::beast::http::verb::get, "/fapi/v1/continuousKlines", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata index_price_klines_endpoint{"index_price_klines", boost::beast::http::verb::get, "/fapi/v1/indexPriceKlines", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata mark_price_klines_endpoint{"mark_price_klines", boost::beast::http::verb::get, "/fapi/v1/markPriceKlines", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata premium_index_klines_endpoint{"premium_index_klines", boost::beast::http::verb::get, "/fapi/v1/premiumIndexKlines", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata book_ticker_endpoint{"book_ticker", boost::beast::http::verb::get, "/fapi/v1/ticker/bookTicker", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata price_ticker_endpoint{"price_ticker", boost::beast::http::verb::get, "/fapi/v1/ticker/price", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata ticker_24hr_endpoint{"ticker_24hr", boost::beast::http::verb::get, "/fapi/v1/ticker/24hr", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata mark_price_endpoint{"mark_price", boost::beast::http::verb::get, "/fapi/v1/premiumIndex", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata funding_rate_history_endpoint{"funding_rate_history", boost::beast::http::verb::get, "/fapi/v1/fundingRate", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata funding_rate_info_endpoint{"funding_rate_info", boost::beast::http::verb::get, "/fapi/v1/fundingInfo", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata open_interest_endpoint{"open_interest", boost::beast::http::verb::get, "/fapi/v1/openInterest", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata open_interest_statistics_endpoint{"open_interest_statistics", boost::beast::http::verb::get, "/futures/data/openInterestHist", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata top_long_short_account_ratio_endpoint{"top_long_short_account_ratio", boost::beast::http::verb::get, "/futures/data/topLongShortAccountRatio", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata top_trader_long_short_ratio_endpoint{"top_trader_long_short_ratio", boost::beast::http::verb::get, "/futures/data/topLongShortPositionRatio", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata long_short_ratio_endpoint{"long_short_ratio", boost::beast::http::verb::get, "/futures/data/globalLongShortAccountRatio", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata taker_buy_sell_volume_endpoint{"taker_buy_sell_volume", boost::beast::http::verb::get, "/futures/data/takerlongshortRatio", types::security_type::none, false, false, false};
inline constexpr endpoint_metadata historical_trades_endpoint{"historical_trades", boost::beast::http::verb::get, "/fapi/v1/historicalTrades", types::security_type::market_data, false, false, false};
inline constexpr endpoint_metadata account_information_endpoint{"account_information", boost::beast::http::verb::get, "/fapi/v3/account", types::security_type::user_data, true, true, true};
inline constexpr endpoint_metadata account_balances_endpoint{"account_balances", boost::beast::http::verb::get, "/fapi/v3/balance", types::security_type::user_data, true, true, true};
inline constexpr endpoint_metadata position_risk_endpoint{"position_risk", boost::beast::http::verb::get, "/fapi/v2/positionRisk", types::security_type::user_data, true, true, true};
inline constexpr endpoint_metadata start_listen_key_endpoint{"start_listen_key", boost::beast::http::verb::post, "/fapi/v1/listenKey", types::security_type::user_stream, false, false, false};
inline constexpr endpoint_metadata keepalive_listen_key_endpoint{"keepalive_listen_key", boost::beast::http::verb::put, "/fapi/v1/listenKey", types::security_type::user_stream, false, false, false};
inline constexpr endpoint_metadata close_listen_key_endpoint{"close_listen_key", boost::beast::http::verb::delete_, "/fapi/v1/listenKey", types::security_type::user_stream, false, false, false};
inline constexpr endpoint_metadata new_order_endpoint{"new_order", boost::beast::http::verb::post, "/fapi/v1/order", types::security_type::trade, true, true, true};
inline constexpr endpoint_metadata modify_order_endpoint{"modify_order", boost::beast::http::verb::put, "/fapi/v1/order", types::security_type::trade, true, true, true};
inline constexpr endpoint_metadata cancel_order_endpoint{"cancel_order", boost::beast::http::verb::delete_, "/fapi/v1/order", types::security_type::trade, true, true, true};
inline constexpr endpoint_metadata query_order_endpoint{"query_order", boost::beast::http::verb::get, "/fapi/v1/order", types::security_type::trade, true, true, true};

} // namespace binapi2::umf::rest
