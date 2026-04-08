# SPDX-License-Identifier: Apache-2.0
#
# demo: shared commands with two entry points (sync and async).

# Sync entry point: regular main(), io_thread drives coroutines.
add_executable(demo-sync sync_main.cpp commands.cpp)
target_link_libraries(demo-sync PRIVATE binapi2_fapi)

# Async entry point: cobalt::main, no io_thread needed.
add_executable(demo-async async_main.cpp commands.cpp)
target_link_libraries(demo-async PRIVATE binapi2_fapi)
