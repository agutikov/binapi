// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/common.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::rest {

class user_data_stream_service
{
public:
    explicit user_data_stream_service(client& owner) noexcept;

    [[nodiscard]] result<types::listen_key_response> start();
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response>> async_start();
    [[nodiscard]] result<types::listen_key_response> keepalive();
    [[nodiscard]] boost::cobalt::task<result<types::listen_key_response>> async_keepalive();
    [[nodiscard]] result<types::empty_response> close();
    [[nodiscard]] boost::cobalt::task<result<types::empty_response>> async_close();

private:
    client& owner_;
};

} // namespace binapi2::fapi::rest
