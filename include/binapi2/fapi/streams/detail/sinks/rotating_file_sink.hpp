// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file rotating_file_sink.hpp
/// @brief Rotating async file sink: size/time rollover, fsync, zstd compression.
///
/// A drop-in replacement for `file_sink` that:
///   - rolls a new segment when the current file exceeds `max_size_bytes`
///     or its age exceeds `max_seconds`;
///   - `fsync(2)`s the outgoing segment before closing it (asio::stream_file
///     `close()` is only `close(2)`, which does NOT imply a sync);
///   - hands the closed segment to `zstd --rm` via `posix_spawnp(3)` so
///     compression runs out-of-process without blocking the recorder;
///   - reaps finished child processes lazily on each rotation;
///   - on destruction, fsync+close the current segment, spawn the final
///     compressor, and best-effort reap all children.
///
/// Requires BOOST_ASIO_HAS_FILE (io_uring) like `file_sink`.

#pragma once

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_HAS_FILE)
#error "rotating_file_sink requires BOOST_ASIO_HAS_FILE (BOOST_ASIO_HAS_IO_URING + liburing)"
#endif

#include <binapi2/fapi/streams/detail/async_stream_recorder.hpp>
#include <binapi2/fapi/streams/detail/threaded_stream_recorder.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/write.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/task.hpp>

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

extern char** environ;

namespace binapi2::fapi::streams::sinks {

/// @brief Configuration for rotating_file_sink.
struct rotating_file_sink_config
{
    std::filesystem::path dir{};                  ///< output directory (must exist)
    std::string basename{};                       ///< segment filename prefix
    std::string extension{ ".jsonl" };
    std::size_t max_size_bytes{ 100ULL * 1024 * 1024 };  ///< 100 MiB
    std::uint64_t max_seconds{ 15*60 };                    ///< 15m; 0 disables time rotation
    bool compress{ true };                        ///< spawn `zstd --rm` on rotate
};

/// @brief Rotating, fsync'ing, compressing async file sink.
///
/// Matches the sink contract required by `basic_async_stream_recorder` and
/// `basic_threaded_stream_recorder` — `operator()(const std::string&)` returns
/// `cobalt::task<void>`.
///
/// The sink is moveable but not copyable. Once installed in a recorder it is
/// not touched from the outside; all state mutation happens inside the drain
/// coroutine on the recorder's executor.
class rotating_file_sink
{
public:
    rotating_file_sink(boost::asio::io_context& ctx, rotating_file_sink_config cfg) :
        ctx_(&ctx), cfg_(std::move(cfg))
    {
        open_new_segment();
    }

    rotating_file_sink(const rotating_file_sink&) = delete;
    rotating_file_sink& operator=(const rotating_file_sink&) = delete;

    rotating_file_sink(rotating_file_sink&& other) noexcept :
        ctx_(other.ctx_),
        cfg_(std::move(other.cfg_)),
        file_(std::move(other.file_)),
        current_path_(std::move(other.current_path_)),
        current_bytes_(other.current_bytes_),
        segment_open_at_(other.segment_open_at_),
        rotations_(other.rotations_),
        children_(std::move(other.children_))
    {
        other.ctx_ = nullptr;
        other.current_bytes_ = 0;
        other.rotations_ = 0;
    }

    rotating_file_sink& operator=(rotating_file_sink&&) = delete;

    ~rotating_file_sink()
    {
        if (!ctx_) return;  // moved-from
        // Best-effort shutdown: sync + close the current segment and kick off
        // a final compressor. Run synchronously — destructor is not a coroutine.
        try {
            if (file_) {
                ::fsync(file_->native_handle());
                file_->close();
                file_.reset();
                if (cfg_.compress && !current_path_.empty() &&
                    std::filesystem::exists(current_path_))
                    spawn_zstd(current_path_);
            }
            reap_children_blocking();
        } catch (...) {
            // Never throw from destructor.
        }
    }

    /// @brief Sink entry point. Writes `frame + '\n'` to the current segment,
    ///        rotating first if the size/time thresholds would be exceeded.
    boost::cobalt::task<void> operator()(const std::string& frame)
    {
        std::string line = frame;
        line.push_back('\n');

        if (should_rotate(line.size()))
            rotate();

        co_await boost::asio::async_write(
            *file_, boost::asio::buffer(line), boost::cobalt::use_op);

        current_bytes_ += line.size();
    }

    // -- Stats accessors (for stats_run) -------------------------------------

    [[nodiscard]] std::size_t current_bytes() const noexcept { return current_bytes_; }
    [[nodiscard]] std::size_t rotations() const noexcept { return rotations_; }
    [[nodiscard]] const std::filesystem::path& current_path() const noexcept
    {
        return current_path_;
    }

private:
    // -- Rotation triggers ---------------------------------------------------

