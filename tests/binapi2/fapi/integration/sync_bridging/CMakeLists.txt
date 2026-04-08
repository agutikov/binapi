# SPDX-License-Identifier: Apache-2.0
#
# Sync bridging integration test: verifies run_sync, future, callback call models.
# Requires the postman mock server to be running.

add_executable(sync_bridging_test main.cpp)
target_link_libraries(sync_bridging_test PRIVATE binapi2_fapi)
