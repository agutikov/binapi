#include <binapi2/umf/streams/market_streams.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::umf::streams {

namespace {

template <typename Event, typename Handler>
result<void> read_stream_loop(transport::websocket_client &transport, Handler handler) {
    return transport.run_read_loop([handler = std::move(handler)](const std::string &payload) {
        Event event{};
        if (glz::read_json(event, payload)) {
            return false;
        }
        return handler(event);
    });
}

} // namespace

market_streams::market_streams(boost::asio::io_context &io_context, config cfg)
    : transport_(io_context, cfg)
    , cfg_(std::move(cfg)) {}

result<void> market_streams::connect_aggregate_trade(const aggregate_trade_subscription &subscription) {
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@aggTrade";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

result<void> market_streams::connect_mark_price(const mark_price_subscription &subscription) {
    const auto suffix = subscription.every_1s ? "@markPrice@1s" : "@markPrice";
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + suffix;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

result<void> market_streams::connect_book_ticker(const book_ticker_subscription &subscription) {
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@bookTicker";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

result<void> market_streams::connect_diff_book_depth(const diff_book_depth_subscription &subscription) {
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@depth@" + subscription.speed;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

result<void> market_streams::read_aggregate_trade_loop(aggregate_trade_handler handler) {
    return read_stream_loop<types::aggregate_trade_stream_event>(transport_, std::move(handler));
}

result<void> market_streams::read_mark_price_loop(mark_price_handler handler) {
    return read_stream_loop<types::mark_price_stream_event>(transport_, std::move(handler));
}

result<void> market_streams::read_book_ticker_loop(book_ticker_handler handler) {
    return read_stream_loop<types::book_ticker_stream_event>(transport_, std::move(handler));
}

result<void> market_streams::read_diff_book_depth_loop(depth_handler handler) {
    return read_stream_loop<types::depth_stream_event>(transport_, std::move(handler));
}

result<void> market_streams::close() {
    return transport_.close();
}

} // namespace binapi2::umf::streams
