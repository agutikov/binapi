// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file market_stream_events.hpp
/// @brief Event types for public market data WebSocket streams.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace binapi2::fapi::types {


// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Book-Ticker-Streams.md
struct book_ticker_stream_event_t
{
    market_event_type_t event_type{};
    std::uint64_t update_id{};
    symbol_t symbol{};
    decimal_t best_bid_price{};
    decimal_t best_bid_qty{};
    decimal_t best_ask_price{};
    decimal_t best_ask_qty{};
    timestamp_ms_t transaction_time{};
    timestamp_ms_t event_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Aggregate-Trade-Streams.md
struct aggregate_trade_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    std::uint64_t agg_trade_id{};
    decimal_t price{};
    decimal_t quantity{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    timestamp_ms_t trade_time{};
    bool is_buyer_maker{};
    std::optional<decimal_t> normal_quantity{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream.md
struct mark_price_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t mark_price_t{};
    std::optional<decimal_t> mark_price_avg{};
    decimal_t index_price{};
    decimal_t settle_price{};
    decimal_t funding_rate{};
    timestamp_ms_t next_funding_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream-for-All-market.md
using all_market_mark_price_stream_event = std::vector<mark_price_stream_event_t>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams.md
struct depth_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    symbol_t symbol{};
    std::uint64_t first_update_id{};
    std::uint64_t final_update_id{};
    std::uint64_t prev_final_update_id{};
    std::vector<price_level_t> bids{};
    std::vector<price_level_t> asks{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
struct mini_ticker_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t close_price{};
    decimal_t open_price{};
    decimal_t high_price{};
    decimal_t low_price{};
    decimal_t base_volume{};
    decimal_t quote_volume{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
using all_market_mini_ticker_stream_event = std::vector<mini_ticker_stream_event_t>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
struct ticker_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t price_change{};
    decimal_t price_change_pct{};
    decimal_t weighted_avg_price{};
    decimal_t last_price{};
    decimal_t last_quantity{};
    decimal_t open_price{};
    decimal_t high_price{};
    decimal_t low_price{};
    decimal_t base_volume{};
    decimal_t quote_volume{};
    timestamp_ms_t stats_open_time{};
    timestamp_ms_t stats_close_time{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    std::uint64_t trade_count{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
using all_market_ticker_stream_event = std::vector<ticker_stream_event_t>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_data_t
{
    symbol_t symbol{};
    order_side_t side{};
    order_type_t type{};
    time_in_force_t tif{};
    decimal_t original_quantity{};
    decimal_t price{};
    decimal_t average_price{};
    order_status_t status{};
    decimal_t last_filled_qty{};
    decimal_t filled_accum_qty{};
    timestamp_ms_t trade_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    liquidation_order_stream_data_t order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
struct kline_stream_data_t
{
    timestamp_ms_t open_time{};
    timestamp_ms_t close_time{};
    symbol_t symbol{};
    kline_interval_t interval{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    decimal_t open_price{};
    decimal_t close_price{};
    decimal_t high_price{};
    decimal_t low_price{};
    decimal_t base_volume{};
    std::uint64_t trade_count{};
    bool is_closed{};
    decimal_t quote_volume{};
    decimal_t taker_buy_base_vol{};
    decimal_t taker_buy_quote_vol{};
    decimal_t ignore{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
struct kline_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    kline_stream_data_t kline_t{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
struct continuous_contract_kline_stream_data_t
{
    timestamp_ms_t open_time{};
    timestamp_ms_t close_time{};
    kline_interval_t interval{};
    std::uint64_t first_update_id{};
    std::uint64_t last_update_id{};
    decimal_t open_price{};
    decimal_t close_price{};
    decimal_t high_price{};
    decimal_t low_price{};
    decimal_t volume{};
    std::uint64_t trade_count{};
    bool is_closed{};
    decimal_t quote_volume{};
    decimal_t taker_buy_volume{};
    decimal_t taker_buy_quote_vol{};
    decimal_t ignore{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
struct continuous_contract_kline_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    pair_t pair{};
    contract_type_t contractType{};
    continuous_contract_kline_stream_data_t kline_t{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_constituent_t
{
    std::string base_asset{};
    std::string quote_asset{};
    decimal_t weight_in_quantity{};
    decimal_t weight_in_pct{};
    decimal_t index_price{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t price{};
    std::optional<std::string> base_asset_type{};
    std::vector<composite_index_constituent_t> composition{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
struct contract_info_bracket_t
{
    int notional_bracket{};
    double bracket_floor{};
    double bracket_cap{};
    double maint_margin_ratio{};
    double calc_factor{};
    int min_leverage{};
    int max_leverage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
struct contract_info_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    pair_t pair{};
    contract_type_t contractType{};
    timestamp_ms_t delivery_time{};
    timestamp_ms_t onboard_time{};
    contract_status_t contractStatus{};
    std::optional<std::vector<contract_info_bracket_t>> brackets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
struct asset_index_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t index_price{};
    decimal_t bid_buffer{};
    decimal_t ask_buffer{};
    decimal_t bid_rate{};
    decimal_t ask_rate{};
    decimal_t auto_exch_bid_buffer{};
    decimal_t auto_exch_ask_buffer{};
    decimal_t auto_exch_bid_rate{};
    decimal_t auto_exch_ask_rate{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
using all_asset_index_stream_event = std::vector<asset_index_stream_event_t>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Trading-Session-Stream.md
struct trading_session_stream_event_t
{
    market_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t session_start_time{};
    timestamp_ms_t session_end_time{};
    session_type_t session_type{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::book_ticker_stream_event_t>
{
    using T = binapi2::fapi::types::book_ticker_stream_event_t;
    static constexpr auto value =
        object("e", &T::event_type, "u", &T::update_id, "s", &T::symbol, "b", &T::best_bid_price, "B", &T::best_bid_qty, "a", &T::best_ask_price, "A", &T::best_ask_qty, "T", &T::transaction_time, "E", &T::event_time);
};

template<>
struct glz::meta<binapi2::fapi::types::aggregate_trade_stream_event_t>
{
    using T = binapi2::fapi::types::aggregate_trade_stream_event_t;
    static constexpr auto value = object("e",
                                         &T::event_type,
                                         "E",
                                         &T::event_time,
                                         "s",
                                         &T::symbol,
                                         "a",
                                         &T::agg_trade_id,
                                         "p",
                                         &T::price,
                                         "q",
                                         &T::quantity,
                                         "f",
                                         &T::first_trade_id,
                                         "l",
                                         &T::last_trade_id,
                                         "T",
                                         &T::trade_time,
                                         "m",
                                         &T::is_buyer_maker,
                                         "nq",
                                         &T::normal_quantity);
};

template<>
struct glz::meta<binapi2::fapi::types::mark_price_stream_event_t>
{
    using T = binapi2::fapi::types::mark_price_stream_event_t;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "p", &T::mark_price_t, "ap", &T::mark_price_avg, "i", &T::index_price, "P", &T::settle_price, "r", &T::funding_rate, "T", &T::next_funding_time);
};

template<>
struct glz::meta<binapi2::fapi::types::depth_stream_event_t>
{
    using T = binapi2::fapi::types::depth_stream_event_t;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "s", &T::symbol, "U", &T::first_update_id, "u", &T::final_update_id, "pu", &T::prev_final_update_id, "b", &T::bids, "a", &T::asks);
};

template<>
struct glz::meta<binapi2::fapi::types::mini_ticker_stream_event_t>
{
    using T = binapi2::fapi::types::mini_ticker_stream_event_t;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "c", &T::close_price, "o", &T::open_price, "h", &T::high_price, "l", &T::low_price, "v", &T::base_volume, "q", &T::quote_volume);
};

template<>
struct glz::meta<binapi2::fapi::types::ticker_stream_event_t>
{
    using T = binapi2::fapi::types::ticker_stream_event_t;
    static constexpr auto value = object("e",
                                         &T::event_type,
                                         "E",
                                         &T::event_time,
                                         "s",
                                         &T::symbol,
                                         "p",
                                         &T::price_change,
                                         "P",
                                         &T::price_change_pct,
                                         "w",
                                         &T::weighted_avg_price,
                                         "c",
                                         &T::last_price,
                                         "Q",
                                         &T::last_quantity,
                                         "o",
                                         &T::open_price,
                                         "h",
                                         &T::high_price,
                                         "l",
                                         &T::low_price,
                                         "v",
                                         &T::base_volume,
                                         "q",
                                         &T::quote_volume,
                                         "O",
                                         &T::stats_open_time,
                                         "C",
                                         &T::stats_close_time,
                                         "F",
                                         &T::first_trade_id,
                                         "L",
                                         &T::last_trade_id,
                                         "n",
                                         &T::trade_count);
};

template<>
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_data_t>
{
    using T = binapi2::fapi::types::liquidation_order_stream_data_t;
    static constexpr auto value = object("s",
                                         &T::symbol,
                                         "S",
                                         &T::side,
                                         "o",
                                         &T::type,
                                         "f",
                                         &T::tif,
                                         "q",
                                         &T::original_quantity,
                                         "p",
                                         &T::price,
                                         "ap",
                                         &T::average_price,
                                         "X",
                                         &T::status,
                                         "l",
                                         &T::last_filled_qty,
                                         "z",
                                         &T::filled_accum_qty,
                                         "T",
                                         &T::trade_time);
};

template<>
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_event_t>
{
    using T = binapi2::fapi::types::liquidation_order_stream_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_stream_data_t>
{
    using T = binapi2::fapi::types::kline_stream_data_t;
    static constexpr auto value = object("t",
                                         &T::open_time,
                                         "T",
                                         &T::close_time,
                                         "s",
                                         &T::symbol,
                                         "i",
                                         &T::interval,
                                         "f",
                                         &T::first_trade_id,
                                         "L",
                                         &T::last_trade_id,
                                         "o",
                                         &T::open_price,
                                         "c",
                                         &T::close_price,
                                         "h",
                                         &T::high_price,
                                         "l",
                                         &T::low_price,
                                         "v",
                                         &T::base_volume,
                                         "n",
                                         &T::trade_count,
                                         "x",
                                         &T::is_closed,
                                         "q",
                                         &T::quote_volume,
                                         "V",
                                         &T::taker_buy_base_vol,
                                         "Q",
                                         &T::taker_buy_quote_vol,
                                         "B",
                                         &T::ignore);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_stream_event_t>
{
    using T = binapi2::fapi::types::kline_stream_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "k", &T::kline_t);
};

template<>
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_data_t>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_data_t;
    static constexpr auto value = object("t",
                                         &T::open_time,
                                         "T",
                                         &T::close_time,
                                         "i",
                                         &T::interval,
                                         "f",
                                         &T::first_update_id,
                                         "L",
                                         &T::last_update_id,
                                         "o",
                                         &T::open_price,
                                         "c",
                                         &T::close_price,
                                         "h",
                                         &T::high_price,
                                         "l",
                                         &T::low_price,
                                         "v",
                                         &T::volume,
                                         "n",
                                         &T::trade_count,
                                         "x",
                                         &T::is_closed,
                                         "q",
                                         &T::quote_volume,
                                         "V",
                                         &T::taker_buy_volume,
                                         "Q",
                                         &T::taker_buy_quote_vol,
                                         "B",
                                         &T::ignore);
};

template<>
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_event_t>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "ps", &T::pair, "ct", &T::contractType, "k", &T::kline_t);
};


template<>
struct glz::meta<binapi2::fapi::types::composite_index_constituent_t>
{
    using T = binapi2::fapi::types::composite_index_constituent_t;
    static constexpr auto value = object("b", &T::base_asset, "q", &T::quote_asset, "w", &T::weight_in_quantity, "W", &T::weight_in_pct, "i", &T::index_price);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_stream_event_t>
{
    using T = binapi2::fapi::types::composite_index_stream_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "p", &T::price, "C", &T::base_asset_type, "c", &T::composition);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_bracket_t>
{
    using T = binapi2::fapi::types::contract_info_bracket_t;
    static constexpr auto value =
        object("bs", &T::notional_bracket, "bnf", &T::bracket_floor, "bnc", &T::bracket_cap, "mmr", &T::maint_margin_ratio, "cf", &T::calc_factor, "mi", &T::min_leverage, "ma", &T::max_leverage);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_stream_event_t>
{
    using T = binapi2::fapi::types::contract_info_stream_event_t;
    static constexpr auto value = object("e",
                                         &T::event_type,
                                         "E",
                                         &T::event_time,
                                         "s",
                                         &T::symbol,
                                         "ps",
                                         &T::pair,
                                         "ct",
                                         &T::contractType,
                                         "dt",
                                         &T::delivery_time,
                                         "ot",
                                         &T::onboard_time,
                                         "cs",
                                         &T::contractStatus,
                                         "bks",
                                         &T::brackets);
};

template<>
struct glz::meta<binapi2::fapi::types::asset_index_stream_event_t>
{
    using T = binapi2::fapi::types::asset_index_stream_event_t;
    static constexpr auto value = object("e",
                                         &T::event_type,
                                         "E",
                                         &T::event_time,
                                         "s",
                                         &T::symbol,
                                         "i",
                                         &T::index_price,
                                         "b",
                                         &T::bid_buffer,
                                         "a",
                                         &T::ask_buffer,
                                         "B",
                                         &T::bid_rate,
                                         "A",
                                         &T::ask_rate,
                                         "q",
                                         &T::auto_exch_bid_buffer,
                                         "g",
                                         &T::auto_exch_ask_buffer,
                                         "Q",
                                         &T::auto_exch_bid_rate,
                                         "G",
                                         &T::auto_exch_ask_rate);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_session_stream_event_t>
{
    using T = binapi2::fapi::types::trading_session_stream_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "t", &T::session_start_time, "T", &T::session_end_time, "S", &T::session_type);
};