    [[nodiscard]] bool should_rotate(std::size_t pending) const
    {
        if (!file_) return true;
        if (current_bytes_ + pending > cfg_.max_size_bytes) return true;
        if (cfg_.max_seconds > 0) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - segment_open_at_)
                           .count();
            if (static_cast<std::uint64_t>(age) >= cfg_.max_seconds) return true;
        }
        return false;
    }

    // -- Rotation primitive --------------------------------------------------

    void rotate()
    {
        if (file_) {
            // asio::stream_file::close() is close(2), not fsync(2). Force
            // durable-on-disk before compressing so a crash between close()
            // and zstd cannot yield a corrupted .zst.
            ::fsync(file_->native_handle());
            file_->close();
            auto finished = current_path_;
            file_.reset();
            ++rotations_;
            if (cfg_.compress) spawn_zstd(finished);
            reap_children_nonblocking();
        }
        open_new_segment();
    }

    void open_new_segment()
    {
        current_path_ = cfg_.dir / (cfg_.basename + "." + now_timestamp() +
                                    "." + zero_pad(rotations_, 4) + cfg_.extension);

        file_ = std::make_shared<boost::asio::stream_file>(
            *ctx_,
            current_path_.string(),
            boost::asio::stream_file::write_only
                | boost::asio::stream_file::create
                | boost::asio::stream_file::truncate);
        current_bytes_ = 0;
        segment_open_at_ = std::chrono::steady_clock::now();
    }

    // -- Timestamp / formatting ---------------------------------------------

    static std::string now_timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count() %
                  1000;
        std::tm tm{};
        ::gmtime_r(&t, &tm);
        char buf[48];
        std::snprintf(buf, sizeof(buf),
                      "%04d%02d%02dT%02d%02d%02d.%03ldZ",
                      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                      tm.tm_hour, tm.tm_min, tm.tm_sec,
                      static_cast<long>(ms));
        return buf;
    }

    static std::string zero_pad(std::size_t n, int width)
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%0*zu", width, n);
        return buf;
    }

    // -- zstd compression hand-off ------------------------------------------

    void spawn_zstd(const std::filesystem::path& path)
    {
        // Silent, remove-input, replace with .zst. Runs out-of-process so the
        // recorder's executor is unblocked immediately.
        std::string arg_path = path.string();
        char arg0[] = "zstd";
        char arg1[] = "-q";
        char arg2[] = "--rm";
        std::vector<char> arg3(arg_path.begin(), arg_path.end());
        arg3.push_back('\0');
        char* argv[] = { arg0, arg1, arg2, arg3.data(), nullptr };

        ::posix_spawn_file_actions_t fa;
        ::posix_spawn_file_actions_init(&fa);
        ::pid_t pid = -1;
        int rc = ::posix_spawnp(&pid, "zstd", &fa, nullptr, argv, environ);
        ::posix_spawn_file_actions_destroy(&fa);

        if (rc == 0 && pid > 0) children_.push_back(pid);
        // On failure, leave the segment uncompressed; the recorder keeps going.
    }

    void reap_children_nonblocking()
    {
        for (auto it = children_.begin(); it != children_.end();) {
            int status = 0;
            ::pid_t r = ::waitpid(*it, &status, WNOHANG);
            if (r == *it)
                it = children_.erase(it);
            else
                ++it;
        }
    }

    void reap_children_blocking()
    {
        for (::pid_t pid : children_) {
            int status = 0;
            ::waitpid(pid, &status, 0);
        }
        children_.clear();
    }

    // -- State ---------------------------------------------------------------

    boost::asio::io_context* ctx_{ nullptr };
    rotating_file_sink_config cfg_{};
    std::shared_ptr<boost::asio::stream_file> file_{};
    std::filesystem::path current_path_{};
    std::size_t current_bytes_{ 0 };
    std::chrono::steady_clock::time_point segment_open_at_{};
    std::size_t rotations_{ 0 };
    std::vector<::pid_t> children_{};
};

} // namespace binapi2::fapi::streams::sinks

namespace binapi2::fapi::streams {

/// @brief Threaded recorder with rotating file sink.
using threaded_rotating_file_stream_recorder =
    basic_threaded_stream_recorder<sinks::rotating_file_sink>;

/// @brief Async (single-executor) recorder with rotating file sink.
using async_rotating_file_stream_recorder =
    basic_async_stream_recorder<sinks::rotating_file_sink>;

} // namespace binapi2::fapi::streams
