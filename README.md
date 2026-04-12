# binapi

C++ client library for the Binance cryptocurrency exchange API.

Two library versions coexist in this repository:

| Version | Namespace | API | C++ | Async model | Status |
|---------|-----------|-----|-----|-------------|--------|
| **binapi** (v1) | `binapi::rest`, `binapi::ws` | Spot | C++14 | Callbacks | maintenance |
| **binapi2** (v2) | `binapi2::fapi` | USD-M Futures | C++23 | Boost.Cobalt coroutines | **release** |

binapi2 is the recommended version for new development. It provides a complete
implementation of the Binance USD-M Futures API: REST, WebSocket API, market
data streams, user data streams, and a synchronized local order book. binapi v1
is maintained for backward compatibility.

## binapi2 feature coverage

| Area | Coverage |
|------|----------|
| REST endpoints | 110 (market data, account, trade, convert, user data streams) |
| WebSocket API methods | 16 (public tickers, order place/modify/cancel/query, account/position, algo orders, listen key) |
| Market data streams | 22 (per-symbol, all-symbol, meta) |
| User data stream events | 10 (variant-based dispatch) |
| Local order book | REST snapshot + WS deltas, plus 3-thread pipelined variant |
| Signing methods | Ed25519 (default), HMAC-SHA256 |
| Secret providers | libsecret (default), systemd-creds, env vars |
| Unit tests | 378 |
| Integration tests | 22 (against local mock server) |
| Testnet verification | 135 commands via `scripts/testnet/*.sh` |

## Quick start

```bash
# Clone (with submodules for glaze and Binance docs)
git clone --recurse-submodules <repo-url>
cd binapi

# Build
./build.sh

# Run unit tests + offline benchmarks
./verify.sh

# Full verification (requires Docker for integration + testnet keys)
./verify.sh --all

# Try the demo CLI (uses testnet by default, public endpoints need no keys)
./_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli ping
./_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli -v time
./_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli -v order-book BTCUSDT 5

# For authenticated commands, store keys first (see "API Keys" below)
./_build/examples/binapi2/fapi/async-demo-cli/binapi2-fapi-async-demo-cli -v account-info
```

## API Keys

The library supports two signing methods:

| Method | Config | Status |
|--------|--------|--------|
| **Ed25519** | `sign_method_t::ed25519` (default) | Recommended by Binance, required for WS API `session.logon` |
| HMAC-SHA256 | `sign_method_t::hmac` | Deprecated by Binance, REST-only |

The demo CLI loads API credentials from a secret provider. The default is
**libsecret** (GNOME Keyring / KDE Wallet). Keys are organized by profile —
you can have multiple key sets (e.g. `demo`, `small`, `prod`).

### Generate an Ed25519 keypair

```bash
# Generate private key
openssl genpkey -algorithm Ed25519 -out ed25519_private.pem

# Extract public key (paste this into Binance API key settings)
openssl pkey -in ed25519_private.pem -pubout -out ed25519_public.pem
cat ed25519_public.pem
```

