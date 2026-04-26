// SPDX-License-Identifier: Apache-2.0

#include <secret_provider/dir_provider.hpp>

#include <boost/asio/post.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <fstream>
#include <sstream>

namespace secret_provider {

dir_provider::dir_provider(std::filesystem::path dir) :
    dir_(std::move(dir))
{
}

boost::cobalt::task<std::expected<std::string, std::string>>
dir_provider::async_get_secret(std::string_view key)
{
    auto path = dir_ / key;
    if (!std::filesystem::exists(path))
        co_return std::unexpected("secret file not found: " + path.string());

    auto exec = co_await boost::cobalt::this_coro::executor;

    std::string contents;
    std::string error;

    auto do_read = [&] {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            error = "failed to open: " + path.string();
            return;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        contents = ss.str();
        if (!contents.empty() && contents.back() == '\n')
            contents.pop_back();
        if (!contents.empty() && contents.back() == '\r')
            contents.pop_back();
    };

    co_await boost::asio::post(exec, boost::cobalt::use_op);
    do_read();

    if (!error.empty())
        co_return std::unexpected(std::move(error));
    co_return std::move(contents);
}

} // namespace secret_provider
