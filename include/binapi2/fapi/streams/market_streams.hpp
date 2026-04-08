// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_streams.hpp
/// @brief Market data stream client for Binance USD-M Futures WebSocket streams.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <boost/asio/io_context.hpp>

#include <functional>
#include <string>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Client for Binance USD-M Futures market data WebSocket streams.
///
/// Usage follows a connect/read-loop pattern:
///  1. Call a @c connect_* method with a subscription to open the stream.
///  2. Call the matching @c read_*_loop method with a handler callback.
///     The handler receives deserialized events and returns @c true to keep
///     reading or @c false to stop.
///
/// Each @c connect_* and @c read_*_loop method has two overloads:
///  - A synchronous overload returning @c result<void>.
///  - An async overload accepting a @c void_callback for non-blocking use.
///
/// For combined streams, call @ref connect_combined first, then use
/// @ref subscribe / @ref unsubscribe to manage stream topics dynamically.
class market_streams
{
public:
    using void_callback = std::function<void(result<void>)>;                                      ///< Async completion callback type.
    using aggregate_trade_handler = std::function<bool(const types::aggregate_trade_stream_event_t&)>; ///< Handler for aggregate trade events.
    using mark_price_handler = std::function<bool(const types::mark_price_stream_event_t&)>;          ///< Handler for mark price events.
    using book_ticker_handler = std::function<bool(const types::book_ticker_stream_event_t&)>;        ///< Handler for individual book ticker events.
    using depth_handler = std::function<bool(const types::depth_stream_event_t&)>;                    ///< Handler for order book depth events.
    using mini_ticker_handler = std::function<bool(const types::mini_ticker_stream_event_t&)>;        ///< Handler for mini ticker events.
    using all_market_mini_ticker_handler = std::function<bool(const types::all_market_mini_ticker_stream_event&)>; ///< Handler for all-market mini ticker events.
    using ticker_handler = std::function<bool(const types::ticker_stream_event_t&)>;                  ///< Handler for 24hr ticker events.
    using all_market_ticker_handler = std::function<bool(const types::all_market_ticker_stream_event&)>;           ///< Handler for all-market 24hr ticker events.
    using kline_handler = std::function<bool(const types::kline_stream_event_t&)>;                    ///< Handler for kline_t/candlestick events.
    using liquidation_order_handler = std::function<bool(const types::liquidation_order_stream_event_t&)>;           ///< Handler for liquidation order events.
    using continuous_contract_kline_handler = std::function<bool(const types::continuous_contract_kline_stream_event_t&)>; ///< Handler for continuous contract kline_t events.
    using all_market_mark_price_handler = std::function<bool(const types::all_market_mark_price_stream_event&)>;   ///< Handler for all-market mark price events.
    using composite_index_handler = std::function<bool(const types::composite_index_stream_event_t&)>; ///< Handler for composite index events.
    using contract_info_handler = std::function<bool(const types::contract_info_stream_event_t&)>;    ///< Handler for contract info events.
    using asset_index_handler = std::function<bool(const types::asset_index_stream_event_t&)>;        ///< Handler for asset index events.
    using all_asset_index_handler = std::function<bool(const types::all_asset_index_stream_event&)>; ///< Handler for all asset index events.
    using trading_session_handler = std::function<bool(const types::trading_session_stream_event_t&)>; ///< Handler for trading session events.

    /// @brief Construct a market streams client with sync + async support.
    /// @param io  The io_thread that owns the io_context.
    /// @param cfg Configuration containing the stream endpoint URL.
    market_streams(detail::io_thread& io, config cfg);

    /// @brief Construct an async-only market streams client (no io_thread).
    /// @param cfg Configuration containing the stream endpoint URL.
    explicit market_streams(config cfg);

    // -- Aggregate trade stream --

