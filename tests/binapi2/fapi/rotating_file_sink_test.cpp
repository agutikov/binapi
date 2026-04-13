// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for rotating_file_sink — size/time rotation, fsync, zstd handoff.

#include <binapi2/fapi/streams/detail/sinks/rotating_file_sink.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <gtest/gtest.h>

#include <sys/wait.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>

namespace fs = std::filesystem;
namespace streams = binapi2::fapi::streams;
using streams::sinks::rotating_file_sink;
using streams::sinks::rotating_file_sink_config;

namespace {

fs::path unique_tmp_dir(const std::string& label)
{
    auto t = std::chrono::steady_clock::now().time_since_epoch().count();
    auto p = fs::temp_directory_path() /
             ("binapi2_rfs_" + label + "_" + std::to_string(t));
    fs::remove_all(p);
    fs::create_directories(p);
    return p;
}

bool zstd_available()
{
    // `command -v zstd` exits 0 when found.
    return std::system("command -v zstd >/dev/null 2>&1") == 0;
}

boost::cobalt::task<void>
drive_writes(rotating_file_sink_config cfg, std::size_t frames, std::size_t frame_size)
{
    auto exec = co_await boost::cobalt::this_coro::executor;
    auto& ctx = static_cast<boost::asio::io_context&>(
        boost::asio::query(exec, boost::asio::execution::context));

    {
        rotating_file_sink sink(ctx, std::move(cfg));
        std::string payload(frame_size, 'x');
        for (std::size_t i = 0; i < frames; ++i)
            co_await sink(payload);
    }  // sink destructor closes + fsyncs + spawns final zstd
}

} // namespace

// =====================================================================
// Size-based rotation, compression disabled
// =====================================================================

TEST(RotatingFileSink, RotatesOnSize)
{
    auto dir = unique_tmp_dir("size");
    rotating_file_sink_config cfg;
    cfg.dir = dir;
    cfg.basename = "test";
    cfg.max_size_bytes = 200;   // small threshold
    cfg.max_seconds = 0;        // disable time rotation
    cfg.compress = false;

    // 50 frames x 20 chars + '\n' = 21B each → ~5 segments of 9 frames.
    boost::cobalt::run(drive_writes(cfg, 50, 20));

    std::size_t total_size = 0;
    int segments = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        EXPECT_EQ(entry.path().extension(), ".jsonl");
        total_size += fs::file_size(entry.path());
        ++segments;
    }
    EXPECT_GT(segments, 1);
    // Each frame is 20 'x' chars + '\n' = 21 bytes; 50 frames total.
    EXPECT_EQ(total_size, 50u * 21u);

    fs::remove_all(dir);
}

// =====================================================================
// No rotation below threshold
// =====================================================================

TEST(RotatingFileSink, SingleSegmentBelowThreshold)
{
    auto dir = unique_tmp_dir("single");
    rotating_file_sink_config cfg;
    cfg.dir = dir;
    cfg.basename = "small";
    cfg.max_size_bytes = 10000;
    cfg.max_seconds = 0;
    cfg.compress = false;

    boost::cobalt::run(drive_writes(cfg, 10, 20));

    int segments = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        (void)entry;
        ++segments;
    }
    EXPECT_EQ(segments, 1);

    fs::remove_all(dir);
}

// =====================================================================
// Compression: .zst appears after rotation
// =====================================================================

TEST(RotatingFileSink, CompressionProducesZst)
{
    if (!zstd_available())
        GTEST_SKIP() << "zstd binary not found on PATH";

    auto dir = unique_tmp_dir("zstd");
    rotating_file_sink_config cfg;
    cfg.dir = dir;
    cfg.basename = "c";
    cfg.max_size_bytes = 100;
    cfg.max_seconds = 0;
    cfg.compress = true;

    // Enough frames to force several rotations.
    boost::cobalt::run(drive_writes(cfg, 40, 30));

    // Destructor has blocking-waited on all zstd children, so .zst files
    // should already be present and their source .jsonl files removed.
    int zst_count = 0;
    int raw_count = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".zst") ++zst_count;
        else if (entry.path().extension() == ".jsonl") ++raw_count;
    }
    EXPECT_GT(zst_count, 0);
    EXPECT_EQ(raw_count, 0) << "zstd --rm should have removed all .jsonl files";

    fs::remove_all(dir);
}

// =====================================================================
// Time-based rotation
// =====================================================================

TEST(RotatingFileSink, RotatesOnTime)
{
    auto dir = unique_tmp_dir("time");
    rotating_file_sink_config cfg;
    cfg.dir = dir;
    cfg.basename = "t";
    cfg.max_size_bytes = 1ULL << 30;  // effectively disabled
    cfg.max_seconds = 1;              // 1 s rotation
    cfg.compress = false;

    auto job = [cfg]() -> boost::cobalt::task<void> {
        auto exec = co_await boost::cobalt::this_coro::executor;
        auto& ctx = static_cast<boost::asio::io_context&>(
            boost::asio::query(exec, boost::asio::execution::context));

        rotating_file_sink sink(ctx, cfg);
        // Write, sleep >1s, write again → second write must land in a new
        // segment because should_rotate() will trip on age.
        co_await sink("first");
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        co_await sink("second");
    };
    boost::cobalt::run(job());

    int segments = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        (void)entry;
        ++segments;
    }
    EXPECT_GE(segments, 2);

    fs::remove_all(dir);
}
