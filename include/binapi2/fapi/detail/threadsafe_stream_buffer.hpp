// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file threadsafe_stream_buffer.hpp
/// @brief Cross-thread SPSC async buffer with lock-free push.

#pragma once

#include <binapi2/fapi/detail/stream_buffer_consumer.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/result.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <atomic>
#include <cstddef>

namespace binapi2::fapi::detail {

/// @brief Async buffer for cross-thread use without backpressure.
///
/// Uses a lock-free SPSC queue for the producer-to-consumer bridge.
/// The producer calls push() from its thread (non-coroutine, wait-free).
/// A forwarder coroutine (async_forward) runs on the channel's executor,
/// drains the SPSC queue into the cobalt::channel for consumer use.
///
/// No backpressure on the producer: push returns false if the SPSC queue
/// is full (caller decides whether to drop or retry).
///
/// Single producer, single consumer only.
template<typename T>
class threadsafe_stream_buffer
    : public stream_buffer_consumer<threadsafe_stream_buffer<T>, T>
{
    friend class stream_buffer_consumer<threadsafe_stream_buffer<T>, T>;

public:
    /// @param buffer_size  Size for both the SPSC queue and the cobalt channel.
    explicit threadsafe_stream_buffer(std::size_t buffer_size) :
        channel_(buffer_size),
        spsc_(buffer_size),
        notify_(channel_.get_executor())
    {
        notify_.expires_at(boost::asio::steady_timer::time_point::max());
    }

    threadsafe_stream_buffer(std::size_t buffer_size, boost::cobalt::executor exec) :
        channel_(buffer_size, exec),
        spsc_(buffer_size),
        notify_(exec)
    {
        notify_.expires_at(boost::asio::steady_timer::time_point::max());
    }

    threadsafe_stream_buffer(const threadsafe_stream_buffer&) = delete;
    threadsafe_stream_buffer& operator=(const threadsafe_stream_buffer&) = delete;

    // -- Producer (any thread, wait-free, non-coroutine) --

    /// @brief Push a value from the producer thread. Wait-free.
    /// @return true if enqueued, false if SPSC queue is full.
    bool push(T&& value)
    {
        if (closed_.load(std::memory_order_relaxed)) return false;
        bool ok = spsc_.push(std::move(value));
        if (ok) notify_.cancel();
        return ok;
    }

    void close()
    {
        closed_.store(true, std::memory_order_release);
        notify_.cancel();
    }

    [[nodiscard]] bool is_open() const { return channel_.is_open(); }

    // -- Forwarder (must be co_awaited on the channel's executor) --

    /// @brief Coroutine that drains the SPSC queue into the cobalt channel.
    /// Must run concurrently with the consumer (e.g. via cobalt::join or spawn).
    /// Exits when close() is called and the SPSC queue is drained.
    [[nodiscard]] boost::cobalt::task<void> async_forward()
    {
        while (true) {
            // Wait for notification from producer
            co_await boost::cobalt::as_result(
                notify_.async_wait(boost::cobalt::use_op));

            // Drain SPSC queue into channel
            T item;
            while (spsc_.pop(item)) {
                co_await channel_.write(std::move(item));
            }

            if (closed_.load(std::memory_order_acquire)) {
                // Final drain
                while (spsc_.pop(item)) {
                    co_await channel_.write(std::move(item));
                }
                channel_.close();
                break;
            }

            // Reset timer for next wait
            notify_.expires_at(boost::asio::steady_timer::time_point::max());
        }
    }

private:
    boost::cobalt::channel<T>& channel() { return channel_; }
    const boost::cobalt::channel<T>& channel() const { return channel_; }

    boost::cobalt::channel<T> channel_;
    boost::lockfree::spsc_queue<T> spsc_;
    boost::asio::steady_timer notify_;
    std::atomic<bool> closed_{false};
};

} // namespace binapi2::fapi::detail
