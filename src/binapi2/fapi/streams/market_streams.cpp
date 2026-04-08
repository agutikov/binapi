// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the async market streams client.

#include <binapi2/fapi/streams/market_streams.hpp>

#include <binapi2/fapi/detail/json_opts.hpp>

#include <glaze/glaze.hpp>

namespace binapi2::fapi::streams {

namespace {

struct stream_control_request
{
    std::string method{};
    std::vector<std::string> params{};
    unsigned int id{};
};

struct stream_list_response
{
    std::vector<std::string> result{};
    unsigned int id{};
};

} // namespace

} // namespace binapi2::fapi::streams

template<>
struct glz::meta<binapi2::fapi::streams::stream_control_request>
{
    using T = binapi2::fapi::streams::stream_control_request;
    static constexpr auto value = object("method", &T::method, "params", &T::params, "id", &T::id);
};

template<>
struct glz::meta<binapi2::fapi::streams::stream_list_response>
{
    using T = binapi2::fapi::streams::stream_list_response;
    static constexpr auto value = object("result", &T::result, "id", &T::id);
};

namespace binapi2::fapi::streams {

market_streams::market_streams(config cfg) :
    transport_(cfg), cfg_(std::move(cfg))
{
}

boost::cobalt::task<result<void>>
market_streams::async_connect(std::string target)
{
    co_return co_await transport_.async_connect(cfg_.stream_host, cfg_.stream_port, std::move(target));
}

boost::cobalt::task<result<std::string>>
market_streams::async_read_text()
{
    co_return co_await transport_.async_read_text();
}

boost::cobalt::task<result<void>>
market_streams::async_close()
{
    co_return co_await transport_.async_close();
}

boost::cobalt::task<result<void>>
market_streams::async_subscribe(const std::vector<std::string>& streams)
{
    stream_control_request req{ .method = "SUBSCRIBE", .params = streams, .id = 1 };
    auto json = glz::write_json(req);
    if (!json) co_return result<void>::failure({ error_code::json, 0, 0, "failed to serialize subscribe request", {} });

    auto wr = co_await transport_.async_write_text(*json);
    if (!wr) co_return result<void>::failure(wr.err);

    // Read and discard the response.
    auto rd = co_await transport_.async_read_text();
    if (!rd) co_return result<void>::failure(rd.err);

    co_return result<void>::success();
}

boost::cobalt::task<result<void>>
market_streams::async_unsubscribe(const std::vector<std::string>& streams)
{
    stream_control_request req{ .method = "UNSUBSCRIBE", .params = streams, .id = 2 };
    auto json = glz::write_json(req);
    if (!json) co_return result<void>::failure({ error_code::json, 0, 0, "failed to serialize unsubscribe request", {} });

    auto wr = co_await transport_.async_write_text(*json);
    if (!wr) co_return result<void>::failure(wr.err);

    auto rd = co_await transport_.async_read_text();
    if (!rd) co_return result<void>::failure(rd.err);

    co_return result<void>::success();
}

boost::cobalt::task<result<std::vector<std::string>>>
market_streams::async_list_subscriptions()
{
    stream_control_request req{ .method = "LIST_SUBSCRIPTIONS", .params = {}, .id = 3 };
    auto json = glz::write_json(req);
    if (!json) co_return result<std::vector<std::string>>::failure({ error_code::json, 0, 0, "failed to serialize list request", {} });

    auto wr = co_await transport_.async_write_text(*json);
    if (!wr) co_return result<std::vector<std::string>>::failure(wr.err);

    auto rd = co_await transport_.async_read_text();
    if (!rd) co_return result<std::vector<std::string>>::failure(rd.err);

    stream_list_response response{};
    glz::context ctx{};
    if (glz::read<detail::json_read_opts>(response, *rd, ctx)) {
        co_return result<std::vector<std::string>>::failure({ error_code::json, 0, 0, "failed to parse list response", *rd });
    }

    co_return result<std::vector<std::string>>::success(std::move(response.result));
}

} // namespace binapi2::fapi::streams
