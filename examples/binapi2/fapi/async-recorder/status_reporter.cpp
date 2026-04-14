// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — periodic app-wide status logger.

#include "status_reporter.hpp"

#include <boost/cobalt/op.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <spdlog/spdlog.h>

#include <exception>

namespace binapi2::examples::async_recorder {

status_reporter::status_reporter(boost::asio::io_context& ioc,
                                 std::chrono::seconds interval) :
    timer_(ioc),
    interval_(interval)
{
}

void status_reporter::add_source(std::string name, source_fn fn)
{
    sources_.push_back({ std::move(name), std::move(fn) });
}

void status_reporter::close()
{
    closed_ = true;
    timer_.cancel();
}

boost::cobalt::task<void> status_reporter::run()
{
    while (!closed_) {
        timer_.expires_after(interval_);
        // async_wait throws system_error on cancel; catch and loop.
        try {
            co_await timer_.async_wait(boost::cobalt::use_op);
        } catch (const boost::system::system_error&) {
            // Cancelled (likely by close()). Fall through.
        } catch (const std::exception& e) {
            spdlog::warn("status_reporter: timer wait failed: {}", e.what());
        }

        if (closed_) break;

        std::string line;
        line.reserve(128);
        for (std::size_t i = 0; i < sources_.size(); ++i) {
            if (i) line += " | ";
            line += sources_[i].name;
            line += "{";
            line += sources_[i].fn();
            line += "}";
        }
        if (line.empty())
            line = "(no sources registered)";
        spdlog::info("status: {}", line);
    }
}

} // namespace binapi2::examples::async_recorder
