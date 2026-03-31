// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/streams/market_streams.hpp>

#include <boost/asio/post.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::fapi::streams {

namespace {

result<void>
invalid_subscription(const std::string& message)
{
    return result<void>::failure({ error_code::invalid_argument, 0, 0, message, {} });
}

template<typename Event, typename Handler>
result<void>
read_stream_loop(transport::websocket_client& transport, Handler handler)
{
    return transport.run_read_loop([handler = std::move(handler)](const std::string& payload) {
        Event event{};
        if (glz::read_json(event, payload)) {
            return false;
        }
        return handler(event);
    });
}

} // namespace

market_streams::market_streams(boost::asio::io_context& io_context, config cfg) :
    io_context_(io_context), transport_(io_context, cfg), cfg_(std::move(cfg))
{
}

result<void>
market_streams::connect_aggregate_trade(const aggregate_trade_subscription& subscription)
{
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@aggTrade";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_aggregate_trade(const aggregate_trade_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_aggregate_trade(subscription));
    });
}

result<void>
market_streams::connect_mark_price(const mark_price_subscription& subscription)
{
    const auto suffix = subscription.every_1s ? "@markPrice@1s" : "@markPrice";
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + suffix;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_mark_price(const mark_price_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_mark_price(subscription));
    });
}

result<void>
market_streams::connect_book_ticker(const book_ticker_subscription& subscription)
{
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@bookTicker";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_book_ticker(const book_ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_book_ticker(subscription));
    });
}

result<void>
market_streams::connect_diff_book_depth(const diff_book_depth_subscription& subscription)
{
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@depth@" + subscription.speed;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_diff_book_depth(const diff_book_depth_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_diff_book_depth(subscription));
    });
}

result<void>
market_streams::connect_mini_ticker(const mini_ticker_subscription& subscription)
{
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@miniTicker";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_mini_ticker(const mini_ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_mini_ticker(subscription));
    });
}

result<void>
market_streams::connect_all_market_mini_tickers(const all_market_mini_ticker_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!miniTicker@arr";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_market_mini_tickers(const all_market_mini_ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_market_mini_tickers(subscription));
    });
}

result<void>
market_streams::connect_ticker(const ticker_subscription& subscription)
{
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@ticker";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_ticker(const ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_ticker(subscription));
    });
}

result<void>
market_streams::connect_all_market_tickers(const all_market_ticker_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!ticker@arr";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_market_tickers(const all_market_ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_market_tickers(subscription));
    });
}

result<void>
market_streams::connect_all_book_tickers(const all_book_ticker_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!bookTicker";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_book_tickers(const all_book_ticker_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_book_tickers(subscription));
    });
}

result<void>
market_streams::connect_liquidation_order(const liquidation_order_subscription& subscription)
{
    if (subscription.symbol.empty()) {
        return invalid_subscription("liquidation order stream requires a symbol");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@forceOrder";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_liquidation_order(const liquidation_order_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_liquidation_order(subscription));
    });
}

result<void>
market_streams::connect_all_market_liquidation_orders(const all_market_liquidation_order_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!forceOrder@arr";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_market_liquidation_orders(const all_market_liquidation_order_subscription& subscription,
                                                      void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_market_liquidation_orders(subscription));
    });
}

result<void>
market_streams::connect_partial_book_depth(const partial_book_depth_subscription& subscription)
{
    if (subscription.symbol.empty()) {
        return invalid_subscription("partial book depth stream requires a symbol");
    }
    if (subscription.levels != 5 && subscription.levels != 10 && subscription.levels != 20) {
        return invalid_subscription("partial book depth stream levels must be 5, 10, or 20");
    }
    std::string target = cfg_.stream_base_target + "/" + subscription.symbol + "@depth" + std::to_string(subscription.levels);
    if (subscription.speed == "100ms" || subscription.speed == "500ms") {
        target += "@" + subscription.speed;
    } else if (subscription.speed != "250ms") {
        return invalid_subscription("partial book depth stream speed must be 250ms, 500ms, or 100ms");
    }
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_partial_book_depth(const partial_book_depth_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_partial_book_depth(subscription));
    });
}

result<void>
market_streams::connect_kline(const kline_subscription& subscription)
{
    const auto target =
        cfg_.stream_base_target + "/" + subscription.symbol + "@kline_" + types::to_string(subscription.interval);
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_kline(const kline_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_kline(subscription));
    });
}

result<void>
market_streams::connect_continuous_contract_kline(const continuous_contract_kline_subscription& subscription)
{
    if (subscription.pair.empty()) {
        return invalid_subscription("continuous contract kline stream requires a pair");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.pair + "_" + types::to_string(subscription.contract_type)
        + "@continuousKline_" + types::to_string(subscription.interval);
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_continuous_contract_kline(const continuous_contract_kline_subscription& subscription,
                                                  void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_continuous_contract_kline(subscription));
    });
}

result<void>
market_streams::read_aggregate_trade_loop(aggregate_trade_handler handler)
{
    return read_stream_loop<types::aggregate_trade_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_aggregate_trade_loop(aggregate_trade_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_aggregate_trade_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_mark_price_loop(mark_price_handler handler)
{
    return read_stream_loop<types::mark_price_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_mark_price_loop(mark_price_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_mark_price_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_book_ticker_loop(book_ticker_handler handler)
{
    return read_stream_loop<types::book_ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_book_ticker_loop(book_ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_book_ticker_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_diff_book_depth_loop(depth_handler handler)
{
    return read_stream_loop<types::depth_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_diff_book_depth_loop(depth_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_diff_book_depth_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_mini_ticker_loop(mini_ticker_handler handler)
{
    return read_stream_loop<types::mini_ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_mini_ticker_loop(mini_ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_mini_ticker_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler)
{
    return read_stream_loop<types::all_market_mini_ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_market_mini_tickers_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_ticker_loop(ticker_handler handler)
{
    return read_stream_loop<types::ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_ticker_loop(ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_ticker_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_all_market_tickers_loop(all_market_ticker_handler handler)
{
    return read_stream_loop<types::all_market_ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_market_tickers_loop(all_market_ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_market_tickers_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_all_book_tickers_loop(book_ticker_handler handler)
{
    return read_stream_loop<types::book_ticker_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_book_tickers_loop(book_ticker_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_book_tickers_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_liquidation_order_loop(liquidation_order_handler handler)
{
    return read_stream_loop<types::liquidation_order_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_liquidation_order_loop(liquidation_order_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_liquidation_order_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_all_market_liquidation_orders_loop(liquidation_order_handler handler)
{
    return read_stream_loop<types::liquidation_order_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_market_liquidation_orders_loop(liquidation_order_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_market_liquidation_orders_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_partial_book_depth_loop(depth_handler handler)
{
    return read_stream_loop<types::depth_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_partial_book_depth_loop(depth_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_partial_book_depth_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_kline_loop(kline_handler handler)
{
    return read_stream_loop<types::kline_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_kline_loop(kline_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_kline_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_continuous_contract_kline_loop(continuous_contract_kline_handler handler)
{
    return read_stream_loop<types::continuous_contract_kline_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_continuous_contract_kline_loop(continuous_contract_kline_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_continuous_contract_kline_loop(std::move(handler)));
    });
}

result<void>
market_streams::close()
{
    return transport_.close();
}

void
market_streams::close(void_callback callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(close()); });
}

} // namespace binapi2::fapi::streams
