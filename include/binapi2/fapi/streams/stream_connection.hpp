// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file stream_connection.hpp
/// @brief Raw WebSocket stream connection — owns the transport, manages
///        connect/close, produces raw JSON frames. Protocol-agnostic.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/hopping_stream_buffer.hpp>
#include <binapi2/fapi/detail/stream_buffer.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/stream_consumer.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/transport/websocket_transport.hpp>

#include <boost/cobalt/task.hpp>

#include <glaze/glaze.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace binapi2::fapi::streams {

/// @brief Type-erased push target for stream buffers.
///
/// Allows stream_connection to push frames into any buffer type
/// (stream_buffer, hopping_stream_buffer, threadsafe_stream_buffer)
/// without knowing the concrete type.
class frame_push_target
{
public:
    virtual ~frame_push_target() = default;
    virtual boost::cobalt::task<void> async_push(std::string&& frame) = 0;
};

/// @brief Adapter that wraps a buffer with async_push into a frame_push_target.
template<typename Buffer>
class frame_push_adapter final : public frame_push_target
{
public:
    explicit frame_push_adapter(Buffer& buffer) : buffer_(buffer) {}
    boost::cobalt::task<void> async_push(std::string&& frame) override
    {
        co_await buffer_.async_push(std::move(frame));
    }

private:
    Buffer& buffer_;
};

/// @brief Adapter for threadsafe_stream_buffer which uses non-coroutine push().
template<typename T>
class frame_push_adapter<fapi::detail::threadsafe_stream_buffer<T>> final
    : public frame_push_target
{
public:
    explicit frame_push_adapter(fapi::detail::threadsafe_stream_buffer<T>& buffer) :
        buffer_(buffer)
    {
    }
    boost::cobalt::task<void> async_push(std::string&& frame) override
    {
        buffer_.push(std::move(frame));
        co_return;
    }

private:
    fapi::detail::threadsafe_stream_buffer<T>& buffer_;
};

namespace detail {

/// @brief Control message for combined stream SUBSCRIBE/UNSUBSCRIBE/LIST_SUBSCRIPTIONS.
struct stream_control_request
{
    std::string method{};
    std::vector<std::string> params{};
    unsigned int id{};
};

} // namespace detail

} // namespace binapi2::fapi::streams

template<>
struct glz::meta<binapi2::fapi::streams::detail::stream_control_request>
{
    using T = binapi2::fapi::streams::detail::stream_control_request;
    static constexpr auto value = object("method", &T::method, "params", &T::params, "id", &T::id);
};

namespace binapi2::fapi::streams {

/// @brief Raw WebSocket stream connection.
///
/// Owns the WebSocket transport. Manages connect/close and raw frame I/O.
/// Knows nothing about Binance stream protocol, subscriptions, or event types.
///
/// Two frame interception mechanisms:
///   - Consumer template param (compile-time, zero-overhead for inline_consumer)
///   - attach_buffer() (runtime, dynamic attach/detach)
///
/// @tparam Transport WebSocket transport type (default: transport::websocket_client).
/// @tparam Consumer  Frame consumer type (default: inline_consumer — no-op).
template<transport::websocket_transport Transport = transport::websocket_client,
         stream_consumer Consumer = inline_consumer>
class basic_stream_connection
{
public:
    explicit basic_stream_connection(config cfg) :
        transport_(cfg), cfg_(std::move(cfg))
    {
    }

    basic_stream_connection(config cfg, Consumer consumer) :
        transport_(cfg), cfg_(std::move(cfg)), consumer_(std::move(consumer))
    {
    }

    basic_stream_connection(const basic_stream_connection&) = delete;
    basic_stream_connection& operator=(const basic_stream_connection&) = delete;

    // -- Buffer --

    /// @brief Attach any stream buffer type to receive copies of all incoming frames.
    template<typename Buffer>
    void attach_buffer(Buffer& buffer)
    {
        push_target_ = std::make_unique<frame_push_adapter<Buffer>>(buffer);
    }

    /// @brief Detach the stream buffer.
    void detach_buffer() { push_target_.reset(); }

    // -- Connection --

    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect(std::string host, std::string port, ws_target_t target)
    {
        co_return co_await transport_.async_connect(std::move(host), std::move(port), std::move(target));
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_close()
    {
        co_return co_await transport_.async_close();
    }

    // -- Raw frame I/O --

    [[nodiscard]] boost::cobalt::task<result<std::string>> async_read_text()
    {
        auto msg = co_await transport_.async_read_text();
        if (msg) {
            // Compile-time consumer (zero-overhead for inline_consumer)
            if constexpr (consumer_detail::async_on_frame<Consumer>) {
                co_await consumer_.on_frame(std::string(*msg));
            } else {
                consumer_.on_frame(std::string(*msg));
            }
            // Runtime-attached buffer (optional recording)
            if (push_target_) {
                co_await push_target_->async_push(std::string(*msg));
            }
        }
        co_return msg;
    }

    [[nodiscard]] boost::cobalt::task<result<void>> async_write_text(std::string message)
    {
        co_return co_await transport_.async_write_text(std::move(message));
    }

    // -- Accessors --

    [[nodiscard]] const config& configuration() const noexcept { return cfg_; }
    [[nodiscard]] Transport& transport() noexcept { return transport_; }
    [[nodiscard]] Consumer& consumer() noexcept { return consumer_; }

private:
    Transport transport_;
    config cfg_;
    Consumer consumer_;
    std::unique_ptr<frame_push_target> push_target_;
};

/// @brief Default stream connection using the WebSocket transport.
using stream_connection = basic_stream_connection<>;

} // namespace binapi2::fapi::streams
