// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor helpers.

#include "helpers.hpp"

#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <utility>

namespace streams = ::binapi2::fapi::streams;

namespace binapi2::examples::async_recorder::detail_impl {

streams::sinks::rotating_file_sink_config
make_detail_rfs_cfg(const recorder_config& cfg,
                    const std::string& symbol_upper,
                    const std::string& stream_type)
{
    std::filesystem::path dir = cfg.root_dir / "detail" / symbol_upper / stream_type;
    std::filesystem::create_directories(dir);

    streams::sinks::rotating_file_sink_config rfs_cfg;
    rfs_cfg.dir = std::move(dir);
    rfs_cfg.basename = stream_type;
    rfs_cfg.extension = ".jsonl";
    rfs_cfg.max_size_bytes = cfg.rotation_size_bytes;
    rfs_cfg.max_seconds = cfg.rotation_seconds;
    rfs_cfg.compress = true;
    return rfs_cfg;
}

std::optional<std::pair<std::string, std::string>>
parse_stream_header(std::string_view frame)
{
    static constexpr std::string_view key = "\"stream\":\"";
    auto k = frame.find(key);
    if (k == std::string_view::npos) return std::nullopt;
    auto begin = k + key.size();
    auto end = frame.find('"', begin);
    if (end == std::string_view::npos) return std::nullopt;

    std::string_view topic{ frame.data() + begin, end - begin };
    auto at = topic.find('@');
    if (at == std::string_view::npos) return std::nullopt;

    return std::make_pair(std::string(topic.substr(0, at)),
                          std::string(topic.substr(at + 1)));
}

std::string to_upper_copy(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
        out.push_back(static_cast<char>(
            std::toupper(static_cast<unsigned char>(c))));
    return out;
}

std::string now_iso_ms()
{
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count() %
                    1000;
    std::tm tm{};
    ::gmtime_r(&t, &tm);
    char buf[80];
    std::snprintf(buf, sizeof(buf),
                  "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long>(ms));
    return buf;
}

std::string now_filename_ts()
{
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    ::gmtime_r(&t, &tm);
    char buf[80];
    std::snprintf(buf, sizeof(buf),
                  "%04d%02d%02dT%02d%02d%02dZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

} // namespace binapi2::examples::async_recorder::detail_impl