Register the public key on [Binance](https://www.binance.com/en/my/settings/api-management)
or the [testnet](https://testnet.binancefuture.com) — select **Ed25519** as the key type
and paste the contents of `ed25519_public.pem`.

### Store keys (libsecret)

```bash
# Store Ed25519 keys for the "demo" profile
secret-tool store --label "binapi2 demo api_key" \
    service binapi2 key demo/api_key <<< "your-api-key"
secret-tool store --label "binapi2 demo ed25519_private_key" \
    service binapi2 key demo/ed25519_private_key < ed25519_private.pem

# Store keys for a "prod" profile
secret-tool store --label "binapi2 prod api_key" \
    service binapi2 key prod/api_key <<< "your-prod-key"
secret-tool store --label "binapi2 prod ed25519_private_key" \
    service binapi2 key prod/ed25519_private_key < ed25519_private.pem

# For legacy HMAC keys (deprecated)
secret-tool store --label "binapi2 demo secret_key" \
    service binapi2 key demo/secret_key <<< "your-hmac-secret"

# Verify
secret-tool lookup service binapi2 key demo/api_key
secret-tool lookup service binapi2 key demo/ed25519_private_key
```

### Use keys in the CLI

```bash
# Use "demo" profile (testnet, Ed25519 by default)
./binapi2-fapi-async-demo-cli -K libsecret:demo -v account-info

# Use "prod" profile (live)
./binapi2-fapi-async-demo-cli -K libsecret:prod --live -v account-info

# Default profile ("default") — no -K flag needed
./binapi2-fapi-async-demo-cli -v account-info
```

### Other secret providers

```bash
# systemd-creds (encrypted credential files)
echo -n "your-key" | systemd-creds encrypt - /etc/credstore/api_key
systemd-creds encrypt ed25519_private.pem /etc/credstore/ed25519_private_key
./binapi2-fapi-async-demo-cli -K systemd-creds:/etc/credstore -v account-info

# Environment variables (DEPRECATED — shows warnings)
export BINANCE_API_KEY="your-key"
export BINANCE_ED25519_PRIVATE_KEY="$(cat ed25519_private.pem)"
./binapi2-fapi-async-demo-cli -K env -v account-info
```

### Delete keys

```bash
# Remove a specific key
secret-tool clear service binapi2 key demo/api_key
secret-tool clear service binapi2 key demo/ed25519_private_key
secret-tool clear service binapi2 key demo/secret_key
```

### Minimal example (async coroutine)

The library is async-only: every I/O method returns `boost::cobalt::task<result<T>>`.
The user provides the executor (via `cobalt::main`, `io_thread`, or a manual
`io_context`). The top-level entry point is `binapi2::futures_usdm_api`, which
acts as a factory for REST, WS API, and stream clients.

```cpp
#include <binapi2/futures_usdm_api.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <boost/cobalt/main.hpp>
#include <spdlog/spdlog.h>

#include <fstream>

boost::cobalt::main co_main(int, char*[])
{
    using namespace binapi2::fapi;

    config cfg = config::testnet_config();

    // Ed25519 (default, recommended).
    cfg.api_key = "...";
    std::ifstream pem("ed25519_private.pem");
    cfg.ed25519_private_key_pem.assign(
        std::istreambuf_iterator<char>(pem), {});

    // Or HMAC (deprecated):
    // cfg.sign_method = sign_method_t::hmac;
    // cfg.secret_key  = "...";

    binapi2::futures_usdm_api api(std::move(cfg));

    // Public REST call — no auth needed.
    auto rest = co_await api.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }

    auto book = co_await (*rest)->market_data.async_execute(
        types::order_book_request_t{ .symbol = "BTCUSDT", .limit = 5 });
    if (!book) { spdlog::error("{}", book.err.message); co_return 1; }

    spdlog::info("best bid: {} x {}", book->bids[0].price, book->bids[0].quantity);

    // Authenticated REST call.
    auto bal = co_await (*rest)->account.async_execute(types::balances_request_t{});
    if (bal) {
        for (const auto& b : *bal)
            spdlog::info("{}: {}", b.asset, b.balance);
    }

    co_return 0;
}
```

### Stream example (generator)

```cpp
auto streams = api.create_market_stream();
auto gen = streams->subscribe(types::book_ticker_subscription{ .symbol = "BTCUSDT" });
while (gen) {
    auto event = co_await gen;
    if (!event) break;
    spdlog::info("{}  bid {} x {}  ask {} x {}",
                 event->symbol, event->bidPrice, event->bidQty,
                 event->askPrice, event->askQty);
}
```

### Sync bridging

For non-async callers, `io_thread::run_sync()` wraps an async task:

```cpp
#include <binapi2/fapi/detail/io_thread.hpp>

binapi2::fapi::detail::io_thread io;
binapi2::futures_usdm_api api(cfg);

auto rest = io.run_sync(api.create_rest_client());
auto pong = io.run_sync((*rest)->market_data.async_execute(types::ping_request_t{}));
```

The `examples/binapi2/fapi/sync-demo/` directory has complete examples for
blocking, `std::future`, callback, and manual `io_context` patterns.

## Build

### Requirements

| Dependency | Notes |
|------------|-------|
| C++23 compiler | GCC 13+ or Clang 17+ |
| CMake 3.20+ | |
| Boost | Beast, ASIO, Cobalt (coroutines) |
| OpenSSL | TLS for HTTPS and WSS |
| zlib | Compression |
| [Glaze](https://github.com/stephenberry/glaze) | JSON serialization (bundled in `deps/glaze/`) |

### Build commands

```bash
./build.sh                    # CMake Release build

# Or manually:
cmake -S . -B _build -DCMAKE_BUILD_TYPE=Release
cmake --build _build -j16
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `BINAPI2_WITH_TESTS` | `ON` | Build unit and integration tests |

### Run tests

```bash
./run_tests.sh                     # all unit tests via ctest
```

## Dependencies

**External (system-installed):**
- [Boost](https://www.boost.org/) (Beast, ASIO, Cobalt)
- [OpenSSL](https://www.openssl.org/)
- [zlib](https://zlib.net/)

**Bundled (git submodules in `deps/`):**
- [Glaze](https://github.com/stephenberry/glaze) -- header-only compile-time JSON library

**Test-only:**
- [Google Test](https://github.com/google/googletest)

## Architecture

See [docs/binapi2/DESIGN.md](docs/binapi2/DESIGN.md) for the full architecture documentation with
PlantUML diagrams.  Key design points:

- **Generic dispatch** -- request types carry all API metadata via `endpoint_traits<Request>`.
  A single `execute(request)` call resolves the HTTP method, path, security level,
  and response type at compile time.
- **Async-primary** -- all I/O is implemented as `boost::cobalt::task<result<T>>` coroutines.
  Synchronous `execute()` is a thin wrapper via `cobalt::run()`.
- **Service groups** -- endpoints are organized into `market_data_service`, `account_service`,
  `trade_service`, `convert_service`, and `user_data_stream_service`.
- **Result monad** -- every fallible operation returns `result<T>` (success value or
  typed error with HTTP status, Binance error code, and message).
- **Custom types** -- `decimal` (128-bit fixed-point) for all monetary values,
  `timestamp_ms` (typed wrapper around `uint64_t`) for all timestamps.

```
binapi2::fapi::client
  |
  +-- market_data    (service)     -- public REST endpoints
  +-- account        (service)     -- authenticated account info
  +-- trade          (service)     -- order management
  +-- convert        (service)     -- asset conversion
  +-- user_data_streams            -- listen key lifecycle
  |
  +-- transport::http_client       -- Boost.Beast HTTPS
  +-- transport::websocket_client  -- Boost.Beast WSS
  |
  +-- websocket_api::client        -- authenticated WS-API (RPC-style)
  +-- streams::market_streams      -- market data subscriptions
  +-- streams::user_streams        -- account/position updates
  +-- streams::local_order_book    -- synchronized local order book
```

## Documentation

### API reference

Generate Doxygen HTML documentation:

```bash
./scripts/gen_docs.sh
# Open _docs/html/index.html
```

### binapi2 documentation

**New users start with the [user guide](docs/binapi2/guide/README.md)** — a
task-oriented walkthrough of the library combining Binance API semantics,
crypto derivatives fundamentals, and binapi2 type mappings. Six chapters
cover architecture, market data, trading, account management, user stream
events, and core types.

Full index in [docs/binapi2/README.md](docs/binapi2/README.md). Reference documents:

| Document | Description |
|----------|-------------|
| [docs/binapi2/guide/](docs/binapi2/guide/README.md) | **User guide** (6 chapters) |
| [docs/binapi2/DESIGN.md](docs/binapi2/DESIGN.md) | Architecture, async model, request flow, stream lifecycle |
| [docs/binapi2/streams.md](docs/binapi2/streams.md) | WebSocket stream components and usage patterns |
| [docs/binapi2/threading_and_io.md](docs/binapi2/threading_and_io.md) | Executor ownership and coroutine environments |
| [docs/binapi2/json_parsing.md](docs/binapi2/json_parsing.md) | Glaze deserializer behaviour |
| [docs/binapi2/implementation_status.md](docs/binapi2/implementation_status.md) | Per-endpoint coverage matrix |
| [docs/binapi2/data_types.md](docs/binapi2/data_types.md) | C++ request/response type catalog |
| [docs/binapi2/demo_cli_all_apis.md](docs/binapi2/demo_cli_all_apis.md) | Demo CLI command reference + known testnet issues |
| [docs/binapi2/secret_provider.md](docs/binapi2/secret_provider.md) | Credential loading (libsecret / systemd-creds) |

### Binance API reference (local)

The `docs/api/` directory contains a local copy of the official Binance USD-M
Futures API documentation in three formats:

```
docs/api/
  html/       -- mirrored HTML pages from developers.binance.com
  json/       -- structured JSON (extracted from HTML)
  md/         -- Markdown conversion
  binance-api-postman/     -- official Postman collections and environments
  binance-spot-api-docs/   -- official Binance spot API docs (git submodule)
```

### Scripts for grabbing API reference

Three scripts in `scripts/api/docs/` form a pipeline for maintaining a local
copy of the Binance public API documentation:

| Script | Description |
|--------|-------------|
| `download-usdm-api-reference.sh` | `wget` mirror of the USD-M Futures docs from developers.binance.com to `docs/api/html/` |
| `extract_usdm_docs_to_json.py` | Parse the mirrored HTML into structured JSON (endpoint path, method, parameters, response examples) in `docs/api/json/` |
| `convert_usdm_json_to_markdown.py` | Convert the JSON docs to Markdown in `docs/api/md/` |

Usage:

```bash
# 1. Download (requires wget)
./scripts/api/docs/download-usdm-api-reference.sh

# 2. Extract HTML -> JSON (requires Python with lxml, orjson)
python3 scripts/api/docs/extract_usdm_docs_to_json.py \
    --input-dir docs/api/html/developers.binance.com/docs/derivatives/usds-margined-futures \
    --output-dir docs/api/json/developers.binance.com/docs/derivatives/usds-margined-futures

# 3. Convert JSON -> Markdown
python3 scripts/api/docs/convert_usdm_json_to_markdown.py \
    --input-dir docs/api/json/developers.binance.com/docs/derivatives/usds-margined-futures \
    --output-dir docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures
```

## Testing

### One-shot verification

`verify.sh` runs every verification level and reports pass/fail/skipped counts.

```bash
./verify.sh              # Levels 1-2 (no Docker, no keys)
./verify.sh --mock       # Levels 1-4 (mock server must be running)
./verify.sh --testnet    # Levels 1-2 + 5 (testnet keys required)
./verify.sh --all        # Levels 1-5 (starts/stops mock server, runs testnet)
```

Levels 3-5 categorize testnet failures against a known-failure list and only
reports unexpected errors as test failures. Known testnet-side issues (see
[demo_cli_all_apis.md](docs/binapi2/demo_cli_all_apis.md#known-testnet-issues))
are counted separately.

### Verification checklist

The library is verified at five levels. All levels must pass before release.

#### Level 1: Unit tests (378 tests, offline)

```bash
./build.sh
./run_tests.sh          # ctest -j20, ~1s
```

| Binary | Tests | What it covers |
|--------|------:|----------------|
| `signing_test` | 21 | HMAC-SHA256 hex, Ed25519 base64 signing, query string construction, percent-encoding, auth injection |
| `enums_test` | 49 | Enum-to-string, string-to-enum, JSON round-trip for all 23 enum types |
| `enum_set_test` | 32 | `enum_set<E>` bitset operations, JSON array serialization |
| `decimal_test` | 114 | 128-bit decimal parsing (string and JSON number), arithmetic, overflow, comparison, formatting, JSON round-trip |
| `json_query_test` | 14 | `to_query_map()` serialization, optional field omission, enum/decimal/timestamp encoding |
| `result_test` | 46 | `result<T>` monad: success/failure, value access, error propagation, void specialization |
| `response_parse_test` | 30 | JSON deserialization of all response types (valid, missing required, missing optional, extra fields, corrupted) |
| `postman_validation_test` | 6 | Validates endpoint metadata (path, method, parameters) against the official Binance Postman collection |
| `stream_test` | 32 | Market/user stream event parsing, subscription topic generation, stream trait dispatch |
| `stream_buffer_test` | 12 | `threadsafe_stream_buffer` SPSC ring buffer, async read/write, close semantics |
| `stream_consumer_test` | 8 | `buffer_consumer` and `inline_consumer` frame dispatch |
| `stream_recorder_test` | 6 | Stream frame recording with callback and spdlog sinks |
| `io_thread_test` | 4 | Dedicated I/O thread lifecycle, executor dispatch |

#### Level 2: Offline benchmarks

```bash
./_build/tests/binapi2/fapi/benchmarks/stream_parse_benchmark
./_build/tests/binapi2/fapi/benchmarks/stream_buffer_benchmark
./_build/tests/binapi2/fapi/benchmarks/stream_recorder_benchmark
```

| Binary | What it measures |
|--------|------------------|
| `stream_parse_benchmark` | Stream JSON parsing throughput (book ticker, depth, kline, user events) |
| `stream_buffer_benchmark` | `threadsafe_stream_buffer` SPSC throughput and latency |
| `stream_recorder_benchmark` | Stream frame recording pipeline throughput |

#### Level 3: Integration tests (22 tests, requires Docker)

```bash
scripts/api/postman_mock/start.sh       # start mock server
scripts/api/postman_mock/run_test.sh    # 18 async + 4 sync bridging tests
scripts/api/postman_mock/stop.sh        # stop mock server
```

Exercises the full client stack (TLS, HTTP/1.1, JSON deserialization, signing)
against a local mock Binance API server built from the official Postman
collection with injected response examples.

| Suite | Tests | What it covers |
|-------|------:|----------------|
| `postman_mock_integration` | 18 | REST pipeline end-to-end: ping, time, exchange info, order book, recent trades, klines, tickers (price, book, mark), funding rate, account info, balances, position risk, query order, open orders, listen key lifecycle |
| `sync_bridging_test` | 4 | Synchronous wrappers: `run_sync`, `future`, `callback`, `manual_io_context` |

#### Level 4: Online benchmark (requires Docker)

```bash
scripts/api/postman_mock/run_benchmark.sh
```

| Binary | What it measures |
|--------|------------------|
| `rest_benchmark` | REST request-to-response latency and throughput (16 endpoints, public + signed, against mock server) |

#### Level 5: Testnet verification (135 commands, requires API keys)

```bash
scripts/testnet/market_data.sh      # 39 market data REST
scripts/testnet/account.sh          # 21 account REST
scripts/testnet/trade.sh            # 29 trade REST
scripts/testnet/convert.sh          # 3 convert REST
scripts/testnet/ws_api.sh           # 16 WebSocket API
scripts/testnet/streams.sh          # 22 market data streams
scripts/testnet/user_streams.sh     # 4 user data streams
scripts/testnet/order_book.sh       # 2 live order book
```

Each script saves per-command output to `testnet_output/<group>/<command>/`:

| File | Content |
|------|---------|
| `request` | Raw HTTP request or WS-API JSON |
| `response.json` | Response body |
| `log.txt` | Trace-level log |
| `stdout.txt` | CLI output with parsed JSON (`-v`) |
| `stream.jsonl` | Raw WebSocket frames (stream/order-book commands only) |

**Expected results:** 120 pass, 15 fail (testnet-side issues, not library bugs).

#### Known testnet failures (not fixable)

These commands fail on the Binance USD-M Futures testnet due to server-side
limitations. The library types match the documented production API.

| Category | Commands | Cause |
|----------|----------|-------|
| Analytics not available | `open-interest-stats`, `top-ls-account-ratio`, `top-ls-trader-ratio`, `long-short-ratio`, `taker-volume`, `basis`, `delivery-price` | Testnet returns plain text `ok` instead of JSON |
| Non-standard response | `test-order` | Testnet returns empty enum strings `""` in stub response |
| Non-standard response | `tradfi-perps` | Testnet returns plain string `SUCCESS` instead of `{"code":200,"msg":"success"}` |
| Endpoint missing | `pm-account-info` | HTTP 404 — endpoint not deployed on testnet |
| Server error | `quantitative-rules` | HTTP 400, Binance code -1000 |
| Service unavailable | `convert-quote`, `convert-order-status` | HTTP 500 — convert service not running on testnet |
| Requires balance | `ws-order-place` | Needs testnet USDT balance to place orders |

Mock server infrastructure is in `compose/postman-mock/` (Dockerfile,
docker-compose, response files, mapping config). It uses
`@jordanwalsh23/postman-local-mock-server` with the official Binance Postman
collection, enriched with response examples via
`scripts/api/postman_mock/merge_responses.py`.

## Examples

The demo CLI at `examples/binapi2/fapi/async-demo-cli/` demonstrates every
library feature via 135 commands. See
[docs/binapi2/demo_cli_all_apis.md](docs/binapi2/demo_cli_all_apis.md) for the
full command reference. Source files cover different areas:

| File | Area |
|------|------|
| `cmd_market_data.cpp` | 39 public REST endpoints (ping, time, depth, trades, klines, tickers, analytics) |
| `cmd_account.cpp` | 21 account endpoints (info, balances, positions, income, config, downloads) |
| `cmd_trade.cpp` | 29 trade endpoints (order lifecycle, margin, leverage, algo orders) |
| `cmd_convert.cpp` | 3 convert endpoints |
| `cmd_ws_api.cpp` | 16 WebSocket API methods (session logon, tickers, orders, positions) |
| `cmd_stream.cpp` | 22 market data streams (per-symbol, all-symbol, meta) |
| `cmd_user_stream.cpp` | 4 user data stream commands (listen key lifecycle + variant stream) |
| `cmd_order_book.cpp` | Live synchronized local order book (single-thread) |
| `cmd_pipeline_order_book.cpp` | 3-thread pipelined local order book |

A second example, `examples/binapi2/fapi/sync-demo/`, shows four synchronous
bridging patterns against the async-only library: blocking, future, callback,
and manual `io_context`.

### Usage

```bash
binapi2-fapi-async-demo-cli [flags] <command> [args...]

Flags:
  -v          Print JSON responses
  -vv         Print JSON + transport log
  -vvv        Print JSON + full HTTP headers
  --live      Production endpoints
  --testnet   Testnet endpoints (default)
```

### HOWTO: fetch market data

```bash
# Server time
./binapi2-fapi-async-demo-cli -v time

# Order book (top 5 levels)
./binapi2-fapi-async-demo-cli -v order-book BTCUSDT 5

# Klines (1-hour candles, last 10)
./binapi2-fapi-async-demo-cli -v klines BTCUSDT 1h 10

# All mark prices
./binapi2-fapi-async-demo-cli -v mark-prices
```

### HOWTO: place and manage orders (testnet)

```bash
# Store testnet keys first (see "API Keys" above)
# Then use the demo profile:

# Test order (validated but not placed)
./binapi2-fapi-async-demo-cli -K libsecret:demo -v test-order BTCUSDT BUY LIMIT -q 0.001 -p 50000 -t GTC

# Place a real order
./binapi2-fapi-async-demo-cli -K libsecret:demo -v new-order BTCUSDT BUY LIMIT -q 0.001 -p 50000 -t GTC

# Query order status
./binapi2-fapi-async-demo-cli -K libsecret:demo -v query-order BTCUSDT 123456789

# Cancel order
./binapi2-fapi-async-demo-cli -K libsecret:demo -v cancel-order BTCUSDT 123456789
```

### HOWTO: stream market data

```bash
# Live order book (synchronized via WebSocket depth stream + REST snapshot)
./binapi2-fapi-async-demo-cli order-book-live BTCUSDT 10

# Pipeline order book (3-thread: network / parser / logic)
./binapi2-fapi-async-demo-cli pipeline-order-book-live BTCUSDT 10

# Kline stream (with recording to JSONL file)
./binapi2-fapi-async-demo-cli -r klines.jsonl stream-kline BTCUSDT 1m

# All book tickers (real-time best bid/ask for every symbol)
./binapi2-fapi-async-demo-cli stream-all-book-tickers
```

### HOWTO: use the WebSocket API

```bash
# Authenticated WebSocket session
./binapi2-fapi-async-demo-cli -K libsecret:demo -v ws-logon

# Place order via WebSocket API
./binapi2-fapi-async-demo-cli -K libsecret:demo -v ws-order-place BTCUSDT BUY LIMIT -q 0.001 -p 50000 -t GTC
```

## License

[Apache License 2.0](LICENSE)