    /// @brief Connect to the aggregate trade stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    /// @return A result indicating success or connection error.
    [[nodiscard]] result<void> connect_aggregate_trade(const types::aggregate_trade_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_aggregate_trade(const types::aggregate_trade_subscription& subscription, void_callback callback);

    // -- Mark price stream --

    /// @brief Connect to the mark price stream for a symbol.
    /// @param subscription Subscription parameters (symbol, update frequency).
    [[nodiscard]] result<void> connect_mark_price(const types::mark_price_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_mark_price(const types::mark_price_subscription& subscription, void_callback callback);

    // -- Book ticker stream --

    /// @brief Connect to the individual symbol book ticker stream.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_book_ticker(const types::book_ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_book_ticker(const types::book_ticker_subscription& subscription, void_callback callback);

    // -- Diff book depth stream --

    /// @brief Connect to the diff book depth stream for a symbol.
    /// @param subscription Subscription parameters (symbol, update speed).
    [[nodiscard]] result<void> connect_diff_book_depth(const types::diff_book_depth_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_diff_book_depth(const types::diff_book_depth_subscription& subscription, void_callback callback);

    // -- Mini ticker stream --

    /// @brief Connect to the mini ticker stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_mini_ticker(const types::mini_ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_mini_ticker(const types::mini_ticker_subscription& subscription, void_callback callback);

    // -- All market mini tickers --

    /// @brief Connect to the all-market mini ticker stream.
    /// @param subscription Subscription parameters (default-constructed for all markets).
    [[nodiscard]] result<void> connect_all_market_mini_tickers(const types::all_market_mini_ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_market_mini_tickers(const types::all_market_mini_ticker_subscription& subscription, void_callback callback);

    // -- Ticker stream --

    /// @brief Connect to the 24hr ticker stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_ticker(const types::ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_ticker(const types::ticker_subscription& subscription, void_callback callback);

    // -- All market tickers --

    /// @brief Connect to the all-market 24hr ticker stream.
    [[nodiscard]] result<void> connect_all_market_tickers(const types::all_market_ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_market_tickers(const types::all_market_ticker_subscription& subscription, void_callback callback);

    // -- All book tickers --

    /// @brief Connect to the all-market book ticker stream.
    [[nodiscard]] result<void> connect_all_book_tickers(const types::all_book_ticker_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_book_tickers(const types::all_book_ticker_subscription& subscription, void_callback callback);

    // -- Liquidation orders --

    /// @brief Connect to the liquidation order stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_liquidation_order(const types::liquidation_order_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_liquidation_order(const types::liquidation_order_subscription& subscription, void_callback callback);

    // -- All market liquidation orders --

    /// @brief Connect to the all-market liquidation order stream.
    [[nodiscard]] result<void> connect_all_market_liquidation_orders(const types::all_market_liquidation_order_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_market_liquidation_orders(const types::all_market_liquidation_order_subscription& subscription, void_callback callback);

    // -- Partial book depth --

    /// @brief Connect to the partial book depth stream for a symbol.
    /// @param subscription Subscription parameters (symbol, depth levels, speed).
    [[nodiscard]] result<void> connect_partial_book_depth(const types::partial_book_depth_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_partial_book_depth(const types::partial_book_depth_subscription& subscription, void_callback callback);

    // -- Kline stream --

    /// @brief Connect to the kline_t/candlestick stream for a symbol.
    /// @param subscription Subscription parameters (symbol, interval).
    [[nodiscard]] result<void> connect_kline(const types::kline_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_kline(const types::kline_subscription& subscription, void_callback callback);

    // -- Continuous contract kline_t --

    /// @brief Connect to the continuous contract kline_t stream.
    /// @param subscription Subscription parameters (pair, contract type, interval).
    [[nodiscard]] result<void> connect_continuous_contract_kline(const types::continuous_contract_kline_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_continuous_contract_kline(const types::continuous_contract_kline_subscription& subscription, void_callback callback);

    // -- Read loops (connect first, then call the matching read loop) --

    /// @brief Read aggregate trade events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_aggregate_trade_loop(aggregate_trade_handler handler);
    /// @brief Async overload with completion callback.
    void read_aggregate_trade_loop(aggregate_trade_handler handler, void_callback callback);

    /// @brief Read mark price events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_mark_price_loop(mark_price_handler handler);
    /// @brief Async overload with completion callback.
    void read_mark_price_loop(mark_price_handler handler, void_callback callback);

    /// @brief Read book ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_book_ticker_loop(book_ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_book_ticker_loop(book_ticker_handler handler, void_callback callback);

    /// @brief Read diff book depth events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_diff_book_depth_loop(depth_handler handler);
    /// @brief Async overload with completion callback.
    void read_diff_book_depth_loop(depth_handler handler, void_callback callback);

    /// @brief Read mini ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_mini_ticker_loop(mini_ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_mini_ticker_loop(mini_ticker_handler handler, void_callback callback);

    /// @brief Read all-market mini ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_market_mini_tickers_loop(all_market_mini_ticker_handler handler, void_callback callback);

    /// @brief Read 24hr ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_ticker_loop(ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_ticker_loop(ticker_handler handler, void_callback callback);

    /// @brief Read all-market 24hr ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_market_tickers_loop(all_market_ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_market_tickers_loop(all_market_ticker_handler handler, void_callback callback);

    /// @brief Read all-market book ticker events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_book_tickers_loop(book_ticker_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_book_tickers_loop(book_ticker_handler handler, void_callback callback);

    /// @brief Read liquidation order events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_liquidation_order_loop(liquidation_order_handler handler);
    /// @brief Async overload with completion callback.
    void read_liquidation_order_loop(liquidation_order_handler handler, void_callback callback);

    /// @brief Read all-market liquidation order events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_market_liquidation_orders_loop(liquidation_order_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_market_liquidation_orders_loop(liquidation_order_handler handler, void_callback callback);

    /// @brief Read partial book depth events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_partial_book_depth_loop(depth_handler handler);
    /// @brief Async overload with completion callback.
    void read_partial_book_depth_loop(depth_handler handler, void_callback callback);

    /// @brief Read kline_t/candlestick events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_kline_loop(kline_handler handler);
    /// @brief Async overload with completion callback.
    void read_kline_loop(kline_handler handler, void_callback callback);

    /// @brief Read continuous contract kline_t events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_continuous_contract_kline_loop(continuous_contract_kline_handler handler);
    /// @brief Async overload with completion callback.
    void read_continuous_contract_kline_loop(continuous_contract_kline_handler handler, void_callback callback);

    // -- Additional streams --

    /// @brief Connect to the all-market mark price stream.
    /// @param subscription Subscription parameters (update frequency).
    [[nodiscard]] result<void> connect_all_market_mark_price(const types::all_market_mark_price_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_market_mark_price(const types::all_market_mark_price_subscription& subscription, void_callback callback);

    /// @brief Connect to the composite index stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_composite_index(const types::composite_index_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_composite_index(const types::composite_index_subscription& subscription, void_callback callback);

    /// @brief Connect to the contract info stream.
    /// @param subscription Subscription parameters (default-constructed for all contracts).
    [[nodiscard]] result<void> connect_contract_info(const types::contract_info_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_contract_info(const types::contract_info_subscription& subscription, void_callback callback);

    /// @brief Connect to the asset index stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_asset_index(const types::asset_index_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_asset_index(const types::asset_index_subscription& subscription, void_callback callback);

    /// @brief Connect to the all asset index stream.
    [[nodiscard]] result<void> connect_all_asset_index(const types::all_asset_index_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_all_asset_index(const types::all_asset_index_subscription& subscription, void_callback callback);

    /// @brief Connect to the trading session stream.
    [[nodiscard]] result<void> connect_trading_session(const types::trading_session_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_trading_session(const types::trading_session_subscription& subscription, void_callback callback);

    /// @brief Connect to the RPI diff book depth stream for a symbol.
    /// @param subscription Subscription parameters (symbol).
    [[nodiscard]] result<void> connect_rpi_diff_book_depth(const types::rpi_diff_book_depth_subscription& subscription);
    /// @brief Async overload with completion callback.
    void connect_rpi_diff_book_depth(const types::rpi_diff_book_depth_subscription& subscription, void_callback callback);

    // -- Additional read loops --

    /// @brief Read all-market mark price events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_market_mark_price_loop(all_market_mark_price_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_market_mark_price_loop(all_market_mark_price_handler handler, void_callback callback);

    /// @brief Read composite index events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_composite_index_loop(composite_index_handler handler);
    /// @brief Async overload with completion callback.
    void read_composite_index_loop(composite_index_handler handler, void_callback callback);

    /// @brief Read contract info events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_contract_info_loop(contract_info_handler handler);
    /// @brief Async overload with completion callback.
    void read_contract_info_loop(contract_info_handler handler, void_callback callback);

    /// @brief Read asset index events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_asset_index_loop(asset_index_handler handler);
    /// @brief Async overload with completion callback.
    void read_asset_index_loop(asset_index_handler handler, void_callback callback);

    /// @brief Read all asset index events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_all_asset_index_loop(all_asset_index_handler handler);
    /// @brief Async overload with completion callback.
    void read_all_asset_index_loop(all_asset_index_handler handler, void_callback callback);

    /// @brief Read trading session events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_trading_session_loop(trading_session_handler handler);
    /// @brief Async overload with completion callback.
    void read_trading_session_loop(trading_session_handler handler, void_callback callback);

    /// @brief Read RPI diff book depth events in a loop.
    /// @param handler Callback for each event; return @c false to stop.
    [[nodiscard]] result<void> read_rpi_diff_book_depth_loop(depth_handler handler);
    /// @brief Async overload with completion callback.
    void read_rpi_diff_book_depth_loop(depth_handler handler, void_callback callback);

    // -- Combined stream management --

    /// @brief Connect to the combined stream endpoint.
    ///
    /// After connecting, use @ref subscribe and @ref unsubscribe to manage
    /// which stream topics are active on this single connection.
    ///
    /// @param target WebSocket target path (default: "/stream").
    /// @return A result indicating success or connection error.
    [[nodiscard]] result<void> connect_combined(const std::string& target = "/stream");
    /// @brief Async overload with completion callback.
    void connect_combined(const std::string& target, void_callback callback);

    /// @brief Subscribe to one or more stream topics on a combined connection.
    /// @param streams Vector of stream names (e.g. "btcusdt@aggTrade").
    /// @return A result indicating success or error.
    [[nodiscard]] result<void> subscribe(const std::vector<std::string>& streams);

    /// @brief Unsubscribe from one or more stream topics.
    /// @param streams Vector of stream names to unsubscribe from.
    /// @return A result indicating success or error.
    [[nodiscard]] result<void> unsubscribe(const std::vector<std::string>& streams);

    /// @brief List currently active subscriptions on the combined connection.
    /// @return A result containing the vector of active stream names.
    [[nodiscard]] result<std::vector<std::string>> list_subscriptions();

    /// @brief Synchronously close the stream connection.
    [[nodiscard]] result<void> close();
    /// @brief Async overload with completion callback.
    void close(void_callback callback);

    // -- Async (cobalt::task) transport access --

    /// @brief Asynchronously connect to a stream endpoint.
    /// @param target Full WebSocket target path (e.g. "/ws/btcusdt@bookTicker").
    [[nodiscard]] boost::cobalt::task<result<void>> async_connect(std::string target);

    /// @brief Asynchronously read a single raw text frame.
    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text();

    /// @brief Asynchronously close the stream connection.
    [[nodiscard]] boost::cobalt::task<result<void>> async_close();

    /// @brief Access the stream config (for building target paths externally).
    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }

private:
    boost::asio::io_context* io_context_{};  // nullptr in async-only mode
    transport::websocket_client transport_;
    config cfg_;
};

} // namespace binapi2::fapi::streams
