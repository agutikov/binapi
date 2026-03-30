#pragma once

#include <binapi2/umf/types/websocket_api.hpp>

namespace binapi2::umf::websocket_api {

struct session_logon_call {
    types::session_logon_request params{};
};

} // namespace binapi2::umf::websocket_api
