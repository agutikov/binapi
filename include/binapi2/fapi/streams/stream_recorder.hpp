// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_recorder.hpp
/// @brief Async stream frame recorder using cobalt::channel.
///
/// Non-blocking from the producer side when the buffer has space.
/// Applies backpressure (suspends the producer) when the buffer is full.
/// Frames are drained by an async coroutine that writes to a user-provided sink.

#pragma once

#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/task.hpp>

#include <functional>
#include <string>

namespace binapi2::fapi::streams {

/// @brief Async stream frame recorder.
///
/// Wraps a cobalt::channel<string> with a configurable sink. The producer
/// calls co_await async_record(frame) to push frames. The consumer runs
/// async_drain() which calls the sink for each frame.
///
/// Usage:
///   stream_recorder recorder(1024, [&file](const std::string& s) {
///       file << s << '\n';
///   });
///
///   // Set on stream_connection:
///   conn.set_recorder(recorder);
///
///   // Run drain concurrently with the stream consumer:
///   co_await cobalt::join(
///       recorder.async_drain(),
///       consume_stream(conn)
///   );
class stream_recorder
{
public:
    using sink_fn = std::function<void(const std::string&)>;

    /// @param buffer_size  Maximum number of frames buffered before backpressure.
    /// @param sink         Callback invoked for each frame (from drain coroutine).
    stream_recorder(std::size_t buffer_size, sink_fn sink) :
        channel_(buffer_size), sink_(std::move(sink))
    {
    }

    stream_recorder(const stream_recorder&) = delete;
    stream_recorder& operator=(const stream_recorder&) = delete;

    /// @brief Push a frame to the recorder. Suspends if buffer is full (backpressure).
    boost::cobalt::task<void> async_record(std::string frame)
    {
        co_await channel_.write(std::move(frame));
    }

    /// @brief Drain coroutine — reads frames from the channel and calls the sink.
    ///
    /// Runs until the channel is closed. Must be co_awaited concurrently with
    /// the stream consumer (e.g. via cobalt::join or cobalt::spawn).
    boost::cobalt::task<void> async_drain()
    {
        while (channel_.is_open()) {
            auto frame = co_await channel_.read();
            sink_(frame);
        }
    }

    /// @brief Close the channel. Causes async_drain to finish after remaining frames.
    void close() { channel_.close(); }

private:
    boost::cobalt::channel<std::string> channel_;
    sink_fn sink_;
};

} // namespace binapi2::fapi::streams
