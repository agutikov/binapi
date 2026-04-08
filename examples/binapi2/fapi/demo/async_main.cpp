// SPDX-License-Identifier: Apache-2.0
//
// Async entry point: cobalt::main co_main(), no io_thread needed.
// Commands (coroutines) are co_awaited directly on cobalt's event loop.

#include "commands.hpp"

#include <boost/cobalt/main.hpp>

#include <cstdlib>
#include <iostream>

static binapi2::fapi::config make_config()
{
    auto cfg = binapi2::fapi::config::testnet_config();
    if (const char* k = std::getenv("BINANCE_API_KEY"))    cfg.api_key = k;
    if (const char* s = std::getenv("BINANCE_SECRET_KEY")) cfg.secret_key = s;
    return cfg;
}

boost::cobalt::main co_main(int argc, char* argv[])
{
    if (argc < 2) { demo::print_help(); co_return 1; }

    try {
        const auto& cmd = demo::find_command(argv[1]);
        demo::args_t args(argv + 2, argv + argc);

        // Async mode: no io_thread, coroutines run on cobalt's event loop.
        binapi2::fapi::client c(make_config(), binapi2::fapi::async_mode);
        co_return co_await cmd.fn(c, args);
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        co_return 1;
    }
}
