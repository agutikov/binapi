#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/result.hpp>
#include <binapi2/umf/streams/subscriptions.hpp>
#include <binapi2/umf/transport/websocket_client.hpp>
#include <binapi2/umf/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>

namespace binapi2::umf::streams {

class market_streams {
  public:
    using aggregate_trade_handler = std::function<bool(const types::aggregate_trade_stream_event &)>;
    using mark_price_handler = std::function<bool(const types::mark_price_stream_event &)>;
    using book_ticker_handler = std::function<bool(const types::book_ticker_stream_event &)>;
    using depth_handler = std::function<bool(const types::depth_stream_event &)>;

    market_streams(boost::asio::io_context &io_context, config cfg);

    [[nodiscard]] result<void> connect_aggregate_trade(const aggregate_trade_subscription &subscription);
    [[nodiscard]] result<void> connect_mark_price(const mark_price_subscription &subscription);
    [[nodiscard]] result<void> connect_book_ticker(const book_ticker_subscription &subscription);
    [[nodiscard]] result<void> connect_diff_book_depth(const diff_book_depth_subscription &subscription);
    [[nodiscard]] result<void> read_aggregate_trade_loop(aggregate_trade_handler handler);
    [[nodiscard]] result<void> read_mark_price_loop(mark_price_handler handler);
    [[nodiscard]] result<void> read_book_ticker_loop(book_ticker_handler handler);
    [[nodiscard]] result<void> read_diff_book_depth_loop(depth_handler handler);
    [[nodiscard]] result<void> close();

  private:
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::umf::streams
