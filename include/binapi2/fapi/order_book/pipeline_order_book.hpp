// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file pipeline_order_book.hpp
/// @brief Multi-threaded order book with dedicated network, parser, and logic threads.
///
/// Three-stage pipeline:
///   1. Network thread: WebSocket read → raw JSON strings
///   2. Parser thread: JSON parse → typed depth_stream_event_t
///   3. Logic thread (caller): sync algorithm + book maintenance
///
/// Uses threadsafe_stream_buffer<T> (SPSC) between each pair of stages.

#pragma once

#include <binapi2/fapi/config.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <binapi2/fapi/detail/threadsafe_stream_buffer.hpp>
#include <binapi2/fapi/order_book/local_order_book.hpp>
#include <binapi2/fapi/rest/services/market_data.hpp>
#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/streams/detail/stream_consumer.hpp>
#include <binapi2/fapi/streams/detail/stream_parser.hpp>
#include <binapi2/fapi/transport/websocket_client.hpp>
#include <binapi2/fapi/types/market_stream_events.hpp>
#include <binapi2/fapi/types/subscriptions.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>

namespace binapi2::fapi::order_book {

/// @brief Multi-threaded order book with 3-stage pipeline.
///
/// Usage:
///   auto rest = co_await api.create_rest_client();
///   pipeline_order_book book(api.configuration(), (*rest)->market_data, 4096);
///   book.set_snapshot_callback([](const auto& snap) { ... });
///   auto r = co_await book.async_run(symbol, depth_limit);
class pipeline_order_book
{
public:
    using snapshot_callback = std::function<void(const order_book_snapshot&)>;

    pipeline_order_book(config cfg, rest::market_data_service& market_data,
                        std::size_t buffer_size);

    /// @brief Run the 3-stage pipeline. Blocks on the caller's executor
    ///        until stop() is called or an error occurs.
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_run(types::symbol_t symbol, int depth_limit);

    /// @brief Signal the pipeline to stop.
    void stop();

    /// @brief Thread-safe snapshot of current book state.
    [[nodiscard]] order_book_snapshot snapshot() const;

    void set_snapshot_callback(snapshot_callback cb);

    /// @brief Attach a record buffer to capture raw WebSocket frames.
    /// Must be called before async_run().
    void set_record_buffer(detail::threadsafe_stream_buffer<std::string>& buffer);

private:
    void apply_event(const types::depth_stream_event_t& event);

    template<class Compare>
    void apply_levels(const types::depth_levels_t& levels,
                      std::map<types::decimal_t, types::decimal_t, Compare>& side);

    config cfg_;
    rest::market_data_service& market_data_;
    std::size_t buffer_size_;

    mutable std::mutex mutex_;
    order_book_snapshot book_;
    snapshot_callback on_snapshot_;
    std::atomic<bool> running_{false};
    detail::threadsafe_stream_buffer<std::string>* record_buffer_{nullptr};
};

} // namespace binapi2::fapi::order_book
