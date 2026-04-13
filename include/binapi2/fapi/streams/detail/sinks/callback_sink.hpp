// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file callback_sink.hpp
/// @brief Generic callback sink for stream_recorder.

#pragma once

#include <binapi2/fapi/streams/detail/async_stream_recorder.hpp>
#include <binapi2/fapi/streams/detail/threaded_stream_recorder.hpp>

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

/// @brief Threaded recorder with a generic callback sink.
using threaded_stream_recorder = basic_threaded_stream_recorder<sinks::callback_sink>;

/// @brief Async (single-executor) recorder with a generic callback sink.
using async_stream_recorder = basic_async_stream_recorder<sinks::callback_sink>;

} // namespace binapi2::fapi::streams
