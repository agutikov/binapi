// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_buffer.hpp
/// @brief Single-executor async buffer wrapping cobalt::channel<T>.

#pragma once

#include <binapi2/fapi/detail/stream_buffer_consumer.hpp>

#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/task.hpp>

#include <cstddef>

namespace binapi2::fapi::detail {

/// @brief Async buffer for use within a single executor.
///
/// Both producer and consumer must run on the executor that was current
/// when the buffer was constructed. No synchronization overhead.
template<typename T>
class stream_buffer : public stream_buffer_consumer<stream_buffer<T>, T>
{
    friend class stream_buffer_consumer<stream_buffer<T>, T>;

public:
    explicit stream_buffer(std::size_t buffer_size) :
        channel_(buffer_size)
    {
    }

    stream_buffer(const stream_buffer&) = delete;
    stream_buffer& operator=(const stream_buffer&) = delete;

    /// @brief Push a value. Suspends if buffer full (backpressure).
    /// Must be called from the channel's executor.
    [[nodiscard]] boost::cobalt::task<void> async_push(T&& value)
    {
        co_await channel_.write(std::move(value));
    }

    void close() { channel_.close(); }
    [[nodiscard]] bool is_open() const { return channel_.is_open(); }

private:
    boost::cobalt::channel<T>& channel() { return channel_; }
    const boost::cobalt::channel<T>& channel() const { return channel_; }

    boost::cobalt::channel<T> channel_;
};

} // namespace binapi2::fapi::detail
