// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/common.hpp>
#include <binapi2/fapi/types/enums.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace binapi2::fapi::types {

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Book-Ticker-Streams.md
struct book_ticker_stream_event
{
    std::string e{};
    std::uint64_t u{};
    std::string s{};
    std::string b{};
    std::string B{};
    std::string a{};
    std::string A{};
    std::uint64_t T{};
    std::uint64_t E{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Aggregate-Trade-Streams.md
struct aggregate_trade_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::uint64_t a{};
    std::string p{};
    std::string q{};
    std::uint64_t f{};
    std::uint64_t l{};
    std::uint64_t T{};
    bool m{};
    std::optional<std::string> nq{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream.md
struct mark_price_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string p{};
    std::optional<std::string> ap{};
    std::string i{};
    std::string P{};
    std::string r{};
    std::uint64_t T{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream-for-All-market.md
using all_market_mark_price_stream_event = std::vector<mark_price_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams.md
struct depth_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::string s{};
    std::uint64_t U{};
    std::uint64_t u{};
    std::uint64_t pu{};
    std::vector<price_level> b{};
    std::vector<price_level> a{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
struct mini_ticker_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string c{};
    std::string o{};
    std::string h{};
    std::string l{};
    std::string v{};
    std::string q{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
using all_market_mini_ticker_stream_event = std::vector<mini_ticker_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
struct ticker_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string p{};
    std::string P{};
    std::string w{};
    std::string c{};
    std::string Q{};
    std::string o{};
    std::string h{};
    std::string l{};
    std::string v{};
    std::string q{};
    std::uint64_t O{};
    std::uint64_t C{};
    std::uint64_t F{};
    std::uint64_t L{};
    std::uint64_t n{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
using all_market_ticker_stream_event = std::vector<ticker_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_data
{
    std::string s{};
    std::string S{};
    std::string o{};
    std::string f{};
    std::string q{};
    std::string p{};
    std::string ap{};
    std::string X{};
    std::string l{};
    std::string z{};
    std::uint64_t T{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
struct liquidation_order_stream_event
{
    std::string e{};
    std::uint64_t E{};
    liquidation_order_stream_data o{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
struct kline_stream_data
{
    std::uint64_t t{};
    std::uint64_t T{};
    std::string s{};
    std::string i{};
    std::uint64_t f{};
    std::uint64_t L{};
    std::string o{};
    std::string c{};
    std::string h{};
    std::string l{};
    std::string v{};
    std::uint64_t n{};
    bool x{};
    std::string q{};
    std::string V{};
    std::string Q{};
    std::string B{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
struct kline_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    kline_stream_data k{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
struct continuous_contract_kline_stream_data
{
    std::uint64_t t{};
    std::uint64_t T{};
    std::string i{};
    std::uint64_t f{};
    std::uint64_t L{};
    std::string o{};
    std::string c{};
    std::string h{};
    std::string l{};
    std::string v{};
    std::uint64_t n{};
    bool x{};
    std::string q{};
    std::string V{};
    std::string Q{};
    std::string B{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
struct continuous_contract_kline_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string ps{};
    std::string ct{};
    continuous_contract_kline_stream_data k{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_constituent
{
    std::string b{};
    std::string q{};
    std::string w{};
    std::string W{};
    std::string i{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
struct composite_index_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string p{};
    std::optional<std::string> C{};
    std::vector<composite_index_constituent> c{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
struct contract_info_bracket
{
    int bs{};
    double bnf{};
    double bnc{};
    double mmr{};
    double cf{};
    int mi{};
    int ma{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
struct contract_info_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string ps{};
    std::string ct{};
    std::uint64_t dt{};
    std::uint64_t ot{};
    std::string cs{};
    std::optional<std::vector<contract_info_bracket>> bks{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
struct asset_index_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string i{};
    std::string b{};
    std::string a{};
    std::string B{};
    std::string A{};
    std::string q{};
    std::string g{};
    std::string Q{};
    std::string G{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
using all_asset_index_stream_event = std::vector<asset_index_stream_event>;

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Trading-Session-Stream.md
struct trading_session_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t t{};
    std::uint64_t T{};
    std::string S{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_balance
{
    std::string a{};
    std::string wb{};
    std::string cw{};
    std::string bc{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_position
{
    std::string s{};
    std::string pa{};
    std::string ep{};
    std::string cr{};
    std::string up{};
    std::string mt{};
    std::string iw{};
    std::string ps{};
    std::string bep{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_data
{
    std::string m{};
    std::vector<account_update_balance> B{};
    std::vector<account_update_position> P{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
struct account_update_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    account_update_data a{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_order
{
    std::string s{};
    std::string c{};
    std::string S{};
    std::string o{};
    std::string f{};
    std::string q{};
    std::string p{};
    std::string ap{};
    std::string sp{};
    std::string x{};
    std::string X{};
    std::uint64_t i{};
    std::string l{};
    std::string z{};
    std::string L{};
    std::string N{};
    std::string n{};
    std::uint64_t T{};
    std::uint64_t t{};
    std::string b{};
    std::string a{};
    bool m{};
    bool R{};
    std::string wt{};
    std::string ot{};
    std::string ps{};
    bool cp{};
    std::optional<std::string> AP{};
    std::optional<std::string> cr{};
    bool pP{};
    int si{};
    int ss{};
    std::string rp{};
    std::string V{};
    std::string pm{};
    std::optional<std::uint64_t> gtd{};
    std::optional<std::string> er{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
struct order_trade_update_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    order_trade_update_order o{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_position
{
    std::string s{};
    std::string ps{};
    std::string pa{};
    std::string mt{};
    std::string iw{};
    std::string mp{};
    std::string up{};
    std::string mm{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
struct margin_call_event
{
    std::string e{};
    std::uint64_t E{};
    std::string cw{};
    std::vector<margin_call_position> p{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-User-Data-Stream-Expired.md
struct listen_key_expired_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::string listenKey{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_leverage
{
    std::string s{};
    int l{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_multi_assets
{
    bool j{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
struct account_config_update_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::optional<account_config_leverage> ac{};
    std::optional<account_config_multi_assets> ai{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md
struct trade_lite_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::string s{};
    std::string q{};
    std::string p{};
    bool m{};
    std::string c{};
    std::string S{};
    std::string L{};
    std::string l{};
    std::uint64_t t{};
    std::uint64_t i{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_data
{
    std::string caid{};
    std::uint64_t aid{};
    std::string at{};
    std::string o{};
    std::string s{};
    std::string S{};
    std::string ps{};
    std::string f{};
    std::string q{};
    std::string X{};
    std::optional<std::string> ai{};
    std::optional<std::string> ap{};
    std::optional<std::string> aq{};
    std::optional<std::string> act{};
    std::optional<std::string> tp{};
    std::optional<std::string> p{};
    std::optional<std::string> V{};
    std::optional<std::string> wt{};
    std::optional<std::string> pm{};
    std::optional<bool> cp{};
    std::optional<bool> pP{};
    std::optional<bool> R{};
    std::optional<std::uint64_t> tt{};
    std::optional<std::uint64_t> gtd{};
    std::optional<std::string> rm{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
struct algo_order_update_event
{
    std::string e{};
    std::uint64_t T{};
    std::uint64_t E{};
    algo_order_update_data o{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_reject_data
{
    std::string s{};
    std::uint64_t i{};
    std::string r{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
struct conditional_order_trigger_reject_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    conditional_order_reject_data or_{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_data
{
    std::uint64_t si{};
    std::string st{};
    std::string ss{};
    std::string s{};
    std::string r{};
    std::string up{};
    std::string uq{};
    std::string uf{};
    std::string mp{};
    std::uint64_t ut{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
struct grid_update_event
{
    std::string e{};
    std::uint64_t T{};
    std::uint64_t E{};
    grid_update_data gu{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_data
{
    std::uint64_t si{};
    std::string st{};
    std::string ss{};
    std::string s{};
    std::uint64_t ut{};
    int c{};
};

// doc: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
struct strategy_update_event
{
    std::string e{};
    std::uint64_t T{};
    std::uint64_t E{};
    strategy_update_data su{};
};

} // namespace binapi2::fapi::types

template<>
struct glz::meta<binapi2::fapi::types::book_ticker_stream_event>
{
    using T = binapi2::fapi::types::book_ticker_stream_event;
    static constexpr auto value =
        object("e", &T::e, "u", &T::u, "s", &T::s, "b", &T::b, "B", &T::B, "a", &T::a, "A", &T::A, "T", &T::T, "E", &T::E);
};

template<>
struct glz::meta<binapi2::fapi::types::aggregate_trade_stream_event>
{
    using T = binapi2::fapi::types::aggregate_trade_stream_event;
    static constexpr auto value = object("e",
                                         &T::e,
                                         "E",
                                         &T::E,
                                         "s",
                                         &T::s,
                                         "a",
                                         &T::a,
                                         "p",
                                         &T::p,
                                         "q",
                                         &T::q,
                                         "f",
                                         &T::f,
                                         "l",
                                         &T::l,
                                         "T",
                                         &T::T,
                                         "m",
                                         &T::m,
                                         "nq",
                                         &T::nq);
};

template<>
struct glz::meta<binapi2::fapi::types::mark_price_stream_event>
{
    using T = binapi2::fapi::types::mark_price_stream_event;
    static constexpr auto value =
        object("e", &T::e, "E", &T::E, "s", &T::s, "p", &T::p, "ap", &T::ap, "i", &T::i, "P", &T::P, "r", &T::r, "T", &T::T);
};

template<>
struct glz::meta<binapi2::fapi::types::depth_stream_event>
{
    using T = binapi2::fapi::types::depth_stream_event;
    static constexpr auto value =
        object("e", &T::e, "E", &T::E, "T", &T::T, "s", &T::s, "U", &T::U, "u", &T::u, "pu", &T::pu, "b", &T::b, "a", &T::a);
};

template<>
struct glz::meta<binapi2::fapi::types::mini_ticker_stream_event>
{
    using T = binapi2::fapi::types::mini_ticker_stream_event;
    static constexpr auto value =
        object("e", &T::e, "E", &T::E, "s", &T::s, "c", &T::c, "o", &T::o, "h", &T::h, "l", &T::l, "v", &T::v, "q", &T::q);
};

template<>
struct glz::meta<binapi2::fapi::types::ticker_stream_event>
{
    using T = binapi2::fapi::types::ticker_stream_event;
    static constexpr auto value = object("e",
                                         &T::e,
                                         "E",
                                         &T::E,
                                         "s",
                                         &T::s,
                                         "p",
                                         &T::p,
                                         "P",
                                         &T::P,
                                         "w",
                                         &T::w,
                                         "c",
                                         &T::c,
                                         "Q",
                                         &T::Q,
                                         "o",
                                         &T::o,
                                         "h",
                                         &T::h,
                                         "l",
                                         &T::l,
                                         "v",
                                         &T::v,
                                         "q",
                                         &T::q,
                                         "O",
                                         &T::O,
                                         "C",
                                         &T::C,
                                         "F",
                                         &T::F,
                                         "L",
                                         &T::L,
                                         "n",
                                         &T::n);
};

template<>
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_data>
{
    using T = binapi2::fapi::types::liquidation_order_stream_data;
    static constexpr auto value = object("s",
                                         &T::s,
                                         "S",
                                         &T::S,
                                         "o",
                                         &T::o,
                                         "f",
                                         &T::f,
                                         "q",
                                         &T::q,
                                         "p",
                                         &T::p,
                                         "ap",
                                         &T::ap,
                                         "X",
                                         &T::X,
                                         "l",
                                         &T::l,
                                         "z",
                                         &T::z,
                                         "T",
                                         &T::T);
};

template<>
struct glz::meta<binapi2::fapi::types::liquidation_order_stream_event>
{
    using T = binapi2::fapi::types::liquidation_order_stream_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "o", &T::o);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_stream_data>
{
    using T = binapi2::fapi::types::kline_stream_data;
    static constexpr auto value = object("t",
                                         &T::t,
                                         "T",
                                         &T::T,
                                         "s",
                                         &T::s,
                                         "i",
                                         &T::i,
                                         "f",
                                         &T::f,
                                         "L",
                                         &T::L,
                                         "o",
                                         &T::o,
                                         "c",
                                         &T::c,
                                         "h",
                                         &T::h,
                                         "l",
                                         &T::l,
                                         "v",
                                         &T::v,
                                         "n",
                                         &T::n,
                                         "x",
                                         &T::x,
                                         "q",
                                         &T::q,
                                         "V",
                                         &T::V,
                                         "Q",
                                         &T::Q,
                                         "B",
                                         &T::B);
};

template<>
struct glz::meta<binapi2::fapi::types::kline_stream_event>
{
    using T = binapi2::fapi::types::kline_stream_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "s", &T::s, "k", &T::k);
};

template<>
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_data>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_data;
    static constexpr auto value = object("t",
                                         &T::t,
                                         "T",
                                         &T::T,
                                         "i",
                                         &T::i,
                                         "f",
                                         &T::f,
                                         "L",
                                         &T::L,
                                         "o",
                                         &T::o,
                                         "c",
                                         &T::c,
                                         "h",
                                         &T::h,
                                         "l",
                                         &T::l,
                                         "v",
                                         &T::v,
                                         "n",
                                         &T::n,
                                         "x",
                                         &T::x,
                                         "q",
                                         &T::q,
                                         "V",
                                         &T::V,
                                         "Q",
                                         &T::Q,
                                         "B",
                                         &T::B);
};

template<>
struct glz::meta<binapi2::fapi::types::continuous_contract_kline_stream_event>
{
    using T = binapi2::fapi::types::continuous_contract_kline_stream_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "ps", &T::ps, "ct", &T::ct, "k", &T::k);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_balance>
{
    using T = binapi2::fapi::types::account_update_balance;
    static constexpr auto value = object("a", &T::a, "wb", &T::wb, "cw", &T::cw, "bc", &T::bc);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_position>
{
    using T = binapi2::fapi::types::account_update_position;
    static constexpr auto value =
        object("s", &T::s, "pa", &T::pa, "ep", &T::ep, "cr", &T::cr, "up", &T::up, "mt", &T::mt, "iw", &T::iw, "ps", &T::ps, "bep", &T::bep);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_data>
{
    using T = binapi2::fapi::types::account_update_data;
    static constexpr auto value = object("m", &T::m, "B", &T::B, "P", &T::P);
};

template<>
struct glz::meta<binapi2::fapi::types::account_update_event>
{
    using T = binapi2::fapi::types::account_update_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "T", &T::T, "a", &T::a);
};

template<>
struct glz::meta<binapi2::fapi::types::order_trade_update_order>
{
    using T = binapi2::fapi::types::order_trade_update_order;
    static constexpr auto value = object("s", &T::s, "c", &T::c, "S", &T::S, "o", &T::o, "f", &T::f,
                                         "q", &T::q, "p", &T::p, "ap", &T::ap, "sp", &T::sp, "x", &T::x,
                                         "X", &T::X, "i", &T::i, "l", &T::l, "z", &T::z, "L", &T::L,
                                         "N", &T::N, "n", &T::n, "T", &T::T, "t", &T::t, "b", &T::b,
                                         "a", &T::a, "m", &T::m, "R", &T::R, "wt", &T::wt, "ot", &T::ot,
                                         "ps", &T::ps, "cp", &T::cp, "AP", &T::AP, "cr", &T::cr, "pP", &T::pP,
                                         "si", &T::si, "ss", &T::ss, "rp", &T::rp, "V", &T::V, "pm", &T::pm,
                                         "gtd", &T::gtd, "er", &T::er);
};

template<>
struct glz::meta<binapi2::fapi::types::order_trade_update_event>
{
    using T = binapi2::fapi::types::order_trade_update_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "T", &T::T, "o", &T::o);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_position>
{
    using T = binapi2::fapi::types::margin_call_position;
    static constexpr auto value =
        object("s", &T::s, "ps", &T::ps, "pa", &T::pa, "mt", &T::mt, "iw", &T::iw, "mp", &T::mp, "up", &T::up, "mm", &T::mm);
};

template<>
struct glz::meta<binapi2::fapi::types::margin_call_event>
{
    using T = binapi2::fapi::types::margin_call_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "cw", &T::cw, "p", &T::p);
};

template<>
struct glz::meta<binapi2::fapi::types::listen_key_expired_event>
{
    using T = binapi2::fapi::types::listen_key_expired_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "T", &T::T, "listenKey", &T::listenKey);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_constituent>
{
    using T = binapi2::fapi::types::composite_index_constituent;
    static constexpr auto value = object("b", &T::b, "q", &T::q, "w", &T::w, "W", &T::W, "i", &T::i);
};

template<>
struct glz::meta<binapi2::fapi::types::composite_index_stream_event>
{
    using T = binapi2::fapi::types::composite_index_stream_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "s", &T::s, "p", &T::p, "C", &T::C, "c", &T::c);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_bracket>
{
    using T = binapi2::fapi::types::contract_info_bracket;
    static constexpr auto value =
        object("bs", &T::bs, "bnf", &T::bnf, "bnc", &T::bnc, "mmr", &T::mmr, "cf", &T::cf, "mi", &T::mi, "ma", &T::ma);
};

template<>
struct glz::meta<binapi2::fapi::types::contract_info_stream_event>
{
    using T = binapi2::fapi::types::contract_info_stream_event;
    static constexpr auto value = object("e",
                                         &T::e,
                                         "E",
                                         &T::E,
                                         "s",
                                         &T::s,
                                         "ps",
                                         &T::ps,
                                         "ct",
                                         &T::ct,
                                         "dt",
                                         &T::dt,
                                         "ot",
                                         &T::ot,
                                         "cs",
                                         &T::cs,
                                         "bks",
                                         &T::bks);
};

template<>
struct glz::meta<binapi2::fapi::types::asset_index_stream_event>
{
    using T = binapi2::fapi::types::asset_index_stream_event;
    static constexpr auto value = object("e",
                                         &T::e,
                                         "E",
                                         &T::E,
                                         "s",
                                         &T::s,
                                         "i",
                                         &T::i,
                                         "b",
                                         &T::b,
                                         "a",
                                         &T::a,
                                         "B",
                                         &T::B,
                                         "A",
                                         &T::A,
                                         "q",
                                         &T::q,
                                         "g",
                                         &T::g,
                                         "Q",
                                         &T::Q,
                                         "G",
                                         &T::G);
};

template<>
struct glz::meta<binapi2::fapi::types::trading_session_stream_event>
{
    using T = binapi2::fapi::types::trading_session_stream_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "t", &T::t, "T", &T::T, "S", &T::S);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_leverage>
{
    using T = binapi2::fapi::types::account_config_leverage;
    static constexpr auto value = object("s", &T::s, "l", &T::l);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_multi_assets>
{
    using T = binapi2::fapi::types::account_config_multi_assets;
    static constexpr auto value = object("j", &T::j);
};

template<>
struct glz::meta<binapi2::fapi::types::account_config_update_event>
{
    using T = binapi2::fapi::types::account_config_update_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "T", &T::T, "ac", &T::ac, "ai", &T::ai);
};

template<>
struct glz::meta<binapi2::fapi::types::trade_lite_event>
{
    using T = binapi2::fapi::types::trade_lite_event;
    static constexpr auto value = object("e",
                                         &T::e,
                                         "E",
                                         &T::E,
                                         "T",
                                         &T::T,
                                         "s",
                                         &T::s,
                                         "q",
                                         &T::q,
                                         "p",
                                         &T::p,
                                         "m",
                                         &T::m,
                                         "c",
                                         &T::c,
                                         "S",
                                         &T::S,
                                         "L",
                                         &T::L,
                                         "l",
                                         &T::l,
                                         "t",
                                         &T::t,
                                         "i",
                                         &T::i);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_order_update_data>
{
    using T = binapi2::fapi::types::algo_order_update_data;
    static constexpr auto value = object("caid",
                                         &T::caid,
                                         "aid",
                                         &T::aid,
                                         "at",
                                         &T::at,
                                         "o",
                                         &T::o,
                                         "s",
                                         &T::s,
                                         "S",
                                         &T::S,
                                         "ps",
                                         &T::ps,
                                         "f",
                                         &T::f,
                                         "q",
                                         &T::q,
                                         "X",
                                         &T::X,
                                         "ai",
                                         &T::ai,
                                         "ap",
                                         &T::ap,
                                         "aq",
                                         &T::aq,
                                         "act",
                                         &T::act,
                                         "tp",
                                         &T::tp,
                                         "p",
                                         &T::p,
                                         "V",
                                         &T::V,
                                         "wt",
                                         &T::wt,
                                         "pm",
                                         &T::pm,
                                         "cp",
                                         &T::cp,
                                         "pP",
                                         &T::pP,
                                         "R",
                                         &T::R,
                                         "tt",
                                         &T::tt,
                                         "gtd",
                                         &T::gtd,
                                         "rm",
                                         &T::rm);
};

template<>
struct glz::meta<binapi2::fapi::types::algo_order_update_event>
{
    using T = binapi2::fapi::types::algo_order_update_event;
    static constexpr auto value = object("e", &T::e, "T", &T::T, "E", &T::E, "o", &T::o);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_reject_data>
{
    using T = binapi2::fapi::types::conditional_order_reject_data;
    static constexpr auto value = object("s", &T::s, "i", &T::i, "r", &T::r);
};

template<>
struct glz::meta<binapi2::fapi::types::conditional_order_trigger_reject_event>
{
    using T = binapi2::fapi::types::conditional_order_trigger_reject_event;
    static constexpr auto value = object("e", &T::e, "E", &T::E, "T", &T::T, "or", &T::or_);
};

template<>
struct glz::meta<binapi2::fapi::types::grid_update_data>
{
    using T = binapi2::fapi::types::grid_update_data;
    static constexpr auto value = object("si",
                                         &T::si,
                                         "st",
                                         &T::st,
                                         "ss",
                                         &T::ss,
                                         "s",
                                         &T::s,
                                         "r",
                                         &T::r,
                                         "up",
                                         &T::up,
                                         "uq",
                                         &T::uq,
                                         "uf",
                                         &T::uf,
                                         "mp",
                                         &T::mp,
                                         "ut",
                                         &T::ut);
};

template<>
struct glz::meta<binapi2::fapi::types::grid_update_event>
{
    using T = binapi2::fapi::types::grid_update_event;
    static constexpr auto value = object("e", &T::e, "T", &T::T, "E", &T::E, "gu", &T::gu);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_data>
{
    using T = binapi2::fapi::types::strategy_update_data;
    static constexpr auto value = object("si", &T::si, "st", &T::st, "ss", &T::ss, "s", &T::s, "ut", &T::ut, "c", &T::c);
};

template<>
struct glz::meta<binapi2::fapi::types::strategy_update_event>
{
    using T = binapi2::fapi::types::strategy_update_event;
    static constexpr auto value = object("e", &T::e, "T", &T::T, "E", &T::E, "su", &T::su);
};
