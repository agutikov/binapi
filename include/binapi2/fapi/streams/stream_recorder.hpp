// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_recorder.hpp
/// @brief Multi-stream recorder with its own io_thread.
///
/// Records frames from multiple stream connections into per-stream sinks.
/// Owns a background io_thread; all buffers and the drain coroutine run on it.
/// Uses cobalt::race to drain all streams from a single coroutine.

#pragma once

#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>

#include <boost/cobalt/result.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <boost/asio/use_future.hpp>

#include <cstddef>
#include <future>
#include <memory>
#include <type_traits>
#include <vector>

namespace binapi2::fapi::streams {

namespace recorder_detail {

/// @brief Detect whether Sink::operator()(const string&) returns cobalt::task<void>.
template<typename Sink, typename = void>
struct is_async_sink : std::false_type
{
};

template<typename Sink>
struct is_async_sink<
    Sink,
    std::void_t<decltype(std::declval<Sink>()(std::declval<const std::string&>()))>>
    : std::is_same<
          boost::cobalt::task<void>,
          decltype(std::declval<Sink>()(std::declval<const std::string&>()))>
{
};

} // namespace recorder_detail

/// @brief Multi-stream recorder parameterized on sink type.
///
/// Each attached stream gets a hopping_stream_buffer created on the recorder's
/// io_thread executor. The connection pushes frames cross-executor with
/// backpressure. A single drain coroutine uses cobalt::race to multiplex
/// all streams and dispatch to per-stream sinks.
///
/// Usage:
///   stream_recorder recorder(4096);
///   auto& buf = recorder.add_stream([&](const std::string& s) { ... });
///   conn.attach_buffer(buf);
///   recorder.start();
///   // ... use stream ...
///   recorder.stop();
///
/// @tparam Sink  Callable with signature void(const string&) (sync) or
///               cobalt::task<void>(const string&) (async).
template<typename Sink>
class basic_stream_recorder
{
public:
    explicit basic_stream_recorder(std::size_t buffer_size_per_stream) :
        buffer_size_(buffer_size_per_stream)
    {
    }

    ~basic_stream_recorder() { stop(); }

    basic_stream_recorder(const basic_stream_recorder&) = delete;
    basic_stream_recorder& operator=(const basic_stream_recorder&) = delete;

    /// @brief Create a buffer on the recorder's executor and associate a sink.
    /// @return Reference to the buffer — pass to connection.attach_buffer().
    /// Must be called before start().
    fapi::detail::threadsafe_stream_buffer<std::string>& add_stream(Sink sink)
    {
        auto exec = boost::asio::require(
            io_.context().get_executor(),
            boost::asio::execution::outstanding_work.tracked);
        auto buf = std::make_unique<fapi::detail::threadsafe_stream_buffer<std::string>>(
            buffer_size_, boost::cobalt::executor(exec));
        auto& ref = *buf;
        streams_.push_back({std::move(buf), std::move(sink)});
        return ref;
    }

    /// @brief Access the recorder's io_context (e.g. for creating file_sinks).
    [[nodiscard]] boost::asio::io_context& io_context() noexcept { return io_.context(); }

    /// @brief Start forwarder + drain coroutines on the background thread.
    /// One forwarder and one drain per stream.
    void start()
    {
        for (auto& entry : streams_) {
            futures_.push_back(
                boost::cobalt::spawn(io_.context(), entry.buffer->async_forward(),
                                     boost::asio::use_future));
            futures_.push_back(
                boost::cobalt::spawn(io_.context(), async_drain_one(entry),
                                     boost::asio::use_future));
        }
    }

    /// @brief Stop recording — close all buffers, wait for everything to finish.
    ///
    /// threadsafe_stream_buffer::close() is thread-safe (atomic + post),
    /// so it can be called from any thread.
    void stop()
    {
        for (auto& entry : streams_) {
            entry.buffer->close();
        }
        for (auto& f : futures_) {
            if (f.valid()) f.get();
        }
    }

private:
    struct stream_entry
    {
        std::unique_ptr<fapi::detail::threadsafe_stream_buffer<std::string>> buffer;
        Sink sink;
    };

    /// @brief Drain a single stream until its channel is closed and empty.
    boost::cobalt::task<void> async_drain_one(stream_entry& entry)
    {
        while (true) {
            auto rv = co_await boost::cobalt::as_result(
                entry.buffer->reader());
            if (!rv) break;

            if constexpr (recorder_detail::is_async_sink<Sink>::value) {
                co_await entry.sink(*rv);
            } else {
                entry.sink(*rv);
            }
        }
    }

    fapi::detail::io_thread io_;
    std::size_t buffer_size_;
    std::vector<stream_entry> streams_;
    std::vector<std::future<void>> futures_;
};

} // namespace binapi2::fapi::streams
