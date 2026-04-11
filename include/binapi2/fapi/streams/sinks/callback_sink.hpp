// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file callback_sink.hpp
/// @brief Generic callback sink for stream_recorder.

#pragma once

#include <binapi2/fapi/streams/stream_recorder.hpp>

#include <functional>
#include <string>

namespace binapi2::fapi::streams::sinks {

/// @brief Sync sink that forwards frames to a user-provided callback.
class callback_sink
{
public:
    using callback_fn = std::function<void(const std::string&)>;

    explicit callback_sink(callback_fn fn) : fn_(std::move(fn)) {}

    void operator()(const std::string& frame) { fn_(frame); }

private:
    callback_fn fn_;
};

} // namespace binapi2::fapi::streams::sinks

namespace binapi2::fapi::streams {

/// @brief Stream recorder with a generic callback sink.
using stream_recorder = basic_stream_recorder<sinks::callback_sink>;

} // namespace binapi2::fapi::streams
