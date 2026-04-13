// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_consumer.hpp
/// @brief Stream frame consumer concept and built-in implementations.
///
/// A Consumer receives every raw JSON frame from the transport after it is
/// read. It may buffer it, forward it cross-thread, record it, or do nothing.
///
/// Consumer is a compile-time template parameter on stream_connection, like
/// Transport. The default inline_consumer is a zero-overhead no-op.

#pragma once

#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>

#include <boost/cobalt/task.hpp>

#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace binapi2::fapi::streams {

// ---------------------------------------------------------------------------
// Consumer detection traits
// ---------------------------------------------------------------------------

namespace consumer_detail {

/// @brief Detect whether C::on_frame returns cobalt::task<void>.
template<typename C>
concept async_on_frame = requires(C c, std::string s) {
    { c.on_frame(std::move(s)) } -> std::same_as<boost::cobalt::task<void>>;
};

} // namespace consumer_detail

// ---------------------------------------------------------------------------
// Consumer concept
// ---------------------------------------------------------------------------

/// @brief Concept for a stream frame consumer.
///
/// A Consumer receives every raw JSON frame from the transport after it is
/// read. on_frame may return void (sync) or cobalt::task<void> (async).
template<class C>
concept stream_consumer = std::movable<C> &&
    requires(C c, std::string s) {
        { c.on_frame(std::move(s)) };
    } &&
    requires(C c) {
        { c.close() };
    };

// ---------------------------------------------------------------------------
// inline_consumer — default, zero-overhead no-op
// ---------------------------------------------------------------------------

/// @brief Default consumer that does nothing. Zero overhead when optimized.
struct inline_consumer
{
    void on_frame(std::string&&) {}
    void close() {}
};

static_assert(stream_consumer<inline_consumer>);

// ---------------------------------------------------------------------------
// buffer_consumer — pushes frames into a buffer (async)
// ---------------------------------------------------------------------------

/// @brief Consumer that pushes every frame into a buffer via async_push.
///
/// Works with stream_buffer and hopping_stream_buffer (both have async_push).
template<typename Buffer>
class buffer_consumer
{
public:
    explicit buffer_consumer(Buffer& buffer) : buffer_(&buffer) {}

    boost::cobalt::task<void> on_frame(std::string&& frame)
    {
        co_await buffer_->async_push(std::move(frame));
    }

    void close() {}

private:
    Buffer* buffer_;
};

// ---------------------------------------------------------------------------
// buffer_consumer specialization for threadsafe_stream_buffer (sync push)
// ---------------------------------------------------------------------------

/// @brief Consumer that pushes frames into a threadsafe_stream_buffer.
///
/// Uses the non-coroutine push() method — fire-and-forget, never blocks.
template<typename T>
class buffer_consumer<fapi::detail::threadsafe_stream_buffer<T>>
{
public:
    explicit buffer_consumer(fapi::detail::threadsafe_stream_buffer<T>& buffer) :
        buffer_(&buffer)
    {
    }

    void on_frame(std::string&& frame)
    {
        buffer_->push(std::move(frame));
    }

    void close() { buffer_->close(); }

private:
    fapi::detail::threadsafe_stream_buffer<T>* buffer_;
};

// ---------------------------------------------------------------------------
// fan_out_consumer — calls multiple consumers (compile-time)
// ---------------------------------------------------------------------------

/// @brief Consumer that fans out each frame to multiple consumers.
///
/// If any consumer is async, the entire fan_out is async.
template<stream_consumer... Consumers>
class fan_out_consumer
{
public:
    explicit fan_out_consumer(Consumers... consumers) :
        consumers_(std::move(consumers)...)
    {
    }

    auto on_frame(std::string&& frame)
    {
        if constexpr (has_async_consumer()) {
            return on_frame_async(std::move(frame));
        } else {
            on_frame_sync(std::move(frame));
        }
    }

    void close()
    {
        std::apply([](auto&... c) { (c.close(), ...); }, consumers_);
    }

private:
    static constexpr bool has_async_consumer()
    {
        return (consumer_detail::async_on_frame<Consumers> || ...);
    }

    void on_frame_sync(std::string&& frame)
    {
        std::apply([&](auto&... c) { (c.on_frame(std::string(frame)), ...); }, consumers_);
    }

    boost::cobalt::task<void> on_frame_async(std::string&& frame)
    {
        co_await on_frame_async_impl(std::move(frame),
                                     std::index_sequence_for<Consumers...>{});
    }

    template<std::size_t... Is>
    boost::cobalt::task<void> on_frame_async_impl(std::string&& frame,
                                                  std::index_sequence<Is...>)
    {
        (co_await dispatch_one(std::get<Is>(consumers_), std::string(frame)), ...);
    }

    template<typename C>
    boost::cobalt::task<void> dispatch_one(C& consumer, std::string frame)
    {
        if constexpr (consumer_detail::async_on_frame<C>) {
            co_await consumer.on_frame(std::move(frame));
        } else {
            consumer.on_frame(std::move(frame));
        }
    }

    std::tuple<Consumers...> consumers_;
};

} // namespace binapi2::fapi::streams
