# WebSocket Streams Architecture

## Overview

binapi2 provides three stream components:

| Component | Purpose | Connection | Events |
|---|---|---|---|
| `streams::market_streams` | Public market data | 1 WS per stream (or combined) | book tickers, depth, klines, trades, etc. |
| `streams::user_streams` | Private account events | 1 WS per listen key | orders, balances, margin calls, etc. |
| `streams::local_order_book` | Synchronized order book | Uses market_streams + REST | Depth snapshots |

All are built on `transport::websocket_client` which provides async-primary I/O via Boost.Cobalt coroutines.

---

## Usage Patterns

### Pattern 1: Sync callback loop

```cpp
client c(cfg);
auto& streams = c.streams();

// Connect to a single stream
streams.connect_book_ticker({.symbol = "BTCUSDT"});

// Block on read loop — handler called for each event.
// Return true to keep reading, false to stop the loop.
streams.read_book_ticker_loop([](const types::book_ticker_stream_event_t& e) {
    std::cout << e.symbol << " " << e.best_bid_price << "/" << e.best_ask_price << "\n";
    return true;  // true = keep reading, false = stop
});

streams.close();
```

The sync read loop blocks the calling thread. Internally: `run_read_loop` calls `read_text()` which calls `io_thread::run_sync(async_read_text())`.

See `examples/binapi2/fapi/demo-cli/cmd_stream.cpp` for working examples of all stream types.

### Pattern 2: Async coroutine loop

```cpp
client c(cfg, async_mode);
auto& streams = c.streams();

co_await streams.async_connect("/ws/btcusdt@bookTicker");

while (true) {
    auto msg = co_await streams.async_read_text();
    if (!msg) break;

    types::book_ticker_stream_event_t event{};
    glz::context ctx{};
    if (glz::read<detail::json_read_opts>(event, *msg, ctx)) continue;

    // process event
}

co_await streams.async_close();
```

The async pattern uses `cobalt::task` — each `co_await` suspends, yielding to the event loop. No thread is blocked.

### Pattern 3: Local order book

```cpp
client c(cfg);
local_order_book book(c.streams(), c);

book.set_snapshot_callback([](const order_book_snapshot& snap) {
    // called under mutex after each update
    std::cout << "bids: " << snap.bids.size() << " asks: " << snap.asks.size() << "\n";
});

book.start("BTCUSDT", 1000);  // blocks until stop() or error
```

Internally:
1. Connects to `@depth@100ms` diff stream
2. Buffers incoming events
3. Fetches REST snapshot (`/fapi/v1/depth`)
4. Discards buffered events before snapshot
5. Applies remaining buffered events
6. Enters continuous apply loop
7. Detects sequence gaps and re-syncs automatically

---

## Event Flow

### No internal queues

Events flow synchronously from transport to handler with no intermediate buffering:

```
Binance WS server
    │
    ▼
transport::websocket_client::async_read_text()
    │
    ▼ raw JSON string
stream_recorder callback (if configured — raw frame recording)
    │
    ▼ raw JSON string
read_stream_loop<Event>()
    │ glz::read<json_read_opts>(event, payload)
    ▼ typed event
handler(event) → bool
```

The handler runs on the io_thread's background thread (sync mode) or on the cobalt event loop thread (async mode). The next frame is not read until the handler returns.

**Consequence:** A slow handler blocks the entire stream. If processing takes longer than the inter-message interval, messages queue in the kernel TCP receive buffer and eventually in Binance's send buffer, potentially causing the server to drop the connection.

### Where buffering exists today

| Component | Buffer | Bounded? | Overflow |
|---|---|---|---|
| `transport::websocket_client` | Beast `flat_buffer` — 1 frame | Yes (1) | N/A — read one, process, repeat |
| `local_order_book` | `std::vector<depth_event>` — pre-sync events | No | Unbounded growth until snapshot completes |
| TCP kernel buffer | OS-level socket receive buffer | Yes (OS-tuned) | Binance drops connection |

### What's missing: application-level queues

The library does not provide:
- Bounded queues between transport and handlers
- Overflow policies (drop oldest, drop newest, block, fail)
- Async access to queued data (e.g., drain N events)
- Backpressure signaling to the transport layer

