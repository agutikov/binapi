// SPDX-License-Identifier: Apache-2.0
//
// POD option structs for the parameter shapes shared between multiple
// commands. Both the CLI (CLI11 `add_option`) and the UI (FTXUI `Input` /
// `Toggle` widgets) bind their surface fields to these structs, then hand
// them to the shared `make_*_request` builders in `builders.hpp`.
//
// One-off parameter shapes that appear in a single command (e.g. `basis`'s
// `<pair> <period> [--limit N]`, or `modify-isolated-margin`'s triple of
// symbol/amount/type) stay inline in their respective surface files —
// lifting them here wouldn't eliminate any duplication.

#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace binapi2::demo {

/// Commands taking only a `<symbol>` argument.
///
/// Used by both the required-symbol shape (`book-ticker`) and the
/// optional-symbol shape (`exchange-info [symbol]`) — the binding
/// surface decides which.
struct symbol_opts
{
    std::string symbol;
};

/// `<symbol>` + optional `--limit N`.
struct symbol_limit_opts
{
    std::string symbol;
    std::optional<int> limit;
};

/// `<symbol> <orderId>` — the shape for query-order / cancel-order / etc.
struct symbol_order_id_opts
{
    std::string symbol;
    std::uint64_t order_id = 0;
};

/// `<symbol> <interval>` + optional `--limit N` — klines family.
struct kline_opts
{
    std::string symbol;
    std::string interval;
    std::optional<int> limit;
};

/// `<pair> <interval>` + optional `--limit N` — continuous kline / index kline.
struct pair_kline_opts
{
    std::string pair;
    std::string interval;
    std::optional<int> limit;
};

/// `<symbol> <period>` + optional `--limit N` — futures analytics family
/// (open-interest-stats, top L/S ratio, taker volume, …).
struct analytics_opts
{
    std::string symbol;
    std::string period;
    std::optional<int> limit;
};

/// `<startTime> <endTime>` — download-id-* requests (epoch milliseconds).
struct download_id_opts
{
    std::uint64_t start_time = 0;
    std::uint64_t end_time = 0;
};

/// `<downloadId>` — download-link-* requests.
struct download_link_opts
{
    std::string download_id;
};

/// `<symbol> <side> <type> [-q Q] [-p P] [-t TIF]` — new-order family.
/// Holds string copies of the typed fields so the surface doesn't need to
/// understand enums/decimals; the builder converts on the way into the
/// typed request.
struct order_opts
{
    std::string symbol;
    std::string side;
    std::string type;
    std::string quantity;
    std::string price;
    std::string tif;
};

} // namespace binapi2::demo
