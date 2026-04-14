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
///
/// Maintains lightweight observability counters (pushed / occupancy /
/// high-water / push-suspends) readable from the same thread at any
/// time. The drained counter lives on the consumer base.
template<typename T>
class stream_buffer : public stream_buffer_consumer<stream_buffer<T>, T>
{
    friend class stream_buffer_consumer<stream_buffer<T>, T>;

public:
    explicit stream_buffer(std::size_t buffer_size) :
        channel_(buffer_size),
        capacity_(buffer_size)
    {
    }

    stream_buffer(const stream_buffer&) = delete;
    stream_buffer& operator=(const stream_buffer&) = delete;

    /// @brief Push a value. Suspends if buffer full (backpressure).
    /// Must be called from the channel's executor.
    [[nodiscard]] boost::cobalt::task<void> async_push(T&& value)
    {
        // Track backpressure: if the buffer is already at capacity, this
        // push will suspend.
        if (occupancy() >= capacity_)
            ++push_suspends_total_;

        co_await channel_.write(std::move(value));
        ++pushed_total_;
        const auto occ = occupancy();
        if (occ > high_water_) high_water_ = occ;
    }

    void close() { channel_.close(); }
    [[nodiscard]] bool is_open() const { return channel_.is_open(); }

    // -- Observability counters ---------------------------------------------

    [[nodiscard]] std::size_t pushed_total() const noexcept { return pushed_total_; }
    [[nodiscard]] std::size_t push_suspends_total() const noexcept
    {
        return push_suspends_total_;
    }
    [[nodiscard]] std::size_t high_water() const noexcept { return high_water_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

    /// @brief Current in-flight (pushed minus drained). Safe to call
    /// from the same executor that owns the buffer.
    [[nodiscard]] std::size_t occupancy() const noexcept
    {
        const auto pushed = pushed_total_;
        const auto drained = this->drained_total();
        return pushed >= drained ? (pushed - drained) : 0;
    }

private:
    boost::cobalt::channel<T>& channel() { return channel_; }
    const boost::cobalt::channel<T>& channel() const { return channel_; }

    boost::cobalt::channel<T> channel_;
    std::size_t capacity_{ 0 };
    std::size_t pushed_total_{ 0 };
    std::size_t push_suspends_total_{ 0 };
    std::size_t high_water_{ 0 };
};

} // namespace binapi2::fapi::detail
