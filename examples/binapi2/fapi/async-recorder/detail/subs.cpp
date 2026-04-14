// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor subscription management.

#include "subs.hpp"

#include "../selector.hpp"
#include "helpers.hpp"
#include "snapshot.hpp"

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <utility>

#include <boost/asio/steady_timer.hpp>
#include <boost/cobalt/op.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace fapi = ::binapi2::fapi;
namespace streams = ::binapi2::fapi::streams;
namespace types = ::binapi2::fapi::types;

namespace binapi2::examples::async_recorder::detail_impl {

boost::cobalt::task<void>
connection_read_loop(streams::dynamic_market_stream& d)
{
    while (true) {
        auto r = co_await d.connection().async_read_text();
        if (!r) break;
    }
}

boost::cobalt::task<void>
manage_subs_loop(const recorder_config& cfg,
                 selector& sel,
                 streams::dynamic_market_stream& dyn,
                 ::binapi2::fapi::rest::client& rest_client,
                 detail_state& st,
                 boost::asio::io_context& ioc)
{
    boost::asio::steady_timer timer(ioc);
    const auto interval = std::chrono::seconds(
        std::max<std::uint64_t>(1, cfg.stats_interval_seconds));

    while (!st.closed) {
        timer.expires_after(interval);
        try {
            co_await timer.async_wait(boost::cobalt::use_op);
        } catch (const std::exception&) {
            break;  // shutdown — timer cancelled
        }

        const auto& target = sel.active();

        std::vector<std::string> to_add;
        std::vector<std::string> to_remove;
        for (const auto& s : target)
            if (!st.subscribed.count(s)) to_add.push_back(s);
        for (const auto& s : st.subscribed)
            if (!target.count(s)) to_remove.push_back(s);

        // -- Admit: snapshot + open sinks + SUBSCRIBE ----------------------
        const bool partial_depth =
            cfg.with_depth && cfg.depth_mode == depth_mode_t::partial;
        const bool full_depth =
            cfg.with_depth && cfg.depth_mode == depth_mode_t::full;

        for (const auto& sym : to_add) {
            // REST snapshot before any subscribe — gives the diff stream
            // (full-depth mode) a well-defined anchor, and is useful as
            // a one-shot reference even in partial mode.
            if (co_await fetch_depth_snapshot(rest_client, cfg.root_dir,
                                              sym, "startup"))
                ++st.snaps_ok;
            else
                ++st.snaps_err;

            per_symbol_sinks psink;
            psink.agg_trade = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "aggTrade"));
            psink.book_ticker = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "bookTicker"));
            psink.mark_price = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "markPrice"));
            psink.force_order = std::make_unique<rfs>(
                ioc, make_detail_rfs_cfg(cfg, sym, "forceOrder"));

            if (partial_depth) {
                const std::string depth_dir =
                    "depth" + std::to_string(cfg.depth_levels);
                psink.depth = std::make_unique<rfs>(
                    ioc, make_detail_rfs_cfg(cfg, sym, depth_dir));
            } else if (full_depth) {
                psink.depth = std::make_unique<rfs>(
                    ioc, make_detail_rfs_cfg(cfg, sym, "depth_diff"));
                psink.next_resnap_at = std::chrono::steady_clock::now() +
                    std::chrono::seconds(cfg.depth_resnap_seconds);
            }

            st.sinks.emplace(sym, std::move(psink));

            // Combine Tier-0 + (optional) depth into ONE control message.
            // Two separate SUBSCRIBE calls would double our control-
            // message rate to the broker and trip Binance's
            // 5-per-second rate limit, which it answers with
            // "Operation canceled" and a connection drop.
            fapi::result<void> r;
            if (partial_depth) {
                r = co_await dyn.async_subscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym },
                    types::partial_book_depth_subscription{
                        .symbol = sym,
                        .levels = cfg.depth_levels,
                        .speed = "100ms" });
            } else if (full_depth) {
                r = co_await dyn.async_subscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym },
                    types::diff_book_depth_subscription{
                        .symbol = sym,
                        .speed = "100ms" });
            } else {
                r = co_await dyn.async_subscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym });
            }
            if (!r) {
                spdlog::warn("detail[{}]: subscribe failed: {}", sym, r.err.message);
                st.sinks.erase(sym);
                continue;
            }

            st.subscribed.insert(sym);
            spdlog::info("detail[{}]: subscribed Tier-0{}", sym,
                         partial_depth ? " + partial depth"
                         : full_depth ? " + diff depth" : "");
        }

        // -- Evict: UNSUBSCRIBE then drop sinks ----------------------------
        for (const auto& sym : to_remove) {
            // Mirror the admission shape: combine Tier-0 + (optional)
            // depth into one UNSUBSCRIBE message to keep the control-
            // message rate low.
            fapi::result<void> r;
            if (partial_depth) {
                r = co_await dyn.async_unsubscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym },
                    types::partial_book_depth_subscription{
                        .symbol = sym,
                        .levels = cfg.depth_levels,
                        .speed = "100ms" });
            } else if (full_depth) {
                r = co_await dyn.async_unsubscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym },
                    types::diff_book_depth_subscription{
                        .symbol = sym,
                        .speed = "100ms" });
            } else {
                r = co_await dyn.async_unsubscribe(
                    types::aggregate_trade_subscription{ .symbol = sym },
                    types::book_ticker_subscription{ .symbol = sym },
                    types::mark_price_subscription{ .symbol = sym, .every_1s = true },
                    types::liquidation_order_subscription{ .symbol = sym });
            }
            if (!r)
                spdlog::warn("detail[{}]: unsubscribe failed: {}",
                             sym, r.err.message);

            st.sinks.erase(sym);
            st.subscribed.erase(sym);
            spdlog::info("detail[{}]: unsubscribed, sinks closed", sym);
        }

        // -- Periodic depth re-snapshot (full-depth mode only) -------------
        // Walk every currently-subscribed symbol whose deadline has
        // passed, fetch a fresh REST snapshot tagged "resnap", and push
        // the deadline forward. This gives offline diff-stream
        // reconstruction multiple anchor points within a long run.
        if (full_depth) {
            const auto now = std::chrono::steady_clock::now();
            for (auto& [sym, psink] : st.sinks) {
                if (psink.next_resnap_at == std::chrono::steady_clock::time_point::max())
                    continue;
                if (now < psink.next_resnap_at) continue;

                if (co_await fetch_depth_snapshot(rest_client, cfg.root_dir,
                                                  sym, "resnap"))
                    ++st.snaps_ok;
                else
                    ++st.snaps_err;

                psink.next_resnap_at = std::chrono::steady_clock::now() +
                    std::chrono::seconds(cfg.depth_resnap_seconds);
            }
        }
    }

    spdlog::info("detail: subscription manager exiting");
    st.closed = true;
    st.record_buf.close();
}

} // namespace binapi2::examples::async_recorder::detail_impl
