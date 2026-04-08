// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements market data WebSocket stream subscriptions. Each stream
/// type follows a two-phase pattern:
///   1. connect_*() opens a WebSocket connection to a Binance stream endpoint
///      whose URL is built from the subscription parameters (symbol, speed,
///      interval, etc.).
///   2. read_*_loop() pumps incoming JSON frames through read_stream_loop,
///      which deserialises each frame into the appropriate event type and
///      invokes the user-supplied handler. The handler returns false to stop.
///
/// Every method has both a synchronous variant (blocks the calling thread) and
/// an async variant that posts the work onto the io_context via
/// boost::asio::post, invoking a callback on completion. The stream_control
/// helpers (subscribe/unsubscribe/list_subscriptions) use the Binance combined
/// stream protocol to manage multiple subscriptions over a single connection.

#include <binapi2/fapi/streams/market_streams.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>

#include <boost/asio/post.hpp>

#include <glaze/glaze.hpp>

// Wire types for the Binance combined stream control protocol. These are used
// to send SUBSCRIBE/UNSUBSCRIBE/LIST_SUBSCRIPTIONS commands over an existing
// WebSocket connection when using the combined stream endpoint.
namespace binapi2::fapi::streams::detail {

struct stream_control_request
{
    std::string method{};
    std::vector<std::string> params{};
    unsigned int id{};
};

struct stream_list_response
{
    std::vector<std::string> result{};
    unsigned int id{};
};

} // namespace binapi2::fapi::streams::detail

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_control_request>
{
    using T = binapi2::fapi::streams::detail::stream_control_request;
    static constexpr auto value = object("method", &T::method, "params", &T::params, "id", &T::id);
};

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_list_response>
{
    using T = binapi2::fapi::streams::detail::stream_list_response;
    static constexpr auto value = object("result", &T::result, "id", &T::id);
};

namespace binapi2::fapi::streams {

namespace {

result<void>
invalid_subscription(const std::string& message)
{
    return result<void>::failure({ error_code::invalid_argument, 0, 0, message, {} });
}

// Generic stream read loop: reads raw JSON text frames from the WebSocket,
// deserialises each into the specified Event type, and passes it to the
// handler. Returns on handler returning false or on a JSON parse failure
// (which is treated as a terminal condition to avoid silently losing data).
template<typename Event, typename Handler>
result<void>
read_stream_loop(transport::websocket_client& transport, Handler handler)
{
    return transport.run_read_loop([handler = std::move(handler)](const std::string& payload) {
        Event event{};
        glz::context ctx{};
        if (glz::read<fapi::detail::json_read_opts>(event, payload, ctx)) {
            return false;
        }
        return handler(event);
    });
}

} // namespace

market_streams::market_streams(fapi::detail::io_thread& io, config cfg) :
    io_context_(io.context()), transport_(io, cfg), cfg_(std::move(cfg))
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
        return invalid_subscription("continuous contract kline_t stream requires a pair");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.pair + "_" + types::to_string(subscription.contract_type_t)
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
    return read_stream_loop<types::aggregate_trade_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::mark_price_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::book_ticker_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::depth_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::mini_ticker_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::ticker_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::book_ticker_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::liquidation_order_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::liquidation_order_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::depth_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::kline_stream_event_t>(transport_, std::move(handler));
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
    return read_stream_loop<types::continuous_contract_kline_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_continuous_contract_kline_loop(continuous_contract_kline_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_continuous_contract_kline_loop(std::move(handler)));
    });
}

result<void>
market_streams::connect_all_market_mark_price(const all_market_mark_price_subscription& subscription)
{
    const auto suffix = subscription.every_1s ? "/!markPrice@arr@1s" : "/!markPrice@arr";
    const auto target = cfg_.stream_base_target + suffix;
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_market_mark_price(const all_market_mark_price_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_market_mark_price(subscription));
    });
}

result<void>
market_streams::connect_composite_index(const composite_index_subscription& subscription)
{
    if (subscription.symbol.empty()) {
        return invalid_subscription("composite index stream requires a symbol");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@compositeIndex";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_composite_index(const composite_index_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_composite_index(subscription));
    });
}

result<void>
market_streams::connect_contract_info(const contract_info_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!contractInfo";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_contract_info(const contract_info_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_contract_info(subscription));
    });
}

result<void>
market_streams::connect_asset_index(const asset_index_subscription& subscription)
{
    if (subscription.symbol.empty()) {
        return invalid_subscription("asset index stream requires a symbol");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@assetIndex";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_asset_index(const asset_index_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_asset_index(subscription));
    });
}

result<void>
market_streams::connect_all_asset_index(const all_asset_index_subscription&)
{
    const auto target = cfg_.stream_base_target + "/!assetIndex@arr";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_all_asset_index(const all_asset_index_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_all_asset_index(subscription));
    });
}

result<void>
market_streams::connect_trading_session(const trading_session_subscription&)
{
    const auto target = cfg_.stream_base_target + "/tradingSession";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_trading_session(const trading_session_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_trading_session(subscription));
    });
}

