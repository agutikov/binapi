// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor drain-and-route loop.

#include "drain.hpp"

#include "helpers.hpp"

#include <string_view>

namespace binapi2::examples::async_recorder::detail_impl {

boost::cobalt::task<void>
drain_loop(detail_state& st)
{
    while (true) {
        auto r = co_await st.record_buf.async_read();
        if (!r) break;

        const auto& frame = *r;
        auto parsed = parse_stream_header(frame);
        if (!parsed) continue;
        const auto upper = to_upper_copy(parsed->first);

        auto sym_it = st.sinks.find(upper);
        if (sym_it == st.sinks.end()) continue;  // unsubscribed mid-flight
        auto& psink = sym_it->second;

        // type is whatever follows the first '@' in the topic. For
        // single-segment topics (aggTrade, bookTicker, forceOrder) it's
        // an exact match; for markPrice the topic is
        // `<sym>@markPrice@1s` so `type` is `markPrice@1s` — use
        // starts_with for that case. For depth the topic can be
        // `depth5@100ms`, `depth10@100ms`, `depth20@100ms`, or
        // `depth@100ms` — starts_with("depth") matches all of them.
        const auto& type = parsed->second;
        if (type == "aggTrade" && psink.agg_trade) {
            co_await (*psink.agg_trade)(frame);
            ++psink.frames_routed;
        } else if (type == "bookTicker" && psink.book_ticker) {
            co_await (*psink.book_ticker)(frame);
            ++psink.frames_routed;
        } else if (std::string_view(type).starts_with("markPrice") &&
                   psink.mark_price) {
            co_await (*psink.mark_price)(frame);
            ++psink.frames_routed;
        } else if (type == "forceOrder" && psink.force_order) {
            co_await (*psink.force_order)(frame);
            ++psink.frames_routed;
        } else if (std::string_view(type).starts_with("depth") &&
                   psink.depth) {
            co_await (*psink.depth)(frame);
            ++psink.frames_routed;
        }
    }
}

} // namespace binapi2::examples::async_recorder::detail_impl
