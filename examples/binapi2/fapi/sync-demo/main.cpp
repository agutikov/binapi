// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: main entry point — runs all examples sequentially.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/config.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    auto cfg = binapi2::fapi::config::testnet_config();

    // Credentials are loaded from libsecret by the async-demo-cli.
    // This sync demo does not load credentials — auth endpoints will be skipped.

    binapi2::futures_usdm_api c(cfg);

    std::cout << "--- REST examples ---\n\n";
    sync_demo::rest_blocking(c);
    sync_demo::rest_future(c);
    sync_demo::rest_callback(c);

    std::cout << "\n--- WS API examples ---\n\n";
    sync_demo::ws_api_blocking(c);
    sync_demo::ws_api_future(c);
    sync_demo::ws_api_callback(c);

    std::cout << "\n--- Stream examples ---\n\n";
    sync_demo::stream_blocking(c);
    sync_demo::stream_callback(c);

    std::cout << "\ndone\n";

    return 0;
}