These are deliberately left to the application. Possible designs:

**Option A: Lock-free ring buffer (SPSC)**
```
io_thread ──push──► ring_buffer ──pop──► processing thread
                    (fixed size)
```
- Overflow policy: overwrite oldest (lossy) or block producer
- Best for: high-frequency market data where latest state matters more than history

**Option B: Bounded concurrent queue**
```
io_thread ──push──► mpsc_queue ──pop──► N consumer threads
                    (bounded)
```
- Overflow policy: configurable (drop/block/fail)
- Best for: order execution events where every event matters

**Option C: Channel (cobalt::channel)**
```
co_await stream.async_read_text()
    │
    co_await channel.push(event)  ← suspends if full
    │
consumer: co_await channel.pop()
```
- Overflow policy: backpressure (producer suspends)
- Best for: async pipelines within a single io_context

---

## Start / Stop / Subscribe / Unsubscribe

### Single-stream connections

Each `connect_*` method opens a dedicated WebSocket connection to a specific stream endpoint:

```
connect_book_ticker({.symbol = "BTCUSDT"})
    → wss://fstream.binance.com/ws/btcusdt@bookTicker

connect_kline({.symbol = "ETHUSDT", .interval = kline_interval_t::h1})
    → wss://fstream.binance.com/ws/ethusdt@kline_1h
```

**Stop:** Call `close()` / `async_close()`. The WebSocket close handshake is performed.

**No connection reuse:** After `close()`, the connection is gone. A new `connect_*` call creates a new TCP+TLS+WS connection. There is no reconnect or connection pool.

### Combined stream (subscribe/unsubscribe on one connection)

```cpp
auto& streams = c.streams();

// Open a single combined stream connection
streams.connect_combined("/stream");

// Subscribe to topics dynamically
streams.subscribe({"btcusdt@bookTicker", "ethusdt@bookTicker"});

// Read events (all subscribed streams arrive on this connection)
streams.read_book_ticker_loop([](const auto& e) {
    // events from both BTCUSDT and ETHUSDT arrive here
    return true;
});

// Modify subscriptions without disconnecting
streams.unsubscribe({"ethusdt@bookTicker"});
streams.subscribe({"btcusdt@depth@100ms"});

// Check what's active
auto subs = streams.list_subscriptions();

streams.close();
```

**Protocol:** Subscribe/unsubscribe send JSON control messages on the same WebSocket:
```json
{"method": "SUBSCRIBE", "params": ["btcusdt@bookTicker"], "id": 1}
{"method": "UNSUBSCRIBE", "params": ["ethusdt@bookTicker"], "id": 2}
{"method": "LIST_SUBSCRIPTIONS", "params": [], "id": 3}
```

**Connection reuse:** Yes — the combined connection stays open across subscribe/unsubscribe cycles. Topics can be added and removed dynamically.

**Limitation:** subscribe/unsubscribe currently don't validate Binance's response. Transport-level errors are caught, but a rejected subscription (e.g., invalid stream name) is silently ignored.

---

## Multiple Parallel Streams

### Different connections (current implementation)

Each `market_streams` instance owns one `websocket_client`. To run multiple streams in parallel, create multiple `market_streams` instances:

```cpp
// Each stream gets its own WebSocket connection
streams::market_streams book_stream(io, cfg);
streams::market_streams trade_stream(io, cfg);

book_stream.connect_book_ticker({.symbol = "BTCUSDT"});
trade_stream.connect_aggregate_trade({.symbol = "BTCUSDT"});

// These block — need separate threads or async
std::thread t1([&] { book_stream.read_book_ticker_loop(handle_book); });
std::thread t2([&] { trade_stream.read_aggregate_trade_loop(handle_trade); });
```

**Problem:** Each instance creates a separate TLS connection. For N streams = N connections = N TLS handshakes.

**Async alternative:**
```cpp
// In cobalt::main or a spawned coroutine:
co_await cobalt::join(
    read_book_ticker(streams1),
    read_aggregate_trade(streams2)
);
```

### One connection (combined stream)

Binance supports up to ~200 streams on a single combined connection:

