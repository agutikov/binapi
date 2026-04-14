// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — selector coroutine driver.
//
// Kept separate from selector.cpp (pure logic) so the unit test target
// can link only the logic file and avoid dragging in status_reporter
// and the full coroutine runtime.

#include "selector.hpp"
#include "status_reporter.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <string>

namespace binapi2::examples::async_recorder {

boost::cobalt::task<void>
selector_run(const recorder_config& cfg,
             const aggregates_map& aggs,
             selector& sel,
             signals_writer& signals,
             status_reporter& status)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ioc = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    status.add_source("selector", [&sel, &signals]() {
        std::string s = "active=" + std::to_string(sel.active().size());
        s += "/" + std::to_string(sel.cfg().max_active);
        s += " signals=" + std::to_string(signals.lines_written());
        return s;
    });

    boost::asio::steady_timer timer(ioc);
    const auto interval = std::chrono::seconds(
        std::max<std::uint64_t>(1, cfg.stats_interval_seconds));

    while (true) {
        timer.expires_after(interval);
        try {
            co_await timer.async_wait(boost::cobalt::use_op);
        } catch (const std::exception&) {
            spdlog::info("selector: shutdown");
            break;
        }

        auto diffs = sel.tick(aggs, std::chrono::steady_clock::now());
        for (auto& ev : diffs) {
            signals.write(ev);
            spdlog::info("selector: {} {} score={:.2f} reason={}",
                         ev.action == selector_action::add ? "ADD" : "REMOVE",
                         ev.symbol, ev.score, ev.reason);
        }
    }
}

} // namespace binapi2::examples::async_recorder
