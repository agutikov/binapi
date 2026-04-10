// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: declarations of all example functions.

#pragma once

#include <binapi2/futures_usdm_api.hpp>

namespace sync_demo {

void rest_blocking(binapi2::futures_usdm_api& c);
void rest_future(binapi2::futures_usdm_api& c);
void rest_callback(binapi2::futures_usdm_api& c);
void ws_api_blocking(binapi2::futures_usdm_api& c);
void ws_api_future(binapi2::futures_usdm_api& c);
void ws_api_callback(binapi2::futures_usdm_api& c);
void stream_blocking(binapi2::futures_usdm_api& c);
void stream_callback(binapi2::futures_usdm_api& c);

} // namespace sync_demo
