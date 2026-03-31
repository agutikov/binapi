// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/types/websocket_api.hpp>

namespace binapi2::fapi::websocket_api {

struct session_logon_call
{
    types::session_logon_request params{};
};

} // namespace binapi2::fapi::websocket_api
