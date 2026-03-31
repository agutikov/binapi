# Task - add payload, signing, and metadata validation

## Objective

Build a validation base that detects structural regressions as the surface grows.

## Work

- add representative parser and serializer tests for REST responses, stream events, and websocket RPC envelopes
- add signing tests against known canonical query examples where possible
- add endpoint and method coverage checks that compare tracker inventory against declared metadata in [`include/binapi2/fapi/rest/generated_endpoints.hpp`](include/binapi2/fapi/rest/generated_endpoints.hpp) and [`include/binapi2/fapi/websocket_api/generated_methods.hpp`](include/binapi2/fapi/websocket_api/generated_methods.hpp)
- add focused fixtures for the newly introduced long-tail stream and account payloads
