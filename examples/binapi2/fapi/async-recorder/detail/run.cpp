// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor public entry point.
//
// Composes the detail-monitor coroutines (drain + subscription manager)
// against one dynamic_market_stream and one rest::client. The bulk of
// the logic lives in the sibling files in this directory.

#include "../detail.hpp"

#include "../selector.hpp"
#include "../status_reporter.hpp"
#include "drain.hpp"
#include "subs.hpp"
#include "types.hpp"

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/streams/dynamic_market_stream.hpp>
#include <binapi2/futures_usdm_api.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <spdlog/spdlog.h>

#include <string>

namespace fapi = ::binapi2::fapi;
namespace streams = ::binapi2::fapi::streams;

namespace binapi2::examples::async_recorder {

boost::cobalt::task<void>
detail_monitor_run(const recorder_config& cfg,
                   selector& sel,
                   status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    fapi::config net_cfg =
        cfg.testnet ? fapi::config::testnet_config() : fapi::config{};

    // Independent REST client for depth snapshots on symbol admission
    // (and, later, periodic re-anchor in F4). Uses its own HTTP
    // connection — binapi2 doesn't yet share a single pooled client
    // across stages.
    binapi2::futures_usdm_api rest_api(net_cfg);
    auto rc = co_await rest_api.create_rest_client();
    if (!rc) {
        spdlog::error("detail: REST connect failed: {}", rc.err.message);
        co_return;
    }
    auto& rest_client = **rc;
    spdlog::info("detail: REST client connected (for depth snapshots)");

    // Two `dynamic_market_stream` instances — one per Binance URL
    // category (P0.10 / S0.10.5). Public-category streams (bookTicker,
    // depth) and market-category streams (aggTrade, markPrice@1s,
    // forceOrder) cannot share a combined-stream connection: Binance
    // silently drops cross-category frames. The same_category-gated
    // `async_subscribe` enforces this at compile time inside
    // manage_subs_loop. Both connections feed the same shared
    // `record_buf`; the drain layer routes per-frame by stream-type
    // suffix so it doesn't matter which connection delivered any
    // given frame.
    streams::dynamic_market_stream dyn_pub(net_cfg, fapi::stream_category_e::public_);
    streams::dynamic_market_stream dyn_mkt(net_cfg, fapi::stream_category_e::market);
    detail_impl::detail_state st(16384);

    dyn_pub.connection().attach_buffer(st.record_buf);
    dyn_mkt.connection().attach_buffer(st.record_buf);

    if (auto conn = co_await dyn_pub.async_connect(); !conn) {
        spdlog::error("detail: /public connect failed: {}", conn.err.message);
        co_return;
    }
    spdlog::info("detail: connected to /public/stream endpoint");
    if (auto conn = co_await dyn_mkt.async_connect(); !conn) {
        spdlog::error("detail: /market connect failed: {}", conn.err.message);
        (void)co_await dyn_pub.async_close();
        co_return;
    }
    spdlog::info("detail: connected to /market/stream endpoint");

    // Spawn each connection's own read loop as a future so the drain
    // buffer gets fed from both sockets.
    auto f_read_pub = boost::cobalt::spawn(
        exec, detail_impl::connection_read_loop(dyn_pub), boost::asio::use_future);
    auto f_read_mkt = boost::cobalt::spawn(
        exec, detail_impl::connection_read_loop(dyn_mkt), boost::asio::use_future);

    status.add_source("detail", [&st, &sel]() {
        std::string s = "subs=" + std::to_string(st.subscribed.size());
        s += "/" + std::to_string(sel.cfg().max_active);
        s += " snaps=" + std::to_string(st.snaps_ok);
        s += "/" + std::to_string(st.snaps_err);
        s += " buf=[push=" + std::to_string(st.record_buf.pushed_total());
        s += " drn=" + std::to_string(st.record_buf.drained_total());
        s += " occ=" + std::to_string(st.record_buf.occupancy());
        s += " hw=" + std::to_string(st.record_buf.high_water()) + "]";
        return s;
    });

    // Join drain + manage loops. 2-task cobalt::join is stable.
    co_await boost::cobalt::join(
        detail_impl::drain_loop(st),
        detail_impl::manage_subs_loop(cfg, sel, dyn_pub, dyn_mkt, rest_client, st, ioc));

    // Tear down both WS connections so the read_loop futures complete.
    (void)co_await dyn_pub.async_close();
    (void)co_await dyn_mkt.async_close();
    if (f_read_pub.valid()) {
        try { f_read_pub.get(); } catch (...) {}
    }
    if (f_read_mkt.valid()) {
        try { f_read_mkt.get(); } catch (...) {}
    }
    spdlog::info("detail: done");
}

} // namespace binapi2::examples::async_recorder