result<void>
market_streams::connect_rpi_diff_book_depth(const rpi_diff_book_depth_subscription& subscription)
{
    if (subscription.symbol.empty()) {
        return invalid_subscription("RPI diff book depth stream requires a symbol");
    }
    const auto target = cfg_.stream_base_target + "/" + subscription.symbol + "@rpiDepth@500ms";
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_rpi_diff_book_depth(const rpi_diff_book_depth_subscription& subscription, void_callback callback)
{
    boost::asio::post(io_context_, [this, subscription, callback = std::move(callback)]() mutable {
        callback(connect_rpi_diff_book_depth(subscription));
    });
}

result<void>
market_streams::read_all_market_mark_price_loop(all_market_mark_price_handler handler)
{
    return read_stream_loop<types::all_market_mark_price_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_market_mark_price_loop(all_market_mark_price_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_market_mark_price_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_composite_index_loop(composite_index_handler handler)
{
    return read_stream_loop<types::composite_index_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_composite_index_loop(composite_index_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_composite_index_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_contract_info_loop(contract_info_handler handler)
{
    return read_stream_loop<types::contract_info_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_contract_info_loop(contract_info_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_contract_info_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_asset_index_loop(asset_index_handler handler)
{
    return read_stream_loop<types::asset_index_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_asset_index_loop(asset_index_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_asset_index_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_all_asset_index_loop(all_asset_index_handler handler)
{
    return read_stream_loop<types::all_asset_index_stream_event>(transport_, std::move(handler));
}

void
market_streams::read_all_asset_index_loop(all_asset_index_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_all_asset_index_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_trading_session_loop(trading_session_handler handler)
{
    return read_stream_loop<types::trading_session_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_trading_session_loop(trading_session_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_trading_session_loop(std::move(handler)));
    });
}

result<void>
market_streams::read_rpi_diff_book_depth_loop(depth_handler handler)
{
    return read_stream_loop<types::depth_stream_event_t>(transport_, std::move(handler));
}

void
market_streams::read_rpi_diff_book_depth_loop(depth_handler handler, void_callback callback)
{
    boost::asio::post(io_context_, [this, handler = std::move(handler), callback = std::move(callback)]() mutable {
        callback(read_rpi_diff_book_depth_loop(std::move(handler)));
    });
}

result<void>
market_streams::connect_combined(const std::string& target)
{
    return transport_.connect(cfg_.stream_host, cfg_.stream_port, target);
}

void
market_streams::connect_combined(const std::string& target, void_callback callback)
{
    boost::asio::post(io_context_,
                      [this, target, callback = std::move(callback)]() mutable { callback(connect_combined(target)); });
}

// Sends a SUBSCRIBE control message over the combined stream connection.
// The response is read and discarded; Binance returns a null result on
// success. Any transport or serialization error is propagated.
result<void>
market_streams::subscribe(const std::vector<std::string>& streams)
{
    detail::stream_control_request request{ "SUBSCRIBE", streams, 1 };
    auto payload = glz::write_json(request);
    if (!payload) {
        return result<void>::failure({ error_code::json, 0, 0, "failed to serialize subscribe request", {} });
    }
    auto write_result = transport_.write_text(*payload);
    if (!write_result) {
        return result<void>::failure(write_result.err);
    }
    auto raw = transport_.read_text();
    if (!raw) {
        return result<void>::failure(raw.err);
    }
    return result<void>::success();
}

result<void>
market_streams::unsubscribe(const std::vector<std::string>& streams)
{
    detail::stream_control_request request{ "UNSUBSCRIBE", streams, 2 };
    auto payload = glz::write_json(request);
    if (!payload) {
        return result<void>::failure({ error_code::json, 0, 0, "failed to serialize unsubscribe request", {} });
    }
    auto write_result = transport_.write_text(*payload);
    if (!write_result) {
        return result<void>::failure(write_result.err);
    }
    auto raw = transport_.read_text();
    if (!raw) {
        return result<void>::failure(raw.err);
    }
    return result<void>::success();
}

result<std::vector<std::string>>
market_streams::list_subscriptions()
{
    detail::stream_control_request request{ "LIST_SUBSCRIPTIONS", {}, 3 };
    auto payload = glz::write_json(request);
    if (!payload) {
        return result<std::vector<std::string>>::failure(
            { error_code::json, 0, 0, "failed to serialize list subscriptions request", {} });
    }
    auto write_result = transport_.write_text(*payload);
    if (!write_result) {
        return result<std::vector<std::string>>::failure(write_result.err);
    }
    auto raw = transport_.read_text();
    if (!raw) {
        return result<std::vector<std::string>>::failure(raw.err);
    }
    detail::stream_list_response response{};
    glz::context glz_ctx{};
    if (auto ec = glz::read<fapi::detail::json_read_opts>(response, *raw, glz_ctx)) {
        return result<std::vector<std::string>>::failure(
            { error_code::json, 0, 0, glz::format_error(ec, *raw), *raw });
    }
    return result<std::vector<std::string>>::success(std::move(response.result));
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
