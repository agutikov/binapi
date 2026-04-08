// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Book-Ticker-Streams.md
struct book_ticker_stream_event
{
    std::string event_type{};
    std::uint64_t update_id{};
    std::string symbol{};
    decimal_t best_bid_price{};
    decimal_t best_bid_qty{};
    decimal_t best_ask_price{};
    decimal_t best_ask_qty{};
    std::uint64_t transaction_time{};
    std::uint64_t event_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Aggregate-Trade-Streams.md
struct aggregate_trade_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    std::uint64_t agg_trade_id{};
    decimal_t price{};
    decimal_t quantity{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    std::uint64_t trade_time{};
    bool is_buyer_maker{};
    std::optional<decimal_t> normal_quantity{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream.md
struct mark_price_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    decimal_t mark_price{};
    std::optional<decimal_t> mark_price_avg{};
    decimal_t index_price{};
    decimal_t settle_price{};
    decimal_t funding_rate{};
    std::uint64_t next_funding_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream-for-All-market.md
using all_market_mark_price_stream_event = std::vector<mark_price_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams.md
struct depth_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    std::string symbol{};
    std::uint64_t first_update_id{};
    std::uint64_t final_update_id{};
    std::uint64_t prev_final_update_id{};
    std::vector<price_level> bids{};
    std::vector<price_level> asks{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
struct mini_ticker_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    decimal_t close_price{};
    decimal_t open_price{};
    decimal_t high_price{};
    decimal_t low_price{};
    decimal_t base_volume{};
    decimal_t quote_volume{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
using all_market_mini_ticker_stream_event = std::vector<mini_ticker_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
struct ticker_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
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
    std::uint64_t stats_open_time{};
    std::uint64_t stats_close_time{};
    std::uint64_t first_trade_id{};
    std::uint64_t last_trade_id{};
    std::uint64_t trade_count{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
using all_market_ticker_stream_event = std::vector<ticker_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_data
{
    std::string symbol{};
    order_side side{};
    order_type type{};
    time_in_force tif{};
    decimal_t original_quantity{};
    decimal_t price{};
    decimal_t average_price{};
    order_status status{};
    decimal_t last_filled_qty{};
    decimal_t filled_accum_qty{};
    std::uint64_t trade_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    liquidation_order_stream_data order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
struct kline_stream_data
{
    std::uint64_t open_time{};
    std::uint64_t close_time{};
    std::string symbol{};
    kline_interval interval{};
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
struct kline_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    kline_stream_data kline{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
struct continuous_contract_kline_stream_data
{
    std::uint64_t open_time{};
    std::uint64_t close_time{};
    kline_interval interval{};
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
struct continuous_contract_kline_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string pair{};
    contract_type contractType{};
    continuous_contract_kline_stream_data kline{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_constituent
{
    std::string base_asset{};
    std::string quote_asset{};
    decimal_t weight_in_quantity{};
    decimal_t weight_in_pct{};
    decimal_t index_price{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    decimal_t price{};
    std::optional<std::string> base_asset_type{};
    std::vector<composite_index_constituent> composition{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
struct contract_info_bracket
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
struct contract_info_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
    std::string pair{};
    contract_type contractType{};
    std::uint64_t delivery_time{};
    std::uint64_t onboard_time{};
    contract_status contractStatus{};
    std::optional<std::vector<contract_info_bracket>> brackets{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
struct asset_index_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::string symbol{};
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
using all_asset_index_stream_event = std::vector<asset_index_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Trading-Session-Stream.md
struct trading_session_stream_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t session_start_time{};
    std::uint64_t session_end_time{};
    std::string session_type{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_balance
{
    std::string asset{};
    decimal_t wallet_balance{};
    decimal_t cross_wallet_balance{};
    decimal_t balance_change{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_position
{
    std::string symbol{};
    decimal_t position_amount{};
    decimal_t entry_price{};
    decimal_t accum_realized{};
    decimal_t unrealized_pnl{};
    margin_type margin{};
    decimal_t isolated_wallet{};
    position_side pos_side{};
    decimal_t breakeven_price{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_data
{
    std::string reason_type{};
    std::vector<account_update_balance> balances{};
    std::vector<account_update_position> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    account_update_data update_data{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_order
{
    std::string symbol{};
    std::string client_order_id{};
    order_side side{};
    order_type type{};
    time_in_force tif{};
    decimal_t original_quantity{};
    decimal_t original_price{};
    decimal_t average_price{};
    decimal_t stop_price{};
    execution_type exec_type{};
    order_status status{};
    std::uint64_t order_id{};
    decimal_t last_filled_qty{};
    decimal_t filled_accum_qty{};
    decimal_t last_filled_price{};
    std::string commission_asset{};
    decimal_t commission{};
    std::uint64_t trade_time{};
    std::uint64_t trade_id{};
    decimal_t bids_notional{};
    decimal_t ask_notional{};
    bool is_maker{};
    bool is_reduce_only{};
    working_type work_type{};
    order_type orig_order_type{};
    position_side pos_side{};
    bool is_close_all{};
    std::optional<decimal_t> activation_price{};
    std::optional<decimal_t> callback_rate{};
    bool price_protection{};
    int ignore_si{};
    int ignore_ss{};
    decimal_t realized_profit{};
    stp_mode stp{};
    price_match price_match_mode{};
    std::optional<std::uint64_t> gtd_auto_cancel{};
    std::optional<std::string> expiry_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    order_trade_update_order order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_position
{
    std::string symbol{};
    position_side pos_side{};
    decimal_t position_amount{};
    margin_type margin{};
    decimal_t isolated_wallet{};
    decimal_t mark_price{};
    decimal_t unrealized_pnl{};
    decimal_t maint_margin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    decimal_t cross_wallet_balance{};
    std::vector<margin_call_position> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-User-Data-Stream-Expired.md
struct listen_key_expired_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    std::string listen_key{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_leverage
{
    std::string symbol{};
    int leverage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_multi_assets
{
    bool multi_assets_mode{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_update_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    std::optional<account_config_leverage> leverage_config{};
    std::optional<account_config_multi_assets> multi_assets_config{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md
struct trade_lite_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t transaction_time{};
    std::string symbol{};
    decimal_t original_quantity{};
    decimal_t original_price{};
    bool is_maker{};
    std::string client_order_id{};
    order_side side{};
    decimal_t last_filled_price{};
    decimal_t last_filled_qty{};
    std::uint64_t trade_id{};
    std::uint64_t order_id{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_data
{
    std::string client_algo_id{};
    std::uint64_t algo_id{};
    algo_type alg_type{};
    order_type type{};
    std::string symbol{};
    order_side side{};
    position_side pos_side{};
    time_in_force tif{};
    decimal_t quantity{};
    algo_status alg_status{};
    std::optional<std::string> matched_order_id{};
    std::optional<decimal_t> avg_fill_price{};
    std::optional<decimal_t> executed_quantity{};
    std::optional<std::string> actual_order_type{};
    std::optional<decimal_t> trigger_price{};
    std::optional<decimal_t> order_price{};
    std::optional<stp_mode> stp{};
    std::optional<working_type> work_type{};
    std::optional<price_match> price_match_mode{};
    std::optional<bool> is_close_all{};
    std::optional<bool> price_protection{};
    std::optional<bool> is_reduce_only{};
    std::optional<std::uint64_t> trigger_time{};
    std::optional<std::uint64_t> gtd_cancel_time{};
    std::optional<std::string> reject_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_event
{
    std::string event_type{};
    std::uint64_t transaction_time{};
    std::uint64_t event_time{};
    algo_order_update_data order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_reject_data
{
    std::string symbol{};
    std::uint64_t order_id{};
    std::string reject_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_trigger_reject_event
{
    std::string event_type{};
    std::uint64_t event_time{};
    std::uint64_t message_send_time{};
    conditional_order_reject_data order_reject{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_data
{
    std::uint64_t strategy_id{};
    std::string strategy_type{};
    std::string strategy_status{};
    std::string symbol{};
    decimal_t realized_pnl{};
    decimal_t unmatched_avg_price{};
    decimal_t unmatched_qty{};
    decimal_t unmatched_fee{};
    decimal_t matched_pnl{};
    std::uint64_t update_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_event
{
    std::string event_type{};
    std::uint64_t transaction_time{};
    std::uint64_t event_time{};
    grid_update_data grid_update{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_data
{
    std::uint64_t strategy_id{};
    std::string strategy_type{};
    std::string strategy_status{};
    std::string symbol{};
    std::uint64_t update_time{};
    int op_code{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_event
{
    std::string event_type{};
    std::uint64_t transaction_time{};
    std::uint64_t event_time{};
    strategy_update_data strategy_update{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::book_ticker_stream_event>
{
    using T = binapi2::fapi::types::book_ticker_stream_event;
    static constexpr auto value =
        object("e", &T::event_type, "u", &T::update_id, "s", &T::symbol, "b", &T::best_bid_price, "B", &T::best_bid_qty, "a", &T::best_ask_price, "A", &T::best_ask_qty, "T", &T::transaction_time, "E", &T::event_time);
};

template<>
struct glz::meta<binapi2::fapi::types::aggregate_trade_stream_event>
{
    using T = binapi2::fapi::types::aggregate_trade_stream_event;
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
struct glz::meta<binapi2::fapi::types::mark_price_stream_event>
{
    using T = binapi2::fapi::types::mark_price_stream_event;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "p", &T::mark_price, "ap", &T::mark_price_avg, "i", &T::index_price, "P", &T::settle_price, "r", &T::funding_rate, "T", &T::next_funding_time);
};

template<>
struct glz::meta<binapi2::fapi::types::depth_stream_event>
{
    using T = binapi2::fapi::types::depth_stream_event;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "s", &T::symbol, "U", &T::first_update_id, "u", &T::final_update_id, "pu", &T::prev_final_update_id, "b", &T::bids, "a", &T::asks);
};

template<>
struct glz::meta<binapi2::fapi::types::mini_ticker_stream_event>
{
    using T = binapi2::fapi::types::mini_ticker_stream_event;
    static constexpr auto value =
        object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "c", &T::close_price, "o", &T::open_price, "h", &T::high_price, "l", &T::low_price, "v", &T::base_volume, "q", &T::quote_volume);
};

template<>
struct glz::meta<binapi2::fapi::types::ticker_stream_event>
{
    using T = binapi2::fapi::types::ticker_stream_event;
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
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_data>
{
    using T = binapi2::fapi::types::liquidation_order_stream_data;
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
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_event>
{
    using T = binapi2::fapi::types::liquidation_order_stream_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_stream_data>
{
    using T = binapi2::fapi::types::kline_stream_data;
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
struct glz::meta<binapi2::fapi::types::kline_stream_event>
{
    using T = binapi2::fapi::types::kline_stream_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "k", &T::kline);
};

template<>
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_data>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_data;
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
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_event>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "ps", &T::pair, "ct", &T::contractType, "k", &T::kline);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_balance>
{
    using T = binapi2::fapi::types::account_update_balance;
    static constexpr auto value = object("a", &T::asset, "wb", &T::wallet_balance, "cw", &T::cross_wallet_balance, "bc", &T::balance_change);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_position>
{
    using T = binapi2::fapi::types::account_update_position;
    static constexpr auto value =
        object("s", &T::symbol, "pa", &T::position_amount, "ep", &T::entry_price, "cr", &T::accum_realized, "up", &T::unrealized_pnl, "mt", &T::margin, "iw", &T::isolated_wallet, "ps", &T::pos_side, "bep", &T::breakeven_price);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_data>
{
    using T = binapi2::fapi::types::account_update_data;
    static constexpr auto value = object("m", &T::reason_type, "B", &T::balances, "P", &T::positions);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_event>
{
    using T = binapi2::fapi::types::account_update_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "a", &T::update_data);
};

template<>
struct glz::meta<binapi2::fapi::types::order_trade_update_order>
{
    using T = binapi2::fapi::types::order_trade_update_order;
    static constexpr auto value = object("s", &T::symbol, "c", &T::client_order_id, "S", &T::side, "o", &T::type, "f", &T::tif,
                                         "q", &T::original_quantity, "p", &T::original_price, "ap", &T::average_price, "sp", &T::stop_price, "x", &T::exec_type,
                                         "X", &T::status, "i", &T::order_id, "l", &T::last_filled_qty, "z", &T::filled_accum_qty, "L", &T::last_filled_price,
                                         "N", &T::commission_asset, "n", &T::commission, "T", &T::trade_time, "t", &T::trade_id, "b", &T::bids_notional,
                                         "a", &T::ask_notional, "m", &T::is_maker, "R", &T::is_reduce_only, "wt", &T::work_type, "ot", &T::orig_order_type,
                                         "ps", &T::pos_side, "cp", &T::is_close_all, "AP", &T::activation_price, "cr", &T::callback_rate, "pP", &T::price_protection,
                                         "si", &T::ignore_si, "ss", &T::ignore_ss, "rp", &T::realized_profit, "V", &T::stp, "pm", &T::price_match_mode,
                                         "gtd", &T::gtd_auto_cancel, "er", &T::expiry_reason);
};

template<>
struct glz::meta<binapi2::fapi::types::order_trade_update_event>
{
    using T = binapi2::fapi::types::order_trade_update_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_position>
{
    using T = binapi2::fapi::types::margin_call_position;
    static constexpr auto value =
        object("s", &T::symbol, "ps", &T::pos_side, "pa", &T::position_amount, "mt", &T::margin, "iw", &T::isolated_wallet, "mp", &T::mark_price, "up", &T::unrealized_pnl, "mm", &T::maint_margin);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_event>
{
    using T = binapi2::fapi::types::margin_call_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "cw", &T::cross_wallet_balance, "p", &T::positions);
};

template<>
struct glz::meta<binapi2::fapi::types::listen_key_expired_event>
{
    using T = binapi2::fapi::types::listen_key_expired_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "listenKey", &T::listen_key);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_constituent>
{
    using T = binapi2::fapi::types::composite_index_constituent;
    static constexpr auto value = object("b", &T::base_asset, "q", &T::quote_asset, "w", &T::weight_in_quantity, "W", &T::weight_in_pct, "i", &T::index_price);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_stream_event>
{
    using T = binapi2::fapi::types::composite_index_stream_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "s", &T::symbol, "p", &T::price, "C", &T::base_asset_type, "c", &T::composition);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_bracket>
{
    using T = binapi2::fapi::types::contract_info_bracket;
    static constexpr auto value =
        object("bs", &T::notional_bracket, "bnf", &T::bracket_floor, "bnc", &T::bracket_cap, "mmr", &T::maint_margin_ratio, "cf", &T::calc_factor, "mi", &T::min_leverage, "ma", &T::max_leverage);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_stream_event>
{
    using T = binapi2::fapi::types::contract_info_stream_event;
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
struct glz::meta<binapi2::fapi::types::asset_index_stream_event>
{
    using T = binapi2::fapi::types::asset_index_stream_event;
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
struct glz::meta<binapi2::fapi::types::trading_session_stream_event>
{
    using T = binapi2::fapi::types::trading_session_stream_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "t", &T::session_start_time, "T", &T::session_end_time, "S", &T::session_type);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_leverage>
{
    using T = binapi2::fapi::types::account_config_leverage;
    static constexpr auto value = object("s", &T::symbol, "l", &T::leverage);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_multi_assets>
{
    using T = binapi2::fapi::types::account_config_multi_assets;
    static constexpr auto value = object("j", &T::multi_assets_mode);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_update_event>
{
    using T = binapi2::fapi::types::account_config_update_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "ac", &T::leverage_config, "ai", &T::multi_assets_config);
};

template<>
struct glz::meta<binapi2::fapi::types::trade_lite_event>
{
    using T = binapi2::fapi::types::trade_lite_event;
    static constexpr auto value = object("e",
                                         &T::event_type,
                                         "E",
                                         &T::event_time,
                                         "T",
                                         &T::transaction_time,
                                         "s",
                                         &T::symbol,
                                         "q",
                                         &T::original_quantity,
                                         "p",
                                         &T::original_price,
                                         "m",
                                         &T::is_maker,
                                         "c",
                                         &T::client_order_id,
                                         "S",
                                         &T::side,
                                         "L",
                                         &T::last_filled_price,
                                         "l",
                                         &T::last_filled_qty,
                                         "t",
                                         &T::trade_id,
                                         "i",
                                         &T::order_id);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_order_update_data>
{
    using T = binapi2::fapi::types::algo_order_update_data;
    static constexpr auto value = object("caid",
                                         &T::client_algo_id,
                                         "aid",
                                         &T::algo_id,
                                         "at",
                                         &T::alg_type,
                                         "o",
                                         &T::type,
                                         "s",
                                         &T::symbol,
                                         "S",
                                         &T::side,
                                         "ps",
                                         &T::pos_side,
                                         "f",
                                         &T::tif,
                                         "q",
                                         &T::quantity,
                                         "X",
                                         &T::alg_status,
                                         "ai",
                                         &T::matched_order_id,
                                         "ap",
                                         &T::avg_fill_price,
                                         "aq",
                                         &T::executed_quantity,
                                         "act",
                                         &T::actual_order_type,
                                         "tp",
                                         &T::trigger_price,
                                         "p",
                                         &T::order_price,
                                         "V",
                                         &T::stp,
                                         "wt",
                                         &T::work_type,
                                         "pm",
                                         &T::price_match_mode,
                                         "cp",
                                         &T::is_close_all,
                                         "pP",
                                         &T::price_protection,
                                         "R",
                                         &T::is_reduce_only,
                                         "tt",
                                         &T::trigger_time,
                                         "gtd",
                                         &T::gtd_cancel_time,
                                         "rm",
                                         &T::reject_reason);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_order_update_event>
{
    using T = binapi2::fapi::types::algo_order_update_event;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_reject_data>
{
    using T = binapi2::fapi::types::conditional_order_reject_data;
    static constexpr auto value = object("s", &T::symbol, "i", &T::order_id, "r", &T::reject_reason);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_trigger_reject_event>
{
    using T = binapi2::fapi::types::conditional_order_trigger_reject_event;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::message_send_time, "or", &T::order_reject);
};

template<>
struct glz::meta<binapi2::fapi::types::grid_update_data>
{
    using T = binapi2::fapi::types::grid_update_data;
    static constexpr auto value = object("si",
                                         &T::strategy_id,
                                         "st",
                                         &T::strategy_type,
                                         "ss",
                                         &T::strategy_status,
                                         "s",
                                         &T::symbol,
                                         "r",
                                         &T::realized_pnl,
                                         "up",
                                         &T::unmatched_avg_price,
                                         "uq",
                                         &T::unmatched_qty,
                                         "uf",
                                         &T::unmatched_fee,
                                         "mp",
                                         &T::matched_pnl,
                                         "ut",
                                         &T::update_time);
};

template<>
struct glz::meta<binapi2::fapi::types::grid_update_event>
{
    using T = binapi2::fapi::types::grid_update_event;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "gu", &T::grid_update);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_data>
{
    using T = binapi2::fapi::types::strategy_update_data;
    static constexpr auto value = object("si", &T::strategy_id, "st", &T::strategy_type, "ss", &T::strategy_status, "s", &T::symbol, "ut", &T::update_time, "c", &T::op_code);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_event>
{
    using T = binapi2::fapi::types::strategy_update_event;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "su", &T::strategy_update);
};
