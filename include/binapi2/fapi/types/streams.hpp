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
};

struct mark_price_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    std::string p{};
    std::string i{};
    std::string P{};
    std::string r{};
    std::uint64_t T{};
};

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

struct kline_stream_event
{
    std::string e{};
    std::uint64_t E{};
    std::string s{};
    kline_stream_data k{};
};

struct account_update_balance
{
    std::string a{};
    std::string wb{};
    std::string cw{};
    std::string bc{};
};

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
};

struct account_update_data
{
    std::string m{};
    std::vector<account_update_balance> B{};
    std::vector<account_update_position> P{};
};

struct account_update_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    account_update_data a{};
};

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
    std::string X{};
    std::string x{};
    std::uint64_t i{};
};

struct order_trade_update_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    order_trade_update_order o{};
};

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

struct margin_call_event
{
    std::string e{};
    std::uint64_t E{};
    std::string cw{};
    std::vector<margin_call_position> p{};
};

struct listen_key_expired_event
{
    std::string e{};
    std::uint64_t E{};
    std::uint64_t T{};
    std::string listenKey{};
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
                                         &T::m);
};

template<>
struct glz::meta<binapi2::fapi::types::mark_price_stream_event>
{
    using T = binapi2::fapi::types::mark_price_stream_event;
    static constexpr auto value =
        object("e", &T::e, "E", &T::E, "s", &T::s, "p", &T::p, "i", &T::i, "P", &T::P, "r", &T::r, "T", &T::T);
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
        object("s", &T::s, "pa", &T::pa, "ep", &T::ep, "cr", &T::cr, "up", &T::up, "mt", &T::mt, "iw", &T::iw, "ps", &T::ps);
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
    static constexpr auto value = object("s",
                                         &T::s,
                                         "c",
                                         &T::c,
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
                                         "x",
                                         &T::x,
                                         "i",
                                         &T::i);
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
