// SPDX-License-Identifier: Apache-2.0
//
// sync-demo: main entry point — runs all examples sequentially.

#include "examples.hpp"

#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/config.hpp>

#include <CLI/CLI.hpp>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
    CLI::App app{ "binapi2-fapi synchronous demonstration client" };
    bool live = false;
    app.add_flag("-l,--live,--prod", live, "Use production endpoints (default: testnet)");
    CLI11_PARSE(app, argc, argv);

    binapi2::fapi::config cfg;
    if (live) {
        cfg.rest_host = "fapi.binance.com";
        cfg.websocket_api_host = "ws-fapi.binance.com";
        cfg.websocket_api_target = "/ws-fapi/v1";
        cfg.stream_host = "fstream.binance.com";
    } else {
        cfg = binapi2::fapi::config::testnet_config();
    }

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