```cpp
auto& streams = c.streams();
streams.connect_combined("/stream");
streams.subscribe({
    "btcusdt@bookTicker",
    "ethusdt@bookTicker",
    "btcusdt@depth@100ms",
    "btcusdt@aggTrade"
});
```

**Caveat:** The combined connection delivers all events through a single read loop. The events arrive as:
```json
{"stream": "btcusdt@bookTicker", "data": { ... }}
```

The current typed `read_*_loop` methods assume a single-stream connection and parse the `data` field directly. For combined streams, the caller must use `async_read_text()` and dispatch manually based on the `"stream"` field.

---

## User Data Streams

User streams require a listen key from the REST API:

```cpp
// Get listen key
auto key = c.user_data_streams.start();

// Connect
auto& us = c.user_streams();
us.connect(key->listenKey);

// Register handlers
streams::user_streams::handlers h;
h.on_order_trade_update = [](const auto& e) {
    std::cout << e.order.symbol << " " << e.order.status << "\n";
    return true;
};
h.on_account_update = [](const auto& e) { return true; };
h.on_listen_key_expired = [](const auto& e) { return false; };  // stop on expiry

// Block on read loop
us.read_loop(h);
us.close();
```

**Event dispatch:** String pattern matching on `"e"` field, then full JSON parse only for registered handlers. Unregistered event types are skipped without parsing.

**Listen key lifecycle:**
- `user_data_streams.start()` — creates listen key (valid 60 minutes)
- `user_data_streams.keepalive()` — extend validity (call every 30 minutes)
- `user_data_streams.close()` — invalidate listen key
- `on_listen_key_expired` handler fires when server expires the key

The library does not automatically manage listen key keepalive. The application must schedule periodic keepalive calls.

---

## Connection Lifecycle

```
                    ┌──────────┐
                    │ Idle     │
                    └────┬─────┘
                         │ connect_*() / async_connect()
                         ▼
              ┌──────────────────────┐
              │ Connected            │
              │  - DNS resolve       │
              │  - TCP connect       │
              │  - TLS handshake     │
              │  - WS handshake      │
              │  - ping/pong active  │
              └──────────┬───────────┘
                         │ read_*_loop() / async_read_text()
                         ▼
              ┌──────────────────────┐
              │ Reading              │◄──── handler returns true
              │  - frame arrives     │
              │  - parse JSON        │
              │  - invoke handler    ├────► handler returns false
              └──────────┬───────────┘
                         │
                         ▼
              ┌──────────────────────┐
              │ Closing              │
              │  - WS close frame    │
              │  - TCP shutdown      │
              └──────────┬───────────┘
                         │
                         ▼
                    ┌──────────┐
                    │ Closed   │  (no reuse — new connect needed)
                    └──────────┘
```

**Ping/pong:** Installed automatically during WS handshake. Binance sends periodic pings; the transport replies with pong synchronously in the control callback. No application action needed.

**No automatic reconnection.** If the connection drops (network error, server disconnect, ping timeout), the read loop returns an error result. The application must detect this and reconnect.

---

## Source Reference

| File | Role |
|---|---|
| `include/binapi2/fapi/streams/market_streams.hpp` | Market stream API: connect, read loops, combined stream control |
| `src/binapi2/fapi/streams/market_streams.cpp` | Implementation: target URL construction, subscribe/unsubscribe JSON protocol |
| `include/binapi2/fapi/streams/user_streams.hpp` | User data stream API: handlers struct, read loop overloads |
| `src/binapi2/fapi/streams/user_streams.cpp` | Implementation: event type matching, dispatch chain |
| `include/binapi2/fapi/streams/local_order_book.hpp` | Thread-safe local order book with automatic sync |
| `src/binapi2/fapi/streams/local_order_book.cpp` | Implementation: buffer → snapshot → apply → gap detection |
| `include/binapi2/fapi/transport/websocket_client.hpp` | Low-level WS transport: async/sync read/write/connect/close |
| `include/binapi2/fapi/types/subscriptions.hpp` | Subscription parameter types (symbol_t, pair_t, intervals) |
| `include/binapi2/fapi/types/streams.hpp` | Stream event types with glaze JSON metadata |
| `include/binapi2/fapi/config.hpp` | Endpoint URLs, stream_recorder callback |
