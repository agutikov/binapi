# WebSocket Streams Architecture

## Overview

binapi2 provides three stream components:

| Component | Purpose | Connection | Events |
|---|---|---|---|
| `streams::market_streams` | Public market data | 1 WS per stream (or combined) | book tickers, depth, klines, trades, etc. |
| `streams::user_streams` | Private account events | 1 WS per listen key | orders, balances, margin calls, etc. |
| `streams::local_order_book` | Synchronized order book | Uses market_streams + rest::pipeline | Depth snapshots |

All are built on `transport::websocket_client` which provides async-only I/O via
Boost.Cobalt coroutines. All methods return `cobalt::task` or `cobalt::generator`.

---

## Usage Patterns

### Pattern 1: Generator (recommended)

The `subscribe()` method connects to the stream and returns a typed async generator:

```cpp
fapi::client c(cfg);
auto& streams = c.streams();

// subscribe() connects and returns a generator<result<Event>>
auto stream = streams.subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});

while (stream) {
    auto event = co_await stream;
    if (!event) {
        spdlog::error("stream error: {}", event.err.message);
        break;
    }
    spdlog::info("{} bid={} ask={}", event->symbol, event->best_bid_price, event->best_ask_price);
}

co_await streams.async_close();
```

The subscription type is resolved at compile time via `stream_traits<Subscription>`:
- `stream_traits<Subscription>::target(cfg, sub)` builds the WS target URL
- `stream_traits<Subscription>::event_type` is the parsed event type

The generator yields `result<Event>` until an error occurs or the caller stops consuming.

**Requires `cobalt::main`** or an equivalent event loop that can drive generators.
Generators cannot be bridged through `io_thread::run_sync()`.

See `examples/binapi2/fapi/async-demo-cli/` for working examples.

### Pattern 2: Typed async connect + read event

For per-event control without the generator wrapper:

```cpp
auto& streams = c.streams();

// Connect using a subscription type (stream_traits resolves the target URL)
co_await streams.async_connect(types::book_ticker_subscription{.symbol = "BTCUSDT"});

// Read events one at a time, fully typed
while (true) {
    auto event = co_await streams.async_read_event<types::book_ticker_stream_event_t>();
    if (!event) break;
    // process event...
}

co_await streams.async_close();
```

This pattern works with both `cobalt::main` and `io_thread::run_sync()` (wrapping
each `async_read_event` call individually).

### Pattern 3: Low-level raw text

For maximum control (custom parsing, combined streams, debugging):

```cpp
auto& streams = c.streams();

// Connect with a raw target path
co_await streams.async_connect("/ws/btcusdt@bookTicker");

// Read raw JSON frames
while (true) {
    auto msg = co_await streams.async_read_text();
    if (!msg) break;
    // *msg is a raw JSON string — parse it yourself
}

co_await streams.async_close();
```

### Pattern 4: Local order book

The local order book is an async coroutine that maintains a synchronized order book:

```cpp
fapi::client c(cfg);
auto& streams = c.streams();

streams::local_order_book book(streams, c.rest());

book.set_snapshot_callback([](const streams::order_book_snapshot& snap) {
    spdlog::info("bids: {} asks: {}", snap.bids.size(), snap.asks.size());
});

// async_run is a coroutine — runs until stop() or error
auto result = co_await book.async_run(types::symbol_t{"BTCUSDT"}, 1000);
```

Internally:
1. Connects to `@depth@100ms` diff stream via `market_streams`
2. Buffers incoming events
3. Fetches REST snapshot via `rest::pipeline` (`/fapi/v1/depth`)
4. Discards buffered events before snapshot
5. Applies remaining buffered events
6. Enters continuous apply loop
7. Detects sequence gaps and re-syncs automatically

---

## Event Flow

### No internal queues

Events flow from transport through the generator to the consumer with no intermediate
buffering:

```
Binance WS server
    |
    v
transport::websocket_client::async_read_text()
    |
    v raw JSON string
market_streams::subscribe() generator body
    | glz::read<json_read_opts>(event, payload)
    v typed event
co_yield result<Event>::success(event)
    |
    v
consumer: co_await generator
```

The consumer runs on the coroutine's executor (typically `cobalt::main`). The next
frame is not read until the consumer resumes the generator by calling `co_await` again.

