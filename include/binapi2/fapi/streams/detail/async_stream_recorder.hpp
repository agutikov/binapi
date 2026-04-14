// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file async_stream_recorder.hpp
/// @brief Single-executor multi-stream recorder.
///
/// Lightweight recorder that runs entirely on the caller's executor.
/// Uses the single-executor stream_buffer (no cross-thread bridge, no
/// background io_thread). Intended for code that already owns an
/// executor and wants to colocate production, recording, and sinks.

#pragma once

#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/streams/detail/recorder_sink_traits.hpp>

#include <boost/cobalt/join.hpp>
#include <boost/cobalt/result.hpp>
#include <boost/cobalt/task.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Single-executor multi-stream recorder.
///
/// Producers and sinks all run on the executor that was current when
/// add_stream() was called. Producers call `co_await buf.async_push(x)`
/// for natural backpressure — the push coroutine suspends when the
/// buffer is full.
///
/// Lifecycle:
///   basic_async_stream_recorder recorder(1024);
///   auto& buf = recorder.add_stream(my_sink{});
///   cobalt::spawn(exec, recorder.run(), use_future);  // or co_await
///   co_await buf.async_push("frame");
///   recorder.close();                                 // signals end
///
/// @tparam Sink  Callable with signature void(const string&) (sync) or
///               cobalt::task<void>(const string&) (async).
template<typename Sink>
class basic_async_stream_recorder
{
public:
    explicit basic_async_stream_recorder(std::size_t buffer_size_per_stream) :
        buffer_size_(buffer_size_per_stream)
    {
    }

    basic_async_stream_recorder(const basic_async_stream_recorder&) = delete;
    basic_async_stream_recorder& operator=(const basic_async_stream_recorder&) = delete;

    /// @brief Create a buffer and associate a sink. Must be called from the
    /// executor that will own the recorder. Must be called before run().
    fapi::detail::stream_buffer<std::string>& add_stream(Sink sink)
    {
        auto buf = std::make_unique<fapi::detail::stream_buffer<std::string>>(buffer_size_);
        auto& ref = *buf;
        streams_.push_back({std::move(buf), std::move(sink)});
        return ref;
    }

    /// @brief Drain all attached streams until every buffer is closed.
    /// Returns when all drain coroutines have finished.
    boost::cobalt::task<void> run()
    {
        std::vector<boost::cobalt::task<void>> drains;
        drains.reserve(streams_.size());
        for (auto& entry : streams_)
            drains.push_back(async_drain_one(entry));

        co_await boost::cobalt::join(drains);
    }

    /// @brief Close every buffer — signals drain coroutines to exit once
    /// all pending frames have been dispatched.
    void close()
    {
        for (auto& entry : streams_)
            entry.buffer->close();
    }

private:
    struct stream_entry
    {
        std::unique_ptr<fapi::detail::stream_buffer<std::string>> buffer;
        Sink sink;
    };

    boost::cobalt::task<void> async_drain_one(stream_entry& entry)
    {
        // Use the counted async_read() path so the buffer's drained_total
        // observability counter stays in sync with the actual work done.
        while (true) {
            auto rv = co_await entry.buffer->async_read();
            if (!rv) break;

            if constexpr (detail::is_async_sink<Sink>::value) {
                co_await entry.sink(*rv);
            } else {
                entry.sink(*rv);
            }
        }
    }

    std::size_t buffer_size_;
    std::vector<stream_entry> streams_;
};

} // namespace binapi2::fapi::streams
