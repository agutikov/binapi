# SPDX-License-Identifier: Apache-2.0
#
# async-demo: cobalt::main prototype with REST, WS stream, and WS API.

add_executable(binapi2-fapi-async-demo main.cpp)
target_link_libraries(binapi2-fapi-async-demo PRIVATE binapi2_fapi)
