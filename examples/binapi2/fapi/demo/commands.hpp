// SPDX-License-Identifier: Apache-2.0
//
// Shared demo commands — all implemented as cobalt::task coroutines.
// Both sync_main.cpp and async_main.cpp dispatch through this interface.

#pragma once

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/detail/json_opts.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/streams.hpp>
#include <binapi2/fapi/types/websocket_api.hpp>

#include <boost/cobalt/task.hpp>
#include <glaze/glaze.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace demo {

using args_t = std::vector<std::string>;
using command_fn = std::function<boost::cobalt::task<int>(binapi2::fapi::client&, const args_t&)>;

struct command_entry
{
    std::string_view name;
    command_fn fn;
    std::string_view help;
};

// Print a value as pretty JSON to stdout.
template<typename T>
void print_json(const T& value)
{
    if (auto j = glz::write<glz::opts{.prettify = true}>(value))
        std::cout << *j << '\n';
}

// --- REST commands ---

boost::cobalt::task<int> cmd_ping(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_server_time(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_exchange_info(binapi2::fapi::client& c, const args_t& args);
boost::cobalt::task<int> cmd_order_book(binapi2::fapi::client& c, const args_t& args);

// --- Stream commands ---

boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::fapi::client& c, const args_t& args);

// --- WS API commands ---

boost::cobalt::task<int> cmd_ws_book_ticker(binapi2::fapi::client& c, const args_t& args);

// --- Command table ---

const command_entry& find_command(std::string_view name);
void print_help();

} // namespace demo
