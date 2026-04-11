# binapi

C++ client library for the Binance cryptocurrency exchange API.

Two library versions coexist in this repository:

| Version | Namespace | API | C++ | Async model |
|---------|-----------|-----|-----|-------------|
| **binapi** (v1) | `binapi::rest`, `binapi::ws` | Spot | C++14 | Callbacks |
| **binapi2** (v2) | `binapi2::fapi` | USD-M Futures | C++23 | Boost.Cobalt coroutines |

binapi2 is the actively developed version.  binapi v1 is maintained for
backward compatibility.

## Quick start

```bash
# Clone (with submodules for glaze and Binance docs)
git clone --recurse-submodules <repo-url>
cd binapi

# Build
./build.sh

# Run unit tests
./run_tests.sh

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

### Minimal example (binapi2)

```cpp
#include <binapi2/fapi/client.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <fstream>

int main() {
    binapi2::fapi::config cfg;       // production by default

    // Ed25519 (default, recommended)
    cfg.api_key = "...";
    std::ifstream pem("ed25519_private.pem");
    cfg.ed25519_private_key_pem.assign(
        std::istreambuf_iterator<char>(pem), {});

    // Or HMAC (deprecated)
    // cfg.sign_method = binapi2::fapi::sign_method_t::hmac;
    // cfg.secret_key  = "...";

    boost::asio::io_context io;
    binapi2::fapi::client client(io, cfg);

    // Public endpoint — no auth needed.
    auto r = client.market_data.execute(binapi2::fapi::types::ping_request{});
    if (!r) {
        std::cerr << r.err.message << "\n";
        return 1;
    }

    // Authenticated endpoint.
    auto bal = client.account.balances();
    if (bal) {
        for (auto& b : *bal)
            std::cout << b.asset << ": " << b.balance << "\n";
    }
}
```

### Coroutine example

```cpp
boost::cobalt::task<void> run(binapi2::fapi::client& client) {
    auto r = co_await client.market_data.async_execute(
        binapi2::fapi::types::order_book_request{.symbol = "BTCUSDT", .limit = 5});
    if (r)
        std::cout << "best bid: " << r->bids[0].price << "\n";
}
```

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

### Data types and implementation status

| Document | Description |
|----------|-------------|
| [docs/binapi2/implementation_status.md](docs/binapi2/implementation_status.md) | Per-endpoint coverage matrix (implemented/partial/TBD) |
| [docs/binapi2/data_types.md](docs/binapi2/data_types.md) | All C++ types with field-level status (complete/partial/extra) |
| [docs/binapi2/DESIGN.md](docs/binapi2/DESIGN.md) | Architecture, async model, request flow, stream lifecycle |

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

### Unit tests

Located in `tests/binapi2/fapi/`.  Run with:

```bash
./run_tests.sh
```

| Test | What it covers |
|------|---------------|
| `signing_test` | HMAC-SHA256 and Ed25519 signing, query string construction, percent-encoding |
| `enums_test` | Enum JSON serialization round-trips |
| `decimal_test` | 128-bit decimal arithmetic, parsing, formatting |
| `json_query_test` | Request struct serialization, query parameter conversion |
| `result_test` | `result<T>` monad semantics |
| `postman_validation_test` | Validates all 95 endpoints against the official Binance Postman collection (path, method, parameters) |

### Integration tests

The integration test exercises the full client stack (TLS, HTTP, JSON
deserialization) against a mock Binance API server running in Docker.

```bash
# Start the mock server
./scripts/api/postman_mock/start.sh

# Run the integration test
./scripts/api/postman_mock/run_run_tests.sh

# Stop the mock server
./scripts/api/postman_mock/stop.sh
```

The mock server uses `@jordanwalsh23/postman-local-mock-server` with the
official Binance Postman collection enriched with response examples from the
API documentation.  A merge script (`scripts/api/postman_mock/merge_responses.py`)
injects the responses into the collection at start time.

Infrastructure is in `compose/postman-mock/` (Dockerfile, docker-compose, response
files, mapping config).

Tested endpoints: ping, server time, exchange info, order book, recent trades,
klines, price ticker, book ticker, mark price, funding rate history, account
information, balances, position risk, query order, open orders, listen key
(start/keepalive/close).

## Examples

The demo CLI at `examples/binapi2/fapi/demo-cli/` demonstrates every library
feature.  Each source file covers a different area:

| File | Area |
|------|------|
| `cmd_market_data.cpp` | Public REST endpoints (ping, time, depth, trades, klines, tickers) |
| `cmd_account.cpp` | Account info, balances, positions, income history |
| `cmd_trade.cpp` | Order placement, cancellation, queries |
| `cmd_ws_api.cpp` | WebSocket API (authenticated RPC: logon, place/cancel orders) |
| `cmd_stream.cpp` | Market data WebSocket streams (klines, depth, tickers, liquidations) |
| `cmd_user_stream.cpp` | User data streams (listen key lifecycle, account updates) |
| `cmd_order_book.cpp` | Live synchronized local order book |

### Usage

```bash
binapi2-fapi-demo-cli [flags] <command> [args...]

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
./binapi2-fapi-demo-cli -v time

# Order book (top 5 levels)
./binapi2-fapi-demo-cli -v order-book BTCUSDT 5

# Klines (1-hour candles, last 10)
./binapi2-fapi-demo-cli -v klines BTCUSDT 1h 10

# All mark prices
./binapi2-fapi-demo-cli -v mark-prices
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
