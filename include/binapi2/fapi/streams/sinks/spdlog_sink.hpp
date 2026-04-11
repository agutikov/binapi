// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file spdlog_sink.hpp
/// @brief spdlog-based sink for stream_recorder.

#pragma once

#include <binapi2/fapi/streams/stream_recorder.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace binapi2::fapi::streams::sinks {

/// @brief Sync sink that logs frames via an spdlog async logger.
///
/// spdlog's own thread pool handles the actual file I/O, so calling
/// logger->info() is non-blocking. Useful when spdlog infrastructure
/// is already set up (e.g. the demo CLI).
class spdlog_sink
{
public:
    explicit spdlog_sink(std::shared_ptr<spdlog::logger> logger) :
        logger_(std::move(logger))
    {
    }

    void operator()(const std::string& frame) { logger_->info("{}", frame); }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace binapi2::fapi::streams::sinks

namespace binapi2::fapi::streams {

/// @brief Stream recorder with spdlog sink.
using spdlog_stream_recorder = basic_stream_recorder<sinks::spdlog_sink>;

} // namespace binapi2::fapi::streams
