// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor internal state types.

/// @file detail/types.hpp
/// @brief Per-symbol sinks and the shared detail_state. Internal to the
///        detail monitor implementation — not part of the example's
///        public interface (`../detail.hpp`). Nested namespace
///        `detail_impl` keeps these out of the way of
///        `binapi2::fapi::detail`.

#pragma once

#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace binapi2::examples::async_recorder::detail_impl {

using rfs = ::binapi2::fapi::streams::sinks::rotating_file_sink;

/// @brief All per-symbol sinks the detail monitor can write to.
///
/// Complete Tier-0 set: aggTrade, bookTicker, markPrice@1s, forceOrder.
/// Optional Tier-1: `depth` slot holds either the partial-book snapshot
/// stream (F3) or the full diff stream (F4), depending on config.
struct per_symbol_sinks
{
    std::unique_ptr<rfs> agg_trade;
    std::unique_ptr<rfs> book_ticker;
    std::unique_ptr<rfs> mark_price;
    std::unique_ptr<rfs> force_order;
    std::unique_ptr<rfs> depth;
    std::size_t frames_routed{ 0 };
};

/// @brief Shared state between the detail monitor's coroutines.
///
/// Accessed from the drain loop, the subscription manager, and the
/// status source — all on the same executor, so no synchronisation.
struct detail_state
{
    ::binapi2::fapi::detail::stream_buffer<std::string> record_buf;
    std::unordered_map<std::string, per_symbol_sinks> sinks{};
    std::unordered_set<std::string> subscribed{};
    std::size_t snaps_ok{ 0 };
    std::size_t snaps_err{ 0 };
    bool closed{ false };

    explicit detail_state(std::size_t capacity) : record_buf(capacity) {}
};

} // namespace binapi2::examples::async_recorder::detail_impl
