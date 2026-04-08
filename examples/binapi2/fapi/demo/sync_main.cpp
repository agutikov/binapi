// SPDX-License-Identifier: Apache-2.0
//
// Sync entry point: regular main(), creates io_thread via client(config).
// Commands (coroutines) are driven to completion via client.run_sync().

#include "commands.hpp"

#include <cstdlib>
#include <iostream>

static binapi2::fapi::config make_config()
{
    auto cfg = binapi2::fapi::config::testnet_config();
    if (const char* k = std::getenv("BINANCE_API_KEY"))    cfg.api_key = k;
    if (const char* s = std::getenv("BINANCE_SECRET_KEY")) cfg.secret_key = s;
    return cfg;
}

int main(int argc, char* argv[])
{
    if (argc < 2) { demo::print_help(); return 1; }

    try {
        const auto& cmd = demo::find_command(argv[1]);
        demo::args_t args(argv + 2, argv + argc);

        // Sync mode: client owns io_thread, run_sync drives the coroutine.
        binapi2::fapi::client c(make_config());
        return c.run_sync(cmd.fn(c, args));
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
