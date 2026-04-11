// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file hopping_stream_buffer.hpp
/// @brief Cross-executor async buffer with backpressure via executor-hopping.

#pragma once

#include <binapi2/fapi/detail/stream_buffer_consumer.hpp>

#include <boost/asio/post.hpp>
#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <cstddef>

namespace binapi2::fapi::detail {

/// @brief Async buffer for cross-executor use with backpressure.
///
/// The consumer must run on the channel's executor. The producer can call
/// async_push from any executor — it hops to the channel's executor to
/// write, then hops back. Two context switches per push.
template<typename T>
class hopping_stream_buffer
    : public stream_buffer_consumer<hopping_stream_buffer<T>, T>
{
    friend class stream_buffer_consumer<hopping_stream_buffer<T>, T>;

public:
    explicit hopping_stream_buffer(std::size_t buffer_size) :
        channel_(buffer_size)
    {
    }

    hopping_stream_buffer(const hopping_stream_buffer&) = delete;
    hopping_stream_buffer& operator=(const hopping_stream_buffer&) = delete;

    /// @brief Push a value. Hops to channel executor, writes, hops back.
    /// Suspends if buffer full (backpressure across executors).
    [[nodiscard]] boost::cobalt::task<void> async_push(T&& value)
    {
        auto caller_exec = co_await boost::cobalt::this_coro::executor;
        auto channel_exec = channel_.get_executor();

        co_await boost::asio::post(channel_exec, boost::cobalt::use_op);
        co_await channel_.write(std::move(value));
        co_await boost::asio::post(caller_exec, boost::cobalt::use_op);
    }

    void close() { channel_.close(); }
    [[nodiscard]] bool is_open() const { return channel_.is_open(); }

private:
    boost::cobalt::channel<T>& channel() { return channel_; }
    const boost::cobalt::channel<T>& channel() const { return channel_; }

    boost::cobalt::channel<T> channel_;
};

} // namespace binapi2::fapi::detail
