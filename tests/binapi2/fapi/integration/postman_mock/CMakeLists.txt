# SPDX-License-Identifier: Apache-2.0
#
# Integration test: binapi2 client against the Postman mock server.
#
# This is NOT discovered by ctest automatically — it requires the mock server
# to be running (scripts/api/postman_mock/start.sh).  Run it manually:
#
#   SSL_CERT_FILE=compose/postman-mock/certs/server.crt \
#       ./_build/tests/binapi2/fapi/integration/postman_mock/postman_mock_integration

add_executable(postman_mock_integration main.cpp)
target_link_libraries(postman_mock_integration PRIVATE binapi2_fapi)
