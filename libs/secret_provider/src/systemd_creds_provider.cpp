// SPDX-License-Identifier: Apache-2.0

#include <secret_provider/systemd_creds_provider.hpp>

#include <boost/asio/post.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <array>
#include <cstdio>
#include <memory>

namespace secret_provider {

systemd_creds_provider::systemd_creds_provider(std::filesystem::path creds_dir) :
    creds_dir_(std::move(creds_dir))
{
}

boost::cobalt::task<std::expected<std::string, std::string>>
systemd_creds_provider::async_get_secret(std::string_view key)
{
    auto file = creds_dir_ / key;
    if (!std::filesystem::exists(file))
        co_return std::unexpected("credential file not found: " + file.string());

    // Run systemd-creds decrypt synchronously on a posted handler to avoid
    // blocking the coroutine executor directly. The subprocess is short-lived.
    auto exec = co_await boost::cobalt::this_coro::executor;

    std::string result;
    std::string error;

    auto do_decrypt = [&] {
        std::string cmd = "systemd-creds decrypt " + file.string() + " - 2>&1";
        std::array<char, 256> buf{};
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            error = "failed to run systemd-creds";
            return;
        }
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe.get())) {
            result += buf.data();
        }
        int status = pclose(pipe.release());
        if (status != 0) {
            error = "systemd-creds decrypt failed (exit " + std::to_string(status) + "): " + result;
            result.clear();
        }
        // Strip trailing newline
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
            result.pop_back();
    };

    // Post the blocking work to the executor and wait
    co_await boost::asio::post(exec, boost::cobalt::use_op);
    do_decrypt();

    if (!error.empty())
        co_return std::unexpected(std::move(error));
    co_return std::move(result);
}

} // namespace secret_provider
