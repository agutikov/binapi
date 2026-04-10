// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file user_stream_events.hpp
/// @brief Event types for private user data WebSocket streams.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace binapi2::fapi::types {

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_balance_t
{
    std::string asset{};
    decimal_t wallet_balance{};
    decimal_t cross_wallet_balance{};
    decimal_t balance_change{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_position_t
{
    symbol_t symbol{};
    decimal_t position_amount{};
    decimal_t entry_price{};
    decimal_t accum_realized{};
    decimal_t unrealized_pnl{};
    margin_type_t margin{};
    decimal_t isolated_wallet{};
    position_side_t pos_side{};
    decimal_t breakeven_price{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_data_t
{
    reason_type_t reason_type{};
    std::vector<account_update_balance_t> balances{};
    std::vector<account_update_position_t> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    account_update_data_t update_data{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_order_t
{
    symbol_t symbol{};
    std::string client_order_id{};
    order_side_t side{};
    order_type_t type{};
    time_in_force_t tif{};
    decimal_t original_quantity{};
    decimal_t original_price{};
    decimal_t average_price{};
    decimal_t stop_price{};
    execution_type_t exec_type{};
    order_status_t status{};
    std::uint64_t order_id{};
    decimal_t last_filled_qty{};
    decimal_t filled_accum_qty{};
    decimal_t last_filled_price{};
    std::string commission_asset{};
    decimal_t commission{};
    timestamp_ms_t trade_time{};
    std::uint64_t trade_id{};
    decimal_t bids_notional{};
    decimal_t ask_notional{};
    bool is_maker{};
    bool is_reduce_only{};
    working_type_t work_type{};
    order_type_t orig_order_type{};
    position_side_t pos_side{};
    bool is_close_all{};
    std::optional<decimal_t> activation_price{};
    std::optional<decimal_t> callback_rate{};
    bool price_protection{};
    int ignore_si{};
    int ignore_ss{};
    decimal_t realized_profit{};
    stp_mode_t stp{};
    price_match_t price_match_mode{};
    std::optional<timestamp_ms_t> gtd_auto_cancel{};
    std::optional<std::string> expiry_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    order_trade_update_order_t order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_position_t
{
    symbol_t symbol{};
    position_side_t pos_side{};
    decimal_t position_amount{};
    margin_type_t margin{};
    decimal_t isolated_wallet{};
    decimal_t mark_price_t{};
    decimal_t unrealized_pnl{};
    decimal_t maint_margin{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    decimal_t cross_wallet_balance{};
    std::vector<margin_call_position_t> positions{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-User-Data-Stream-Expired.md
struct listen_key_expired_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    std::string listen_key{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_leverage_t
{
    symbol_t symbol{};
    int leverage{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_multi_assets_t
{
    bool multi_assets_mode{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    std::optional<account_config_leverage_t> leverage_config{};
    std::optional<account_config_multi_assets_t> multi_assets_config{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md
struct trade_lite_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t transaction_time{};
    symbol_t symbol{};
    decimal_t original_quantity{};
    decimal_t original_price{};
    bool is_maker{};
    std::string client_order_id{};
    order_side_t side{};
    decimal_t last_filled_price{};
    decimal_t last_filled_qty{};
    std::uint64_t trade_id{};
    std::uint64_t order_id{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_data_t
{
    std::string client_algo_id{};
    std::uint64_t algo_id{};
    algo_type_t alg_type{};
    order_type_t type{};
    symbol_t symbol{};
    order_side_t side{};
    position_side_t pos_side{};
    time_in_force_t tif{};
    decimal_t quantity{};
    algo_status_t alg_status{};
    std::optional<std::string> matched_order_id{};
    std::optional<decimal_t> avg_fill_price{};
    std::optional<decimal_t> executed_quantity{};
    std::optional<order_type_t> actual_order_type{};
    std::optional<decimal_t> trigger_price{};
    std::optional<decimal_t> order_price{};
    std::optional<stp_mode_t> stp{};
    std::optional<working_type_t> work_type{};
    std::optional<price_match_t> price_match_mode{};
    std::optional<bool> is_close_all{};
    std::optional<bool> price_protection{};
    std::optional<bool> is_reduce_only{};
    std::optional<timestamp_ms_t> trigger_time{};
    std::optional<timestamp_ms_t> gtd_cancel_time{};
    std::optional<std::string> reject_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t transaction_time{};
    timestamp_ms_t event_time{};
    algo_order_update_data_t order{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_reject_data_t
{
    symbol_t symbol{};
    std::uint64_t order_id{};
    std::string reject_reason{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_trigger_reject_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t event_time{};
    timestamp_ms_t message_send_time{};
    conditional_order_reject_data_t order_reject{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_data_t
{
    std::uint64_t strategy_id{};
    strategy_type_t strategy_type{};
    strategy_status_t strategy_status{};
    symbol_t symbol{};
    decimal_t realized_pnl{};
    decimal_t unmatched_avg_price{};
    decimal_t unmatched_qty{};
    decimal_t unmatched_fee{};
    decimal_t matched_pnl{};
    timestamp_ms_t update_time{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t transaction_time{};
    timestamp_ms_t event_time{};
    grid_update_data_t grid_update{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_data_t
{
    std::uint64_t strategy_id{};
    strategy_type_t strategy_type{};
    strategy_status_t strategy_status{};
    symbol_t symbol{};
    timestamp_ms_t update_time{};
    int op_code{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_event_t
{
    user_event_type_t event_type{};
    timestamp_ms_t transaction_time{};
    timestamp_ms_t event_time{};
    strategy_update_data_t strategy_update{};
};


/// @brief Variant of all user data stream event types.
using user_stream_event_t = std::variant<
    account_update_event_t,
    order_trade_update_event_t,
    margin_call_event_t,
    listen_key_expired_event_t,
    account_config_update_event_t,
    trade_lite_event_t,
    algo_order_update_event_t,
    conditional_order_trigger_reject_event_t,
    grid_update_event_t,
    strategy_update_event_t
>;

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::account_update_balance_t>
{
    using T = binapi2::fapi::types::account_update_balance_t;
    static constexpr auto value = object("a", &T::asset, "wb", &T::wallet_balance, "cw", &T::cross_wallet_balance, "bc", &T::balance_change);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_position_t>
{
    using T = binapi2::fapi::types::account_update_position_t;
    static constexpr auto value =
        object("s", &T::symbol, "pa", &T::position_amount, "ep", &T::entry_price, "cr", &T::accum_realized, "up", &T::unrealized_pnl, "mt", &T::margin, "iw", &T::isolated_wallet, "ps", &T::pos_side, "bep", &T::breakeven_price);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_data_t>
{
    using T = binapi2::fapi::types::account_update_data_t;
    static constexpr auto value = object("m", &T::reason_type, "B", &T::balances, "P", &T::positions);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_event_t>
{
    using T = binapi2::fapi::types::account_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "a", &T::update_data);
};

template<>
struct glz::meta<binapi2::fapi::types::order_trade_update_order_t>
{
    using T = binapi2::fapi::types::order_trade_update_order_t;
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
struct glz::meta<binapi2::fapi::types::order_trade_update_event_t>
{
    using T = binapi2::fapi::types::order_trade_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_position_t>
{
    using T = binapi2::fapi::types::margin_call_position_t;
    static constexpr auto value =
        object("s", &T::symbol, "ps", &T::pos_side, "pa", &T::position_amount, "mt", &T::margin, "iw", &T::isolated_wallet, "mp", &T::mark_price_t, "up", &T::unrealized_pnl, "mm", &T::maint_margin);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_event_t>
{
    using T = binapi2::fapi::types::margin_call_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "cw", &T::cross_wallet_balance, "p", &T::positions);
};

template<>
struct glz::meta<binapi2::fapi::types::listen_key_expired_event_t>
{
    using T = binapi2::fapi::types::listen_key_expired_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "listenKey", &T::listen_key);
};


template<>
struct glz::meta<binapi2::fapi::types::account_config_leverage_t>
{
    using T = binapi2::fapi::types::account_config_leverage_t;
    static constexpr auto value = object("s", &T::symbol, "l", &T::leverage);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_multi_assets_t>
{
    using T = binapi2::fapi::types::account_config_multi_assets_t;
    static constexpr auto value = object("j", &T::multi_assets_mode);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_update_event_t>
{
    using T = binapi2::fapi::types::account_config_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::transaction_time, "ac", &T::leverage_config, "ai", &T::multi_assets_config);
};

template<>
struct glz::meta<binapi2::fapi::types::trade_lite_event_t>
{
    using T = binapi2::fapi::types::trade_lite_event_t;
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
struct glz::meta<binapi2::fapi::types::algo_order_update_data_t>
{
    using T = binapi2::fapi::types::algo_order_update_data_t;
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
struct glz::meta<binapi2::fapi::types::algo_order_update_event_t>
{
    using T = binapi2::fapi::types::algo_order_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "o", &T::order);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_reject_data_t>
{
    using T = binapi2::fapi::types::conditional_order_reject_data_t;
    static constexpr auto value = object("s", &T::symbol, "i", &T::order_id, "r", &T::reject_reason);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_trigger_reject_event_t>
{
    using T = binapi2::fapi::types::conditional_order_trigger_reject_event_t;
    static constexpr auto value = object("e", &T::event_type, "E", &T::event_time, "T", &T::message_send_time, "or", &T::order_reject);
};

template<>
struct glz::meta<binapi2::fapi::types::grid_update_data_t>
{
    using T = binapi2::fapi::types::grid_update_data_t;
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
struct glz::meta<binapi2::fapi::types::grid_update_event_t>
{
    using T = binapi2::fapi::types::grid_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "gu", &T::grid_update);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_data_t>
{
    using T = binapi2::fapi::types::strategy_update_data_t;
    static constexpr auto value = object("si", &T::strategy_id, "st", &T::strategy_type, "ss", &T::strategy_status, "s", &T::symbol, "ut", &T::update_time, "c", &T::op_code);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_event_t>
{
    using T = binapi2::fapi::types::strategy_update_event_t;
    static constexpr auto value = object("e", &T::event_type, "T", &T::transaction_time, "E", &T::event_time, "su", &T::strategy_update);
};
