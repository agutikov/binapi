// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: main entry point — runs all examples sequentially.

#include "examples.hpp"

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/config.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    auto cfg = binapi2::fapi::config::testnet_config();

    // Optionally pick up API credentials from the environment.
    const char* key = std::getenv("BINANCE_API_KEY");
    const char* secret = std::getenv("BINANCE_SECRET_KEY");
    if (key && key[0] != '\0')
        cfg.api_key = key;
    if (secret && secret[0] != '\0')
        cfg.secret_key = secret;

    binapi2::fapi::client c(cfg);

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
