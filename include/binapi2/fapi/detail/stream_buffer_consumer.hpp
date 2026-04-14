// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_buffer_consumer.hpp
/// @brief CRTP mixin providing the consumer interface for stream buffers.
///
/// Shared by stream_buffer, hopping_stream_buffer, and threadsafe_stream_buffer.

#pragma once

#include <binapi2/fapi/result.hpp>

#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/result.hpp>
#include <boost/cobalt/task.hpp>

#include <cstddef>
#include <functional>
#include <vector>

namespace binapi2::fapi::detail {

/// @brief CRTP mixin providing the consumer interface over a cobalt::channel<T>.
///
/// Derived class must provide: channel() returning cobalt::channel<T>&.
template<typename Derived, typename T>
class stream_buffer_consumer
{
public:
    /// @brief Read a single item. Returns failure when channel is closed.
    [[nodiscard]] boost::cobalt::task<result<T>> async_read()
    {
        auto rv = co_await boost::cobalt::as_result(chan().read());
        if (!rv)
            co_return result<T>::failure({error_code::websocket, 0, 0, "channel closed", {}});
        ++drained_total_;
        co_return result<T>::success(std::move(*rv));
    }

    /// @brief Read up to max_count items. Waits for at least one.
    [[nodiscard]] boost::cobalt::task<result<std::vector<T>>>
    async_read_batch(std::size_t max_count)
    {
        auto first = co_await boost::cobalt::as_result(chan().read());
        if (!first)
            co_return result<std::vector<T>>::failure(
                {error_code::websocket, 0, 0, "channel closed", {}});

        std::vector<T> batch;
        batch.reserve(max_count);
        batch.push_back(std::move(*first));
        ++drained_total_;

        while (batch.size() < max_count && chan().is_open()) {
            auto op = chan().read();
            if (!op.await_ready()) break;
            auto rv = op.await_resume(boost::cobalt::as_result_tag{});
            if (!rv) break;
            batch.push_back(std::move(*rv));
            ++drained_total_;
        }

        co_return result<std::vector<T>>::success(std::move(batch));
    }

    /// @brief Drain loop — reads items and calls sink until channel is
    ///        closed and all buffered items are consumed.
    [[nodiscard]] boost::cobalt::task<void>
    async_drain(std::function<void(const T&)> sink)
    {
        while (true) {
            auto rv = co_await boost::cobalt::as_result(chan().read());
            if (!rv) break;
            ++drained_total_;
            sink(*rv);
        }
    }

    /// @brief Cumulative number of items drained through the consumer
    /// interface. Incremented by `async_read`, `async_read_batch`, and
    /// `async_drain`; the raw `reader()` path bypasses this counter.
    [[nodiscard]] std::size_t drained_total() const noexcept
    {
        return drained_total_;
    }

    /// @brief Return a channel_reader for use with cobalt::race.
    [[nodiscard]] boost::cobalt::channel_reader<T> reader()
    {
        return boost::cobalt::channel_reader<T>(chan());
    }

    [[nodiscard]] auto get_executor() const { return chan().get_executor(); }

private:
    boost::cobalt::channel<T>& chan() { return static_cast<Derived*>(this)->channel(); }
    const boost::cobalt::channel<T>& chan() const
    {
        return static_cast<const Derived*>(this)->channel();
    }

    std::size_t drained_total_{ 0 };
};

} // namespace binapi2::fapi::detail