**Consequence:** A slow consumer blocks the entire stream. If processing takes longer
than the inter-message interval, messages queue in the kernel TCP receive buffer and
eventually in Binance's send buffer, potentially causing the server to drop the
connection.

### Where buffering exists today

| Component | Buffer | Bounded? | Overflow |
|---|---|---|---|
| `transport::websocket_client` | Beast `flat_buffer` -- 1 frame | Yes (1) | N/A -- read one, yield, repeat |
| `local_order_book` | `std::vector<depth_event>` -- pre-sync events | No | Unbounded growth until snapshot completes |
| TCP kernel buffer | OS-level socket receive buffer | Yes (OS-tuned) | Binance drops connection |

### What's missing: application-level queues

The library does not provide:
- Bounded queues between transport and consumers
- Overflow policies (drop oldest, drop newest, block, fail)
- Async access to queued data (e.g., drain N events)
- Backpressure signaling to the transport layer

These are deliberately left to the application. Possible designs:

**Option A: cobalt::channel**
```
subscribe() generator
    |
    co_await channel.push(event)  <- suspends if full
    |
consumer: co_await channel.pop()
```
Coroutine-native, backpressure via suspension.

**Option B: Lock-free ring buffer (SPSC)**
```
generator coroutine --push--> ring_buffer --pop--> processing thread
                              (fixed size)
```
Best for high-frequency market data where latest state matters more than history.

---

## Stream Subscriptions

### Subscription types

Subscription parameter types live in `types/subscriptions.hpp`. Each subscription
type has a corresponding `stream_traits` specialization in
`streams/stream_traits.hpp`:

```cpp
// Example: book ticker subscription
types::book_ticker_subscription sub{.symbol = "BTCUSDT"};

// stream_traits resolves:
// - target: "/ws/btcusdt@bookTicker"
// - event_type: types::book_ticker_stream_event_t
auto stream = streams.subscribe(sub);
// stream is cobalt::generator<result<book_ticker_stream_event_t>>
```

### Combined stream (subscribe/unsubscribe on one connection)

```cpp
auto& streams = c.streams();

// Open a combined stream connection
co_await streams.async_connect("/stream");

// Subscribe to topics dynamically
co_await streams.async_subscribe({"btcusdt@bookTicker", "ethusdt@bookTicker"});

// Read events (all subscribed streams arrive on this connection)
while (true) {
    auto msg = co_await streams.async_read_text();
    if (!msg) break;
    // dispatch based on "stream" field in JSON
}

// Modify subscriptions without disconnecting
co_await streams.async_unsubscribe({"ethusdt@bookTicker"});
co_await streams.async_subscribe({"btcusdt@depth@100ms"});

// Check what's active
auto subs = co_await streams.async_list_subscriptions();

co_await streams.async_close();
```

**Protocol:** Subscribe/unsubscribe send JSON control messages on the same WebSocket:
```json
{"method": "SUBSCRIBE", "params": ["btcusdt@bookTicker"], "id": 1}
{"method": "UNSUBSCRIBE", "params": ["ethusdt@bookTicker"], "id": 2}
{"method": "LIST_SUBSCRIPTIONS", "params": [], "id": 3}
```

---

## Multiple Parallel Streams

### Different connections

Each `market_streams` instance owns one `websocket_client`. To run multiple streams
in parallel, create multiple `market_streams` instances:

```cpp
streams::market_streams book_stream(cfg);
streams::market_streams trade_stream(cfg);

// In cobalt::main, run both generators concurrently:
co_await cobalt::join(
    read_books(book_stream),
    read_trades(trade_stream)
);
```

### One connection (combined stream)

Binance supports up to ~200 streams on a single combined connection. Use the
combined stream pattern (see above) with raw text reads and manual dispatch based
on the `"stream"` field.

---

## User Data Streams

User streams require a listen key from the REST API:

