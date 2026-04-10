# WebSocket Streams Architecture

## Overview

binapi2 provides four stream components:

| Component | Purpose | Connection | Events |
|---|---|---|---|
| `streams::market_stream` | Single-subscription market data | 1 WS, 1 typed event | book ticker, depth, kline, etc. |
| `streams::combined_market_stream` | Multi-subscription market data | 1 WS, variant events | Multiple event types via std::variant |
| `streams::user_stream` | Private account events | 1 WS per listen key | orders, balances, margin calls (variant) |
| `order_book::local_order_book` | Synchronized order book | Uses market_stream + market_data_service | Depth snapshots |

All are built on `streams::stream_connection` which wraps `transport::websocket_client`.
All methods return `cobalt::task` or `cobalt::generator`.

---

## Usage Patterns

### Pattern 1: Generator (recommended)

The `subscribe()` method connects to the stream and returns a typed async generator:

```cpp
futures_usdm_api api(cfg);
auto streams = api.create_market_stream();

// subscribe() connects and returns a generator<result<Event>>
auto gen = streams->subscribe(types::book_ticker_subscription{.symbol = "BTCUSDT"});

while (gen) {
    auto event = co_await gen;
    if (!event) {
        spdlog::error("stream error: {}", event.err.message);
        break;
    }
    spdlog::info("{} bid={} ask={}", event->symbol, event->best_bid_price, event->best_ask_price);
}
```

The subscription type is resolved at compile time via `stream_traits<Subscription>`:
- `stream_traits<Subscription>::target(cfg, sub)` builds the WS target URL
- `stream_traits<Subscription>::event_type` is the parsed event type

The generator yields `result<Event>` until an error occurs or the caller stops consuming.

**Requires `cobalt::main`** or an equivalent event loop that can drive generators.
Generators cannot be bridged through `io_thread::run_sync()`.

See `examples/binapi2/fapi/async-demo-cli/` for working examples.

### Pattern 2: Raw stream_connection

For maximum control (custom parsing, combined streams, debugging), use
`stream_connection` directly:

```cpp
streams::stream_connection conn(cfg);
co_await conn.async_connect(cfg.stream_host, cfg.stream_port, "/ws/btcusdt@bookTicker");

while (true) {
    auto msg = co_await conn.async_read_text();
    if (!msg) break;
    // *msg is a raw JSON string — parse it yourself
}

co_await conn.async_close();
```

### Pattern 3: Local order book

The local order book is an async coroutine that maintains a synchronized order book:

```cpp
futures_usdm_api api(cfg);
auto streams = api.create_market_stream();
auto rest = co_await api.create_rest_client();

order_book::local_order_book book(*streams, (*rest)->market_data);

book.set_snapshot_callback([](const order_book::order_book_snapshot& snap) {
    spdlog::info("bids: {} asks: {}", snap.bids.size(), snap.asks.size());
});

// async_run is a coroutine — runs until stop() or error
auto result = co_await book.async_run(types::symbol_t{"BTCUSDT"}, 1000);
```

Internally:
1. Subscribes to `@depth@100ms` diff stream via `market_stream::subscribe()`
2. Buffers incoming typed `depth_stream_event_t` events
3. Fetches REST snapshot via `market_data_service` (`/fapi/v1/depth`)
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

### Combined stream (multiple subscriptions, one connection)

`combined_market_stream` connects to the `/stream` combined endpoint and manages
multiple subscriptions on a single WebSocket. Events arrive as a caller-specified
`std::variant`, dispatched by the `"stream"` field in the JSON wrapper.

```cpp
futures_usdm_api api(cfg);
auto combined = api.create_combined_market_stream();

// Define the variant of event types you want to receive
using events_t = std::variant<
    types::book_ticker_stream_event_t,
    types::depth_stream_event_t>;

// subscribe() connects, sends SUBSCRIBE, and yields typed events
auto gen = combined->subscribe<events_t>(
    types::book_ticker_subscription{.symbol = "BTCUSDT"},
    types::diff_book_depth_subscription{.symbol = "BTCUSDT", .speed = "100ms"});

while (gen) {
    auto event = co_await gen;
    if (!event) break;
    std::visit(overloaded{
        [](const types::book_ticker_stream_event_t& e) {
            // handle book ticker
        },
        [](const types::depth_stream_event_t& e) {
            // handle depth update
        },
    }, *event);
}
```

The variant type must include exactly the event types corresponding to the
subscriptions passed to `subscribe()`. The mapping from subscription type to
event type is resolved at compile time via `stream_traits<Subscription>::event_type`.

Internally, each incoming frame is a JSON wrapper `{"stream": "topic", "data": {...}}`.
The `"stream"` field is matched against the subscription topics, the `"data"` field
is parsed into the correct variant alternative using `glz::raw_json` passthrough.

**Standalone control methods** are also available for advanced use (without the generator):

