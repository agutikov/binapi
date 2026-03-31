// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/enums.hpp>

#include <boost/beast/http/verb.hpp>

#include <string_view>

namespace binapi2::fapi::rest {

struct endpoint_metadata
{
    std::string_view name;
    boost::beast::http::verb method;
    std::string_view path;
    types::security_type security;
    bool signed_request;
    bool requires_timestamp;
    bool allows_recv_window;
};

inline constexpr endpoint_metadata ping_endpoint{
    "ping", boost::beast::http::verb::get, "/fapi/v1/ping", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata server_time_endpoint{
    "server_time", boost::beast::http::verb::get, "/fapi/v1/time", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata exchange_info_endpoint{
    "exchange_info", boost::beast::http::verb::get, "/fapi/v1/exchangeInfo", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata order_book_endpoint{
    "order_book", boost::beast::http::verb::get, "/fapi/v1/depth", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata recent_trades_endpoint{
    "recent_trades", boost::beast::http::verb::get, "/fapi/v1/trades", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata aggregate_trades_endpoint{
    "aggregate_trades", boost::beast::http::verb::get, "/fapi/v1/aggTrades", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata klines_endpoint{
    "klines", boost::beast::http::verb::get, "/fapi/v1/klines", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata continuous_klines_endpoint{ "continuous_klines",
                                                               boost::beast::http::verb::get,
                                                               "/fapi/v1/continuousKlines",
                                                               types::security_type::none,
                                                               false,
                                                               false,
                                                               false };
inline constexpr endpoint_metadata index_price_klines_endpoint{ "index_price_klines",
                                                                boost::beast::http::verb::get,
                                                                "/fapi/v1/indexPriceKlines",
                                                                types::security_type::none,
                                                                false,
                                                                false,
                                                                false };
inline constexpr endpoint_metadata mark_price_klines_endpoint{ "mark_price_klines",
                                                               boost::beast::http::verb::get,
                                                               "/fapi/v1/markPriceKlines",
                                                               types::security_type::none,
                                                               false,
                                                               false,
                                                               false };
inline constexpr endpoint_metadata premium_index_klines_endpoint{ "premium_index_klines",
                                                                  boost::beast::http::verb::get,
                                                                  "/fapi/v1/premiumIndexKlines",
                                                                  types::security_type::none,
                                                                  false,
                                                                  false,
                                                                  false };
inline constexpr endpoint_metadata book_ticker_endpoint{
    "book_ticker", boost::beast::http::verb::get, "/fapi/v1/ticker/bookTicker", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata price_ticker_endpoint{
    "price_ticker", boost::beast::http::verb::get, "/fapi/v1/ticker/price", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata ticker_24hr_endpoint{
    "ticker_24hr", boost::beast::http::verb::get, "/fapi/v1/ticker/24hr", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata mark_price_endpoint{
    "mark_price", boost::beast::http::verb::get, "/fapi/v1/premiumIndex", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata funding_rate_history_endpoint{ "funding_rate_history",
                                                                  boost::beast::http::verb::get,
                                                                  "/fapi/v1/fundingRate",
                                                                  types::security_type::none,
                                                                  false,
                                                                  false,
                                                                  false };
inline constexpr endpoint_metadata funding_rate_info_endpoint{
    "funding_rate_info", boost::beast::http::verb::get, "/fapi/v1/fundingInfo", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata open_interest_endpoint{
    "open_interest", boost::beast::http::verb::get, "/fapi/v1/openInterest", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata open_interest_statistics_endpoint{ "open_interest_statistics",
                                                                      boost::beast::http::verb::get,
                                                                      "/futures/data/openInterestHist",
                                                                      types::security_type::none,
                                                                      false,
                                                                      false,
                                                                      false };
inline constexpr endpoint_metadata top_long_short_account_ratio_endpoint{ "top_long_short_account_ratio",
                                                                          boost::beast::http::verb::get,
                                                                          "/futures/data/topLongShortAccountRatio",
                                                                          types::security_type::none,
                                                                          false,
                                                                          false,
                                                                          false };
inline constexpr endpoint_metadata top_trader_long_short_ratio_endpoint{ "top_trader_long_short_ratio",
                                                                         boost::beast::http::verb::get,
                                                                         "/futures/data/topLongShortPositionRatio",
                                                                         types::security_type::none,
                                                                         false,
                                                                         false,
                                                                         false };
inline constexpr endpoint_metadata long_short_ratio_endpoint{ "long_short_ratio",
                                                              boost::beast::http::verb::get,
                                                              "/futures/data/globalLongShortAccountRatio",
                                                              types::security_type::none,
                                                              false,
                                                              false,
                                                              false };
inline constexpr endpoint_metadata taker_buy_sell_volume_endpoint{ "taker_buy_sell_volume",
                                                                   boost::beast::http::verb::get,
                                                                   "/futures/data/takerlongshortRatio",
                                                                   types::security_type::none,
                                                                   false,
                                                                   false,
                                                                   false };
inline constexpr endpoint_metadata historical_trades_endpoint{ "historical_trades",
                                                               boost::beast::http::verb::get,
                                                               "/fapi/v1/historicalTrades",
                                                               types::security_type::market_data,
                                                               false,
                                                               false,
                                                               false };
inline constexpr endpoint_metadata account_information_endpoint{
    "account_information", boost::beast::http::verb::get, "/fapi/v3/account", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata account_balances_endpoint{
    "account_balances", boost::beast::http::verb::get, "/fapi/v3/balance", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata position_risk_endpoint{
    "position_risk", boost::beast::http::verb::get, "/fapi/v2/positionRisk", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata start_listen_key_endpoint{ "start_listen_key",
                                                              boost::beast::http::verb::post,
                                                              "/fapi/v1/listenKey",
                                                              types::security_type::user_stream,
                                                              false,
                                                              false,
                                                              false };
inline constexpr endpoint_metadata keepalive_listen_key_endpoint{ "keepalive_listen_key",
                                                                  boost::beast::http::verb::put,
                                                                  "/fapi/v1/listenKey",
                                                                  types::security_type::user_stream,
                                                                  false,
                                                                  false,
                                                                  false };
inline constexpr endpoint_metadata close_listen_key_endpoint{ "close_listen_key",
                                                              boost::beast::http::verb::delete_,
                                                              "/fapi/v1/listenKey",
                                                              types::security_type::user_stream,
                                                              false,
                                                              false,
                                                              false };
inline constexpr endpoint_metadata new_order_endpoint{
    "new_order", boost::beast::http::verb::post, "/fapi/v1/order", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata modify_order_endpoint{
    "modify_order", boost::beast::http::verb::put, "/fapi/v1/order", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata cancel_order_endpoint{
    "cancel_order", boost::beast::http::verb::delete_, "/fapi/v1/order", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata query_order_endpoint{
    "query_order", boost::beast::http::verb::get, "/fapi/v1/order", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata basis_endpoint{
    "basis", boost::beast::http::verb::get, "/futures/data/basis", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata price_ticker_v2_endpoint{
    "price_ticker_v2", boost::beast::http::verb::get, "/fapi/v2/ticker/price", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata delivery_price_endpoint{
    "delivery_price", boost::beast::http::verb::get, "/futures/data/delivery-price", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata composite_index_info_endpoint{ "composite_index_info",
                                                                  boost::beast::http::verb::get,
                                                                  "/fapi/v1/indexInfo",
                                                                  types::security_type::none,
                                                                  false,
                                                                  false,
                                                                  false };
inline constexpr endpoint_metadata index_constituents_endpoint{ "index_constituents",
                                                                boost::beast::http::verb::get,
                                                                "/fapi/v1/constituents",
                                                                types::security_type::none,
                                                                false,
                                                                false,
                                                                false };
inline constexpr endpoint_metadata asset_index_endpoint{
    "asset_index", boost::beast::http::verb::get, "/fapi/v1/assetIndex", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata insurance_fund_endpoint{ "insurance_fund",
                                                            boost::beast::http::verb::get,
                                                            "/fapi/v1/insuranceBalance",
                                                            types::security_type::none,
                                                            false,
                                                            false,
                                                            false };
inline constexpr endpoint_metadata adl_risk_endpoint{
    "adl_risk", boost::beast::http::verb::get, "/fapi/v1/symbolAdlRisk", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata rpi_depth_endpoint{
    "rpi_depth", boost::beast::http::verb::get, "/fapi/v1/rpiDepth", types::security_type::none, false, false, false
};
inline constexpr endpoint_metadata trading_schedule_endpoint{ "trading_schedule",
                                                              boost::beast::http::verb::get,
                                                              "/fapi/v1/tradingSchedule",
                                                              types::security_type::none,
                                                              false,
                                                              false,
                                                              false };

inline constexpr endpoint_metadata test_order_endpoint{
    "test_order", boost::beast::http::verb::post, "/fapi/v1/order/test", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata batch_orders_endpoint{
    "batch_orders", boost::beast::http::verb::post, "/fapi/v1/batchOrders", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata modify_batch_orders_endpoint{
    "modify_batch_orders", boost::beast::http::verb::put, "/fapi/v1/batchOrders", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata cancel_batch_orders_endpoint{ "cancel_batch_orders",
                                                                 boost::beast::http::verb::delete_,
                                                                 "/fapi/v1/batchOrders",
                                                                 types::security_type::trade,
                                                                 true,
                                                                 true,
                                                                 true };
inline constexpr endpoint_metadata cancel_all_open_orders_endpoint{ "cancel_all_open_orders",
                                                                    boost::beast::http::verb::delete_,
                                                                    "/fapi/v1/allOpenOrders",
                                                                    types::security_type::trade,
                                                                    true,
                                                                    true,
                                                                    true };
inline constexpr endpoint_metadata auto_cancel_endpoint{
    "auto_cancel", boost::beast::http::verb::post, "/fapi/v1/countdownCancelAll", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata query_open_order_endpoint{
    "query_open_order", boost::beast::http::verb::get, "/fapi/v1/openOrder", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata all_open_orders_endpoint{
    "all_open_orders", boost::beast::http::verb::get, "/fapi/v1/openOrders", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata all_orders_endpoint{
    "all_orders", boost::beast::http::verb::get, "/fapi/v1/allOrders", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata position_risk_v3_endpoint{
    "position_risk_v3", boost::beast::http::verb::get, "/fapi/v3/positionRisk", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata adl_quantile_endpoint{
    "adl_quantile", boost::beast::http::verb::get, "/fapi/v1/adlQuantile", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata force_orders_endpoint{
    "force_orders", boost::beast::http::verb::get, "/fapi/v1/forceOrders", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata account_trades_endpoint{
    "account_trades", boost::beast::http::verb::get, "/fapi/v1/userTrades", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata change_position_mode_endpoint{ "change_position_mode",
                                                                  boost::beast::http::verb::post,
                                                                  "/fapi/v1/positionSide/dual",
                                                                  types::security_type::trade,
                                                                  true,
                                                                  true,
                                                                  true };
inline constexpr endpoint_metadata change_multi_assets_endpoint{ "change_multi_assets",
                                                                 boost::beast::http::verb::post,
                                                                 "/fapi/v1/multiAssetsMargin",
                                                                 types::security_type::trade,
                                                                 true,
                                                                 true,
                                                                 true };
inline constexpr endpoint_metadata change_leverage_endpoint{
    "change_leverage", boost::beast::http::verb::post, "/fapi/v1/leverage", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata change_margin_type_endpoint{
    "change_margin_type", boost::beast::http::verb::post, "/fapi/v1/marginType", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata modify_isolated_margin_endpoint{ "modify_isolated_margin",
                                                                    boost::beast::http::verb::post,
                                                                    "/fapi/v1/positionMargin",
                                                                    types::security_type::trade,
                                                                    true,
                                                                    true,
                                                                    true };
inline constexpr endpoint_metadata position_margin_history_endpoint{ "position_margin_history",
                                                                     boost::beast::http::verb::get,
                                                                     "/fapi/v1/positionMargin/history",
                                                                     types::security_type::user_data,
                                                                     true,
                                                                     true,
                                                                     true };
inline constexpr endpoint_metadata order_modify_history_endpoint{ "order_modify_history",
                                                                  boost::beast::http::verb::get,
                                                                  "/fapi/v1/orderAmendment",
                                                                  types::security_type::user_data,
                                                                  true,
                                                                  true,
                                                                  true };
inline constexpr endpoint_metadata new_algo_order_endpoint{
    "new_algo_order", boost::beast::http::verb::post, "/fapi/v1/algoOrder", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata cancel_algo_order_endpoint{ "cancel_algo_order",
                                                               boost::beast::http::verb::delete_,
                                                               "/fapi/v1/algoOrder",
                                                               types::security_type::trade,
                                                               true,
                                                               true,
                                                               true };
inline constexpr endpoint_metadata query_algo_order_endpoint{
    "query_algo_order", boost::beast::http::verb::get, "/fapi/v1/algoOrder", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata all_algo_orders_endpoint{
    "all_algo_orders", boost::beast::http::verb::get, "/fapi/v1/allAlgoOrders", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata open_algo_orders_endpoint{
    "open_algo_orders", boost::beast::http::verb::get, "/fapi/v1/openAlgoOrders", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata cancel_all_algo_orders_endpoint{ "cancel_all_algo_orders",
                                                                    boost::beast::http::verb::delete_,
                                                                    "/fapi/v1/algoOpenOrders",
                                                                    types::security_type::trade,
                                                                    true,
                                                                    true,
                                                                    true };
inline constexpr endpoint_metadata account_config_endpoint{
    "account_config", boost::beast::http::verb::get, "/fapi/v1/accountConfig", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata symbol_config_endpoint{
    "symbol_config", boost::beast::http::verb::get, "/fapi/v1/symbolConfig", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata get_multi_assets_mode_endpoint{ "get_multi_assets_mode",
                                                                   boost::beast::http::verb::get,
                                                                   "/fapi/v1/multiAssetsMargin",
                                                                   types::security_type::user_data,
                                                                   true,
                                                                   true,
                                                                   true };
inline constexpr endpoint_metadata get_position_mode_endpoint{
    "get_position_mode", boost::beast::http::verb::get, "/fapi/v1/positionSide/dual", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata income_history_endpoint{
    "income_history", boost::beast::http::verb::get, "/fapi/v1/income", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata leverage_brackets_endpoint{
    "leverage_brackets", boost::beast::http::verb::get, "/fapi/v1/leverageBracket", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata commission_rate_endpoint{
    "commission_rate", boost::beast::http::verb::get, "/fapi/v1/commissionRate", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata rate_limit_order_endpoint{
    "rate_limit_order", boost::beast::http::verb::get, "/fapi/v1/rateLimit/order", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata download_id_transaction_endpoint{ "download_id_transaction",
                                                                     boost::beast::http::verb::get,
                                                                     "/fapi/v1/income/asyn",
                                                                     types::security_type::user_data,
                                                                     true,
                                                                     true,
                                                                     true };
inline constexpr endpoint_metadata download_link_transaction_endpoint{ "download_link_transaction",
                                                                       boost::beast::http::verb::get,
                                                                       "/fapi/v1/income/asyn/id",
                                                                       types::security_type::user_data,
                                                                       true,
                                                                       true,
                                                                       true };
inline constexpr endpoint_metadata download_id_order_endpoint{
    "download_id_order", boost::beast::http::verb::get, "/fapi/v1/order/asyn", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata download_link_order_endpoint{ "download_link_order",
                                                                 boost::beast::http::verb::get,
                                                                 "/fapi/v1/order/asyn/id",
                                                                 types::security_type::user_data,
                                                                 true,
                                                                 true,
                                                                 true };
inline constexpr endpoint_metadata download_id_trade_endpoint{
    "download_id_trade", boost::beast::http::verb::get, "/fapi/v1/trade/asyn", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata download_link_trade_endpoint{ "download_link_trade",
                                                                 boost::beast::http::verb::get,
                                                                 "/fapi/v1/trade/asyn/id",
                                                                 types::security_type::user_data,
                                                                 true,
                                                                 true,
                                                                 true };
inline constexpr endpoint_metadata get_bnb_burn_endpoint{
    "get_bnb_burn", boost::beast::http::verb::get, "/fapi/v1/feeBurn", types::security_type::user_data, true, true, true
};
inline constexpr endpoint_metadata toggle_bnb_burn_endpoint{
    "toggle_bnb_burn", boost::beast::http::verb::post, "/fapi/v1/feeBurn", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata quantitative_rules_endpoint{
    "quantitative_rules", boost::beast::http::verb::get, "/fapi/v1/apiTradingStatus", types::security_type::user_data, true, true, true
};

inline constexpr endpoint_metadata convert_get_quote_endpoint{
    "convert_get_quote", boost::beast::http::verb::post, "/fapi/v1/convert/getQuote", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata convert_accept_quote_endpoint{
    "convert_accept_quote", boost::beast::http::verb::post, "/fapi/v1/convert/acceptQuote", types::security_type::trade, true, true, true
};
inline constexpr endpoint_metadata convert_order_status_endpoint{
    "convert_order_status", boost::beast::http::verb::get, "/fapi/v1/convert/orderStatus", types::security_type::user_data, true, true, true
};

} // namespace binapi2::fapi::rest