```cpp
fapi::client c(cfg);

// Get listen key via REST
auto key = co_await c.user_data_streams.async_start();

// Subscribe returns a variant generator
auto stream = c.user_streams().subscribe(key->listenKey);

while (stream) {
    auto event = co_await stream;
    if (!event) break;

    std::visit(overloaded{
        [](const types::order_trade_update_event_t& e) {
            spdlog::info("{} {} {}", e.order.symbol, e.order.side, e.order.status);
        },
        [](const types::account_update_event_t& e) {
            spdlog::info("balance update: {} assets", e.data.balances.size());
        },
        [](const types::listen_key_expired_event_t&) {
            spdlog::warn("listen key expired");
        },
        [](const auto&) {}
    }, *event);
}
```

`user_stream_event_t` is a `std::variant` of 10 event types:
- `account_update_event_t`
- `order_trade_update_event_t`
- `margin_call_event_t`
- `listen_key_expired_event_t`
- `account_config_update_event_t`
- `trade_lite_event_t`
- `algo_order_update_event_t`
- `conditional_order_trigger_reject_event_t`
- `grid_update_event_t`
- `strategy_update_event_t`

**Event dispatch:** The `parse_event()` method detects the event type from the `"e"`
field in the raw JSON, then parses into the appropriate variant alternative.

**Listen key lifecycle:**
- `user_data_streams.async_start()` -- creates listen key (valid 60 minutes)
- `user_data_streams.async_keepalive()` -- extend validity (call every 30 minutes)
- `user_data_streams.async_close()` -- invalidate listen key
- `listen_key_expired_event_t` arrives on the stream when the server expires the key

The library does not automatically manage listen key keepalive. The application must
schedule periodic keepalive calls.

---

## Connection Lifecycle

```
                    +----------+
                    | Idle     |
                    +----+-----+
                         | async_connect()
                         v
              +----------------------+
              | Connected            |
              |  - DNS resolve       |
              |  - TCP connect       |
              |  - TLS handshake     |
              |  - WS handshake      |
              |  - ping/pong active  |
              +----------+-----------+
                         | subscribe() / async_read_*()
                         v
              +----------------------+
              | Reading              |<---- generator yields / co_await resumes
              |  - frame arrives     |
              |  - parse JSON        |
              |  - yield/return      +----> error or consumer stops
              +----------+-----------+
                         |
                         v
              +----------------------+
              | Closing              |
              |  - WS close frame    |
              |  - TCP shutdown      |
              +----------+-----------+
                         |
                         v
                    +----------+
                    | Closed   |  (no reuse -- new connect needed)
                    +----------+
```

**Ping/pong:** Installed automatically during WS handshake. Binance sends periodic
pings; the transport replies with pong synchronously in the control callback. No
application action needed.

**No automatic reconnection.** If the connection drops (network error, server
disconnect, ping timeout), the generator yields an error result. The application must
detect this and reconnect.

---

## Source Reference

| File | Role |
|---|---|
| `include/binapi2/fapi/streams/market_streams.hpp` | Market stream API: subscribe (generator), async_connect, async_read_event, combined stream control |
| `src/binapi2/fapi/streams/market_streams.cpp` | Implementation: target URL construction, async_connect, async_subscribe/unsubscribe |
| `include/binapi2/fapi/streams/user_streams.hpp` | User data stream API: subscribe (variant generator), async_connect |
| `src/binapi2/fapi/streams/user_streams.cpp` | Implementation: event type detection, parse_event, variant dispatch |
| `include/binapi2/fapi/streams/local_order_book.hpp` | Async local order book: async_run coroutine, thread-safe snapshot |
| `src/binapi2/fapi/streams/local_order_book.cpp` | Implementation: buffer -> snapshot -> apply -> gap detection |
| `include/binapi2/fapi/streams/stream_traits.hpp` | Compile-time subscription -> target path + event type mapping |
| `include/binapi2/fapi/transport/websocket_client.hpp` | Low-level WS transport: async-only connect/read/write/close |
| `include/binapi2/fapi/types/subscriptions.hpp` | Subscription parameter types (symbol, pair, intervals) |
| `include/binapi2/fapi/types/market_stream_events.hpp` | Market stream event types with glaze JSON metadata |
| `include/binapi2/fapi/types/user_stream_events.hpp` | User stream event types + user_stream_event_t variant |
| `include/binapi2/fapi/types/streams.hpp` | Convenience header: includes both market_stream_events.hpp and user_stream_events.hpp |
| `include/binapi2/fapi/config.hpp` | Endpoint URLs, stream_recorder callback |