```cpp
auto combined = api.create_combined_market_stream();
co_await combined->async_connect();
co_await combined->async_subscribe(
    types::book_ticker_subscription{.symbol = "BTCUSDT"});
// ... read raw frames via combined->connection().async_read_text() ...
co_await combined->async_unsubscribe(
    types::book_ticker_subscription{.symbol = "BTCUSDT"});
auto subs = co_await combined->async_list_subscriptions();
co_await combined->async_close();
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

Binance supports up to ~200 streams on a single combined connection. Use
`combined_market_stream` (see above) with a `std::variant` of the event types
you want to receive.

---

## User Data Streams

User streams require a listen key from the REST API:

```cpp
futures_usdm_api api(cfg);
auto rest = co_await api.create_rest_client();

// Get listen key via REST (user_data_stream_service)
auto key = co_await (*rest)->user_data_streams.async_execute(types::start_listen_key_request_t{});

// Subscribe returns a variant generator
auto user = api.create_user_stream();
auto gen = user->subscribe(key->listenKey);

while (gen) {
    auto event = co_await gen;
    if (!event) break;

    std::visit(overloaded{
        [](const types::order_trade_update_event_t& e) {
            spdlog::info("{} {} {}", e.order.symbol, e.order.side, e.order.status);
        },
        [](const types::account_update_event_t& e) {
            spdlog::info("balance update: {} assets", e.update_data.balances.size());
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

---

# Planned Architecture Redesign

## Problems with current design

`basic_market_streams` conflates three concerns:

1. **Connection** — owns `websocket_client`, manages connect/close
2. **Subscription management** — subscribe/unsubscribe control messages
3. **Event parsing** — `glz::read` inside the generator body

These are tightly coupled: one connection = one `subscribe()` call = one event type.
The combined stream path (`async_subscribe` + `async_read_text`) exists but drops
back to raw strings — no parsing, no typing.

Other issues:

- `async_connect(string target)` — raw URL path leaked into the stream API
- `async_read_event<E>()` — redundant with the generator
- `stream_recorder` — synchronous callback blocks the stream
- No auto-reconnect, no listen key keepalive, no application buffering

## Current Architecture

```
stream_connection          (raw WebSocket transport, protocol-agnostic)
├── market_stream          (single subscription, typed generator)
├── combined_market_stream (multiple subscriptions, variant generator, demux)
└── user_stream            (single listen key, variant generator)

order_book::local_order_book  (uses market_stream + market_data_service)
```

### Layered design

```
┌─────────────────────────────────────────────────────┐
│  stream_connection                                   │
│  - owns websocket_client                            │
│  - async_connect(host, port, target)                │
│  - async_read_text() / async_write_text()           │
│  - async_close()                                    │
│  - no Binance protocol knowledge                    │
└──────────────┬──────────────────────────────────────┘
               │
    ┌──────────┼──────────┬─────────────────┐
    │          │          │                 │
    ▼          ▼          ▼                 ▼
market_    combined_   user_          (direct use by
stream     market_     stream        local_order_book,
           stream                    custom components)
```

### Missing features (future)

| Feature | Notes |
|---------|-------|
| Application buffering | `cobalt::channel` or ring buffer between generator and consumer |
| Auto-reconnect | In `subscribe()` generator: catch error, reconnect, continue |
| Listen key keepalive | Periodic REST call on async timer in user_stream |
| Async recorder | Non-blocking frame recording on stream_connection |

---

## Source Reference

| File | Role |
|---|---|
| `include/binapi2/fapi/streams/stream_connection.hpp` | Raw WebSocket connection: connect/close/read/write |
| `include/binapi2/fapi/streams/market_stream.hpp` | Single-subscription market stream with typed generator |
| `include/binapi2/fapi/streams/combined_market_stream.hpp` | Multi-subscription stream with variant dispatch |
| `include/binapi2/fapi/streams/user_stream.hpp` | User data stream with variant generator |
| `include/binapi2/fapi/streams/stream_traits.hpp` | Compile-time subscription → topic/target/event_type mapping |
| `include/binapi2/fapi/order_book/local_order_book.hpp` | Synchronized order book (market_stream + market_data_service) |
| `include/binapi2/fapi/types/subscriptions.hpp` | Subscription parameter types |
| `include/binapi2/fapi/types/market_stream_events.hpp` | Market stream event types with glaze metadata |
| `include/binapi2/fapi/types/user_stream_events.hpp` | User stream event types + user_stream_event_t variant |
| `include/binapi2/fapi/types/event_traits.hpp` | Compile-time event type → wire name mapping |
| `include/binapi2/fapi/detail/variant_parse.hpp` | Generic discriminator-based variant JSON parser |
| `include/binapi2/fapi/transport/websocket_client.hpp` | Low-level WS transport (pimpl over Beast) |
| `include/binapi2/fapi/config.hpp` | Endpoint URLs, ws_target_t, listen_key_t |
