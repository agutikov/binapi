// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/client.hpp>

#include <utility>

namespace binapi2::fapi::rest::detail {

template<typename Fn>
void
post_callback(boost::asio::io_context& io_context, Fn&& fn)
{
    boost::asio::post(io_context, std::forward<Fn>(fn));
}

query_map
make_futures_data_query(const types::futures_data_request& request);

} // namespace binapi2::fapi::rest::detail
