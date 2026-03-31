// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/subscriptions.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <string>
#include <vector>

namespace binapi2::fapi::streams {

class market_streams
{
public:
    using void_callback = std::function<void(result<void>)>;
    using aggregate_trade_handler = std::function<bool(const types::aggregate_trade_stream_event&)>;
    using mark_price_handler = std::function<bool(const types::mark_price_stream_event&)>;
    using book_ticker_handler = std::function<bool(const types::book_ticker_stream_event&)>;
    using depth_handler = std::function<bool(const types::depth_stream_event&)>;
    using mini_ticker_handler = std::function<bool(const types::mini_ticker_stream_event&)>;
    using all_market_mini_ticker_handler = std::function<bool(const types::all_market_mini_ticker_stream_event&)>;
    using ticker_handler = std::function<bool(const types::ticker_stream_event&)>;
    using all_market_ticker_handler = std::function<bool(const types::all_market_ticker_stream_event&)>;
    using kline_handler = std::function<bool(const types::kline_stream_event&)>;
    using liquidation_order_handler = std::function<bool(const types::liquidation_order_stream_event&)>;
    using continuous_contract_kline_handler = std::function<bool(const types::continuous_contract_kline_stream_event&)>;
    using all_market_mark_price_handler = std::function<bool(const types::all_market_mark_price_stream_event&)>;
    using composite_index_handler = std::function<bool(const types::composite_index_stream_event&)>;
    using contract_info_handler = std::function<bool(const types::contract_info_stream_event&)>;
    using asset_index_handler = std::function<bool(const types::asset_index_stream_event&)>;
    using all_asset_index_handler = std::function<bool(const types::all_asset_index_stream_event&)>;
    using trading_session_handler = std::function<bool(const types::trading_session_stream_event&)>;

    market_streams(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] result<void> connect_aggregate_trade(const aggregate_trade_subscription& subscription);
    void connect_aggregate_trade(const aggregate_trade_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_mark_price(const mark_price_subscription& subscription);
    void connect_mark_price(const mark_price_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_book_ticker(const book_ticker_subscription& subscription);
    void connect_book_ticker(const book_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_diff_book_depth(const diff_book_depth_subscription& subscription);
    void connect_diff_book_depth(const diff_book_depth_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_mini_ticker(const mini_ticker_subscription& subscription);
    void connect_mini_ticker(const mini_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_all_market_mini_tickers(const all_market_mini_ticker_subscription& subscription = {});
    void connect_all_market_mini_tickers(const all_market_mini_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_ticker(const ticker_subscription& subscription);
    void connect_ticker(const ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_all_market_tickers(const all_market_ticker_subscription& subscription = {});
    void connect_all_market_tickers(const all_market_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_all_book_tickers(const all_book_ticker_subscription& subscription = {});
    void connect_all_book_tickers(const all_book_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_liquidation_order(const liquidation_order_subscription& subscription);
    void connect_liquidation_order(const liquidation_order_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_all_market_liquidation_orders(const all_market_liquidation_order_subscription& subscription = {});
    void connect_all_market_liquidation_orders(const all_market_liquidation_order_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_partial_book_depth(const partial_book_depth_subscription& subscription);
    void connect_partial_book_depth(const partial_book_depth_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_kline(const kline_subscription& subscription);
    void connect_kline(const kline_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_continuous_contract_kline(const continuous_contract_kline_subscription& subscription);
    void connect_continuous_contract_kline(const continuous_contract_kline_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> read_aggregate_trade_loop(aggregate_trade_handler handler);
    void read_aggregate_trade_loop(aggregate_trade_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_mark_price_loop(mark_price_handler handler);
    void read_mark_price_loop(mark_price_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_book_ticker_loop(book_ticker_handler handler);
    void read_book_ticker_loop(book_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_diff_book_depth_loop(depth_handler handler);
    void read_diff_book_depth_loop(depth_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_mini_ticker_loop(mini_ticker_handler handler);
    void read_mini_ticker_loop(mini_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler);
    void read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_ticker_loop(ticker_handler handler);
    void read_ticker_loop(ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_all_market_tickers_loop(all_market_ticker_handler handler);
    void read_all_market_tickers_loop(all_market_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_all_book_tickers_loop(book_ticker_handler handler);
    void read_all_book_tickers_loop(book_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_liquidation_order_loop(liquidation_order_handler handler);
    void read_liquidation_order_loop(liquidation_order_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_all_market_liquidation_orders_loop(liquidation_order_handler handler);
    void read_all_market_liquidation_orders_loop(liquidation_order_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_partial_book_depth_loop(depth_handler handler);
    void read_partial_book_depth_loop(depth_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_kline_loop(kline_handler handler);
    void read_kline_loop(kline_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_continuous_contract_kline_loop(continuous_contract_kline_handler handler);
    void read_continuous_contract_kline_loop(continuous_contract_kline_handler handler, void_callback callback);
    [[nodiscard]] result<void> connect_all_market_mark_price(const all_market_mark_price_subscription& subscription = {});
    void connect_all_market_mark_price(const all_market_mark_price_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_composite_index(const composite_index_subscription& subscription);
    void connect_composite_index(const composite_index_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_contract_info(const contract_info_subscription& subscription = {});
    void connect_contract_info(const contract_info_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_asset_index(const asset_index_subscription& subscription);
    void connect_asset_index(const asset_index_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_all_asset_index(const all_asset_index_subscription& subscription = {});
    void connect_all_asset_index(const all_asset_index_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_trading_session(const trading_session_subscription& subscription = {});
    void connect_trading_session(const trading_session_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_rpi_diff_book_depth(const rpi_diff_book_depth_subscription& subscription);
    void connect_rpi_diff_book_depth(const rpi_diff_book_depth_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> read_all_market_mark_price_loop(all_market_mark_price_handler handler);
    void read_all_market_mark_price_loop(all_market_mark_price_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_composite_index_loop(composite_index_handler handler);
    void read_composite_index_loop(composite_index_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_contract_info_loop(contract_info_handler handler);
    void read_contract_info_loop(contract_info_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_asset_index_loop(asset_index_handler handler);
    void read_asset_index_loop(asset_index_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_all_asset_index_loop(all_asset_index_handler handler);
    void read_all_asset_index_loop(all_asset_index_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_trading_session_loop(trading_session_handler handler);
    void read_trading_session_loop(trading_session_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_rpi_diff_book_depth_loop(depth_handler handler);
    void read_rpi_diff_book_depth_loop(depth_handler handler, void_callback callback);
    [[nodiscard]] result<void> connect_combined(const std::string& target = "/stream");
    void connect_combined(const std::string& target, void_callback callback);
    [[nodiscard]] result<void> subscribe(const std::vector<std::string>& streams);
    [[nodiscard]] result<void> unsubscribe(const std::vector<std::string>& streams);
    [[nodiscard]] result<std::vector<std::string>> list_subscriptions();
    [[nodiscard]] result<void> close();
    void close(void_callback callback);

private:
    boost::asio::io_context& io_context_;
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
