// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file pipeline_order_book.cpp
/// @brief Multi-threaded order book: network → parser → logic pipeline.

#include <binapi2/fapi/order_book/pipeline_order_book.hpp>

#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/streams/stream_connection.hpp>

#include <glaze/glaze.hpp>

#include <algorithm>

namespace binapi2::fapi::order_book {

pipeline_order_book::pipeline_order_book(config cfg, rest::market_data_service& market_data,
                                         std::size_t buffer_size) :
    cfg_(std::move(cfg)), market_data_(market_data), buffer_size_(buffer_size)
{
}

void pipeline_order_book::stop() { running_ = false; }

order_book_snapshot pipeline_order_book::snapshot() const
{
    std::lock_guard lock(mutex_);
    return book_;
}

void pipeline_order_book::set_snapshot_callback(snapshot_callback cb)
{
    std::lock_guard lock(mutex_);
    on_snapshot_ = std::move(cb);
}

void pipeline_order_book::apply_event(const types::depth_stream_event_t& event)
{
    apply_levels(event.bids, book_.bids);
    apply_levels(event.asks, book_.asks);
    book_.last_update_id = event.final_update_id;
}

template<class Compare>
void pipeline_order_book::apply_levels(const std::vector<types::price_level_t>& levels,
                                       std::map<types::decimal_t, types::decimal_t, Compare>& side)
{
    for (const auto& level : levels) {
        if (level.quantity.is_zero()) {
            side.erase(level.price);
        } else {
            side[level.price] = level.quantity;
        }
    }
}

template void pipeline_order_book::apply_levels(
    const std::vector<types::price_level_t>&,
    std::map<types::decimal_t, types::decimal_t, std::greater<>>&);
template void pipeline_order_book::apply_levels(
    const std::vector<types::price_level_t>&,
    std::map<types::decimal_t, types::decimal_t, std::less<>>&);

boost::cobalt::task<result<void>>
pipeline_order_book::async_run(types::symbol_t symbol, int depth_limit)
{
    running_ = true;

    // -- Stage buffers --
    // raw_buf: network → parser (strings)
    // event_buf: parser → logic (typed events)
    detail::io_thread network_thread;
    detail::io_thread parser_thread;

    auto net_exec = boost::cobalt::executor(boost::asio::require(
        network_thread.context().get_executor(),
        boost::asio::execution::outstanding_work.tracked));
    auto parse_exec = boost::cobalt::executor(boost::asio::require(
        parser_thread.context().get_executor(),
        boost::asio::execution::outstanding_work.tracked));

    detail::threadsafe_stream_buffer<std::string> raw_buf(buffer_size_, net_exec);
    detail::threadsafe_stream_buffer<types::depth_stream_event_t> event_buf(
        buffer_size_, parse_exec);

    // -- Stage 1: Network thread --
    // Connection with a buffer_consumer that pushes raw frames into raw_buf.
    using consumer_type = streams::buffer_consumer<detail::threadsafe_stream_buffer<std::string>>;
    auto network_task = [&]() -> boost::cobalt::task<void> {
        streams::basic_stream_connection<transport::websocket_client, consumer_type> conn(
            cfg_, consumer_type(raw_buf));

        types::diff_book_depth_subscription sub;
        sub.symbol = symbol;
        sub.speed = "100ms";
        auto target = streams::stream_traits<types::diff_book_depth_subscription>::target(cfg_, sub);

        auto cr = co_await conn.async_connect(cfg_.stream_host, cfg_.stream_port, target);
        if (!cr) {
            raw_buf.close();
            co_return;
        }

        while (running_) {
            auto msg = co_await conn.async_read_text();
            if (!msg) break;
            // Consumer already pushed the frame via on_frame()
        }

        conn.consumer().close(); // closes raw_buf
    };

    auto network_future = boost::cobalt::spawn(
        network_thread.context(), network_task(), boost::asio::use_future);

    // -- Stage 2: Parser thread --
    auto parse_fn = [](const std::string& json) -> result<types::depth_stream_event_t> {
        types::depth_stream_event_t event{};
        glz::context ctx{};
        auto ec = glz::read<fapi::detail::json_read_opts>(event, json, ctx);
        if (ec) {
            return result<types::depth_stream_event_t>::failure(
                {error_code::json, 0, 0, glz::format_error(ec, json), json});
        }
        return result<types::depth_stream_event_t>::success(std::move(event));
    };

    streams::stream_parser<types::depth_stream_event_t> parser(raw_buf, event_buf, parse_fn);
    auto parser_future = boost::cobalt::spawn(
        parser_thread.context(), parser.async_run(), boost::asio::use_future);

    // -- Stage 3: Logic (this thread / caller's executor) --
    // Same sync algorithm as local_order_book.

    std::vector<types::depth_stream_event_t> buffer;
    bool synced = false;
    std::uint64_t last_u = 0;

    // Run forwarder for event_buf concurrently with the logic loop.
    auto logic_loop = [&]() -> boost::cobalt::task<void> {
        while (running_) {
            auto ev_result = co_await event_buf.async_read();
            if (!ev_result) break;

            const auto& event = *ev_result;

            if (!synced) {
                buffer.push_back(event);

                if (buffer.size() == 1) {
                    types::order_book_request_t req;
                    req.symbol = symbol;
                    req.limit = depth_limit;
                    auto snap = co_await market_data_.async_execute(req);
                    if (!snap) continue;

                    const auto snapshot_id = snap->lastUpdateId;

                    std::erase_if(buffer, [snapshot_id](const auto& ev) {
                        return ev.final_update_id < snapshot_id;
                    });

                    if (buffer.empty()
                        || buffer.front().first_update_id > snapshot_id
                        || buffer.front().final_update_id < snapshot_id) {
                        buffer.clear();
                        continue;
                    }

                    {
                        std::lock_guard lock(mutex_);
                        book_ = {};
                        book_.last_update_id = snapshot_id;
                        for (const auto& level : snap->bids) {
                            if (!level.quantity.is_zero())
                                book_.bids[level.price] = level.quantity;
                        }
                        for (const auto& level : snap->asks) {
                            if (!level.quantity.is_zero())
                                book_.asks[level.price] = level.quantity;
                        }

                        last_u = snapshot_id;
                        for (const auto& buffered : buffer) {
                            if (buffered.final_update_id <= snapshot_id) {
                                last_u = buffered.final_update_id;
                                continue;
                            }
                            apply_event(buffered);
                            last_u = buffered.final_update_id;
                        }

                        if (on_snapshot_) on_snapshot_(book_);
                    }

                    buffer.clear();
                    synced = true;
                }
            } else {
                if (event.prev_final_update_id != last_u) {
                    synced = false;
                    buffer.clear();
                    buffer.push_back(event);
                    continue;
                }

                {
                    std::lock_guard lock(mutex_);
                    apply_event(event);
                    last_u = event.final_update_id;
                    if (on_snapshot_) on_snapshot_(book_);
                }
            }
        }
    };

    co_await boost::cobalt::join(event_buf.async_forward(), logic_loop());

    // Wait for upstream stages to finish
    parser_future.get();
    network_future.get();

    co_return result<void>::success();
}

} // namespace binapi2::fapi::order_book
