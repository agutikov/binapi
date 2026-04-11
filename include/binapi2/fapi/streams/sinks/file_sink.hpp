// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file file_sink.hpp
/// @brief Async file sink for stream_recorder using boost::asio::stream_file.
///
/// Requires BOOST_ASIO_HAS_IO_URING and liburing.

#pragma once

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_HAS_FILE)
#error "file_sink requires BOOST_ASIO_HAS_FILE (define BOOST_ASIO_HAS_IO_URING and link liburing)"
#endif

#include <binapi2/fapi/streams/stream_recorder.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/write.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/task.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace binapi2::fapi::streams::sinks {

/// @brief Async sink that writes JSONL frames to a file via asio::stream_file.
///
/// Each frame is written as one line (frame + '\n'). The write is truly async
/// via the recorder's io_context.
class file_sink
{
public:
    file_sink(boost::asio::io_context& ctx, const std::filesystem::path& path) :
        file_(std::make_shared<boost::asio::stream_file>(
            ctx,
            path.string(),
            boost::asio::stream_file::write_only
                | boost::asio::stream_file::create
                | boost::asio::stream_file::truncate))
    {
    }

    boost::cobalt::task<void> operator()(const std::string& frame)
    {
        std::string line = frame + '\n';
        co_await boost::asio::async_write(
            *file_, boost::asio::buffer(line), boost::cobalt::use_op);
    }

private:
    std::shared_ptr<boost::asio::stream_file> file_;
};

} // namespace binapi2::fapi::streams::sinks

namespace binapi2::fapi::streams {

/// @brief Stream recorder with async file sink.
using file_stream_recorder = basic_stream_recorder<sinks::file_sink>;

} // namespace binapi2::fapi::streams
