// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: declarations of all example functions.

#pragma once

#include <binapi2/fapi/client.hpp>

namespace sync_demo {

void rest_blocking(binapi2::fapi::client& c);
void rest_future(binapi2::fapi::client& c);
void rest_callback(binapi2::fapi::client& c);
void ws_api_blocking(binapi2::fapi::client& c);
void ws_api_future(binapi2::fapi::client& c);
void ws_api_callback(binapi2::fapi::client& c);
void stream_blocking(binapi2::fapi::client& c);
void stream_callback(binapi2::fapi::client& c);

} // namespace sync_demo
