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

namespace binapi2::fapi::streams {

class market_streams
{
public:
    using void_callback = std::function<void(result<void>)>;
    using aggregate_trade_handler = std::function<bool(const types::aggregate_trade_stream_event&)>;
    using mark_price_handler = std::function<bool(const types::mark_price_stream_event&)>;
    using book_ticker_handler = std::function<bool(const types::book_ticker_stream_event&)>;
    using depth_handler = std::function<bool(const types::depth_stream_event&)>;

    market_streams(boost::asio::io_context& io_context, config cfg);

    [[nodiscard]] result<void> connect_aggregate_trade(const aggregate_trade_subscription& subscription);
    void connect_aggregate_trade(const aggregate_trade_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_mark_price(const mark_price_subscription& subscription);
    void connect_mark_price(const mark_price_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_book_ticker(const book_ticker_subscription& subscription);
    void connect_book_ticker(const book_ticker_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> connect_diff_book_depth(const diff_book_depth_subscription& subscription);
    void connect_diff_book_depth(const diff_book_depth_subscription& subscription, void_callback callback);
    [[nodiscard]] result<void> read_aggregate_trade_loop(aggregate_trade_handler handler);
    void read_aggregate_trade_loop(aggregate_trade_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_mark_price_loop(mark_price_handler handler);
    void read_mark_price_loop(mark_price_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_book_ticker_loop(book_ticker_handler handler);
    void read_book_ticker_loop(book_ticker_handler handler, void_callback callback);
    [[nodiscard]] result<void> read_diff_book_depth_loop(depth_handler handler);
    void read_diff_book_depth_loop(depth_handler handler, void_callback callback);
    [[nodiscard]] result<void> close();
    void close(void_callback callback);

private:
    boost::asio::io_context& io_context_;
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
