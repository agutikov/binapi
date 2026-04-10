# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Test

Always use the project build scripts — never add `-j$(nproc)` or custom parallelism flags.

```bash
./build.sh          # CMake configure (Debug) + build with -j10
./run_tests.sh           # Run all tests via ctest -j20
```

Run a single test binary directly:
```bash
./_build/tests/binapi2/fapi/signing_test
./_build/tests/binapi2/fapi/decimal_test --gtest_filter='*SomeCase*'
```

Demo CLI against testnet:
```bash
scripts/testnet_rest.sh      # REST endpoints
scripts/testnet_streams.sh   # Market data streams
scripts/testnet_ws_api.sh    # WebSocket API
```

Integration tests require Docker (Postman mock server):
```bash
scripts/api/postman_mock/start.sh   # Start mock
scripts/api/postman_mock/run_run_tests.sh
scripts/api/postman_mock/stop.sh
```

## Project Overview

C++ client library for the Binance API. Two versions coexist:

| Version | Namespace | API | C++ | Async |
|---------|-----------|-----|-----|-------|
| v1 (legacy) | `binapi::rest/ws` | Spot | C++14 | Callbacks |
| **v2 (active)** | `binapi2::fapi` | USD-M Futures | C++23 | Boost.Cobalt coroutines |

All new work targets **binapi2**. Compiler flags: `-Wall -Wextra -Wpedantic -Werror`.

Dependencies: Boost 1.84+ (ASIO, Beast, Cobalt), OpenSSL, ZLIB, Glaze (submodule), Google Test.

## Architecture

### Request Flow (REST)

Request struct → `endpoint_traits<Request>` resolves method/path/security → `to_query_map()` serializes fields → `signing` adds HMAC-SHA256 → `pipeline` dispatches HTTP → Glaze deserializes JSON → `result<Response>` returned.

### Key Abstractions

- **`client`** — facade holding config, HTTP transport, REST pipeline, and service groups (market_data, account, trade, convert, user_data_streams). WebSocket components are lazy-initialized.
- **`rest::pipeline`** — generic dispatch engine. Uses `endpoint_traits<Request>` for compile-time metadata.
- **`rest::service`** — base class; derived services constrain requests via concepts (e.g., `is_account_request`).
- **`result<T>`** — error monad (value + typed error). All fallible operations return this.
- **`endpoint_traits<Request>`** — specialization carrying method, path, response type, security level.
- **Streams** — `market_streams` uses `cobalt::generator<result<Event>>` pattern; `user_streams` uses `std::variant` of 10 event types dispatched via `std::visit(overloaded{...})`.
- **`local_order_book`** — synchronized depth book (REST snapshot + stream deltas).

### Transport Layer

`transport::http_client` and `transport::websocket_client` wrap Boost.Beast. No connection pooling yet. The library does not own an executor — coroutines run on a user-provided executor.

## Code Layout

```
include/binapi2/fapi/       # Public headers
  rest/                     # REST services + pipeline
  websocket_api/            # RPC-style WebSocket API
  streams/                  # Market/user data streams
  transport/                # HTTP/WebSocket clients
  types/                    # Request/response structs, enums
    detail/                 # decimal_t, timestamp_ms, symbol_t, enum_set
src/binapi2/fapi/           # Implementation (.cpp)
tests/binapi2/fapi/         # Google Test unit tests
examples/binapi2/fapi/      # async-demo-cli, sync-demo
docs/binapi2/               # Architecture and design docs
docs/api/                   # Local Binance API reference (HTML, JSON, Markdown)
compose/postman-mock/       # Docker-based mock server for integration tests
```

## Naming Conventions

- **Types/classes**: `snake_case_t` — e.g., `order_side_t`, `decimal_t`, `http_client`
- **Enum values**: `lower_snake_case` — e.g., `order_side_t::buy`, `time_in_force_t::gtc`. `to_string()` produces the uppercase wire format (`"BUY"`, `"GTC"`).
- **Functions/methods**: `lower_snake_case` — e.g., `to_query_map`, `async_execute`
- **Private members**: `trailing_underscore_` — e.g., `config_`, `pipeline_`
- **Request structs**: `<name>_request` — e.g., `ping_request`, `new_order_request`
- **Response structs**: `<name>_response` or `<name>_t` — e.g., `account_info_response`, `kline_t`
- **Stream events**: `<name>_stream_event_t` — e.g., `book_ticker_stream_event_t`
- **Subscription types**: `<name>_subscription` — e.g., `book_ticker_subscription`
- **Trait specializations**: `endpoint_traits<Request>`, `stream_traits<Subscription>`

## Conventions

- **Strong types**: `decimal_t` (string-backed 128-bit), `timestamp_ms`, `symbol_t`, `pair_t` — never use raw numeric types for money
- **No exceptions in library code** — pure `result<T>` monad
- **No default function argument values** — all parameters explicit
- **No backward-compatibility shims** — remove dead code, don't keep stubs
- **Formatting**: `.clang-format` enforced (Mozilla base, 4-space indent, 128-char columns)
- **License**: SPDX-License-Identifier: Apache-2.0 on all files

## Before Modifying Response/Request Types

Always check `docs/api/md/` for the canonical Binance API field definitions before adding or changing types in `include/binapi2/fapi/types/`. The `docs/api/json/` directory has structured endpoint metadata (method, path, parameters).
