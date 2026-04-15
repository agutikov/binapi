// SPDX-License-Identifier: Apache-2.0
//
// Typed request builders. Each function takes a surface-populated POD
// `*_opts` struct from `opts.hpp` and returns a typed Binance request
// struct with the fields copied in (and enum strings parsed).
//
// The CLI's CLI11 callbacks and the UI's FTXUI form callbacks both land
// here once the user has filled in a form — the request-building logic
// lives in one place.

#pragma once

#include "enum_parse.hpp"
#include "opts.hpp"

#include <binapi2/fapi/types/detail/decimal.hpp>
#include <binapi2/fapi/types/detail/timestamp.hpp>
#include <binapi2/fapi/types/enums.hpp>

namespace binapi2::demo {

/// `<symbol>` only — required or optional, the surface decides.
/// Callers that only want `req.symbol` set should pass a string directly;
/// this overload is a no-op helper for symmetry with the other builders.
template<typename Request>
Request make_symbol_request(const symbol_opts& o)
{
    Request req;
    req.symbol = o.symbol;
    return req;
}

/// `<symbol> [--limit N]` — `req.symbol` plus `req.limit` (optional int).
template<typename Request>
Request make_symbol_limit_request(const symbol_limit_opts& o)
{
    Request req;
    req.symbol = o.symbol;
    req.limit = o.limit;
    return req;
}

/// `<symbol> <orderId>` — `req.symbol` + `req.orderId`.
template<typename Request>
Request make_symbol_order_id_request(const symbol_order_id_opts& o)
{
    Request req;
    req.symbol = o.symbol;
    req.orderId = o.order_id;
    return req;
}

/// `<symbol> <interval> [--limit N]` — parses the interval via glaze meta.
template<typename Request>
Request make_kline_request(const kline_opts& o)
{
    Request req;
    req.symbol = o.symbol;
    req.interval = parse_enum<binapi2::fapi::types::kline_interval_t>(o.interval);
    req.limit = o.limit;
    return req;
}

/// `<pair> <interval> [--limit N]`.
template<typename Request>
Request make_pair_kline_request(const pair_kline_opts& o)
{
    Request req;
    req.pair = o.pair;
    req.interval = parse_enum<binapi2::fapi::types::kline_interval_t>(o.interval);
    req.limit = o.limit;
    return req;
}

/// `<symbol> <period> [--limit N]`. The period is parsed as a kline
/// interval (`5m`, `15m`, `1h`, …), matching the underlying Binance
/// endpoints' accepted values.
template<typename Request>
Request make_analytics_request(const analytics_opts& o)
{
    Request req;
    req.symbol = o.symbol;
    req.period = parse_enum<binapi2::fapi::types::kline_interval_t>(o.period);
    req.limit = o.limit;
    return req;
}

/// `<startTime> <endTime>` (epoch-ms) → a download-id request.
template<typename Request>
Request make_download_id_request(const download_id_opts& o)
{
    Request req;
    req.startTime = binapi2::fapi::types::timestamp_ms_t{ o.start_time };
    req.endTime = binapi2::fapi::types::timestamp_ms_t{ o.end_time };
    return req;
}

/// `<downloadId>` → a download-link request.
template<typename Request>
Request make_download_link_request(const download_link_opts& o)
{
    Request req;
    req.downloadId = o.download_id;
    return req;
}

/// `<symbol> <side> <type> [-q] [-p] [-t]` → a new-order-shaped request.
/// Works for any request type with the expected field names, which covers
/// `new_order_request_t`, `test_order_request_t`, and
/// `websocket_api_order_place_request_t`.
template<typename Request>
Request make_order_request(const order_opts& o)
{
    namespace types = binapi2::fapi::types;
    Request req;
    req.symbol = o.symbol;
    req.side = parse_enum<types::order_side_t>(o.side);
    req.type = parse_enum<types::order_type_t>(o.type);
    if (!o.quantity.empty()) req.quantity = types::decimal_t(o.quantity);
    if (!o.price.empty())    req.price    = types::decimal_t(o.price);
    if (!o.tif.empty())      req.timeInForce = parse_enum<types::time_in_force_t>(o.tif);
    return req;
}

} // namespace binapi2::demo
