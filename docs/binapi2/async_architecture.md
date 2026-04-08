# Async-First Architecture Analysis

## Current Client Responsibilities

`fapi::client` currently does too many things:

| Responsibility | What it does |
|---|---|
| Service container | Holds 5 REST services + 3 lazy WS components |
| Query pipeline | Signing, timestamp injection, query string encoding |
| HTTP transport owner | Owns `http_client`, delegates requests |
| Execution bridge | Owns `io_thread`, provides `run_sync()` |
| Response decoding | JSON deserialization via `decode_response<T>` |
| Mode selection | Chooses sync vs async-only at construction |

These should be separated.

---

## API Types

### REST — request/response over HTTP

- Each request: build query → sign → HTTP request → HTTP response → parse JSON
- Authentication: API key header + HMAC-SHA256 signature per request
- Connection: currently one fresh TCP+TLS connection per request (no pooling)
- Stateless: no session, no login

### WebSocket API — request/response over persistent WebSocket

- Connect once, then logon once (session auth)
- Subsequent requests: JSON-RPC envelope → send frame → read frame → parse response
- Connection: single persistent WebSocket, reused for all requests
- Stateful: session logon, request ID correlation

### Streams — connect and receive

- Connect to endpoint, then receive-only (no request after connect)
- Single stream: one WebSocket per stream topic
- Combined stream: one WebSocket, multiple topics via subscribe/unsubscribe JSON commands
- User data stream: one WebSocket per listen key, multiplexed event types

---

## Execution Variants

The core implementation is always **async coroutine** (`cobalt::task`).  
Everything else is a wrapper that provides the result to the caller differently.

### Matrix: request/response APIs (REST, WS API)

These are single-shot: one request in, one response out.

| Call model | REST | WS API |
|---|---|---|
| **true async** | `co_await async_execute(req)` → `result<T>` | `co_await async_execute(req)` → `result<T>` |
| **future** | `spawn(exec, task, use_future)` → `future.get()` | same |
| **callback** | `spawn(exec, task, callback)` | same |
| **sync** | `io.run_sync(async_execute(req))` → blocks | same |

### Matrix: streams

Streams are not request/response — they are subscriptions producing a continuous sequence of events. A future cannot represent an infinite sequence.

| Call model | How it works |
|---|---|
| **async loop** | `while (auto msg = co_await async_read_text()) { process(msg); }` |
| **callback/handler** | `read_loop(handler)` — handler called per event, returns bool |
| **channel/queue** | async producer → bounded buffer → consumer (not implemented) |
| **sync loop** | `read_loop(handler)` — blocks calling thread until handler returns false |
| **future** | **N/A** — no single result to return |

### What each call model needs

| Model | Executor | Blocking? | Thread |
|---|---|---|---|
| true async | Caller provides (cobalt::main, io_context) | No | Runs on caller's event loop |
| future | Executor to drive task | Caller blocks on `future.get()` | Task runs on executor thread |
| callback | Executor to post completion | No | Callback invoked on executor thread |
| sync | `io_thread` (executor + thread) | Yes | Dedicated background thread |

**Key insight:** All models need the same thing — an executor to run the async task. They differ only in **how the caller receives the result**. Streams add a second dimension: how the caller **consumes** multiple results over time.

---

## Proposed Architecture

### Layer 1: Pure functions (no I/O, no state)

```
signing.hpp     — hmac_sha256_hex, sign_query, inject_auth_query, build_query_string
query.hpp       — to_query_map<T> (reflection-based serialization)
json_opts.hpp   — parse options
decode.hpp      — decode_response<T> (JSON → typed result)
```

No executor, no transport, no config. Pure CPU.

### Layer 2: Async I/O primitives (coroutines, no execution policy)

```
transport::websocket_client   — async_connect, async_read_text, async_write_text, async_close
transport::http_client        — async_request
```

These are `cobalt::task`-returning coroutines. They use `co_await this_coro::executor` to get their executor from whoever drives them. **They don't own an executor.**

### Layer 3: Protocol logic (coroutines, uses Layer 1 + 2)

```
rest::pipeline         — async_request(config, method, path, query, signed) → task<result<T>>
                         combines: sign → build query → http_client::async_request → decode

websocket_api::session — async_connect, async_logon, async_execute(req) → task<result<T>>
                         combines: connect → logon → send_rpc → read → decode

stream::connection     — async_connect(target), async_read_text() → task<result<string>>
                         thin wrapper over transport with config-driven host/port
```

All return `cobalt::task`. No executor ownership. No sync wrappers.

**This is the true async API.** Anyone with an executor can `co_await` these.

### Layer 4: Execution policy (owns executor, bridges to caller)

```
executor (concept/interface):
    spawn(task<T>) → future<T>

io_thread : executor
    owns io_context + background thread
    spawn(task) → blocks caller via future.get()

// Not in library — user provides:
cobalt::main         — cobalt provides executor, user co_awaits directly
thread_pool          — multiple threads, strand per task
inline_executor      — run on caller's thread (for testing)
```

### Layer 5: Convenience facade (optional)

```
client {
    config cfg;
    rest::pipeline rest;
    websocket_api::session ws_api;  // lazy
    stream::connection streams;     // lazy

    // No executor — user provides one:
    // - co_await client.rest.async_execute(req)         from cobalt::main
    // - io.run_sync(client.rest.async_execute(req))     from sync code
}
```

The client is just a **container** — it holds config and typed accessors. It does not own execution.

---

## Separation: Execution Context vs Call Model

### Execution context (who provides the event loop)

| Context | Who creates io_context | Who runs it | Lifetime |
|---|---|---|---|
| `cobalt::main` | cobalt runtime | cobalt runtime | program lifetime |
| `io_thread` | library/user | dedicated background thread | object lifetime |
| `thread_pool` | user | N worker threads | user-controlled |
| `io_context` (manual) | user | user calls `io_context.run()` | user-controlled |

### Call model (how caller gets the result)

| Model | Signature | Caller does |
|---|---|---|
| true async | `task<result<T>> async_execute(req)` | `co_await` |
| future | spawn on executor → `std::future<result<T>>` | `future.get()` (blocks) |
| callback | spawn on executor, post callback on completion | register callback, continue |
| sync | create io_thread, spawn, block on future | call and wait |

### The library should provide Layer 3 (true async) as the primary API.

Bridging (Layer 4) is a one-liner:
```cpp
// True async — in a coroutine
auto r = co_await pipeline.async_execute(req);

// Future — from any thread
auto future = cobalt::spawn(executor, pipeline.async_execute(req), use_future);
auto r = future.get();

// Callback — from any thread  
cobalt::spawn(executor, pipeline.async_execute(req), [](auto r) { handle(r); });

// Sync — convenience
auto r = io_thread.run_sync(pipeline.async_execute(req));
```

The library doesn't need four variants of every method. It needs **one** (`cobalt::task`) and a clear way to provide an executor.

---

## What About `client::run_sync`?

It duplicates `io_thread::run_sync`. Remove it. If the user wants sync:

```cpp
detail::io_thread io;
client c(cfg);  // no io_thread ownership

auto r = io.run_sync(c.rest.async_execute(req));
```

The client doesn't need to know about execution policy.

---

## Production Setup Considerations

### REST

| Aspect | Current | Ideal |
|---|---|---|
| Connections | 1 per request (no reuse) | Connection pool with keep-alive |
| Threading | All on io_thread | Configurable: single thread or pool |
| Priority | None | Separate connections for priority orders |
| Rate limiting | None (server-side only) | Client-side rate limiter |

### WebSocket API

| Aspect | Current | Ideal |
|---|---|---|
| Connection | 1 persistent | 1 persistent + spare for failover |
| Request pipelining | Sequential (write → read) | Concurrent with ID correlation |
| Reconnection | None | Automatic with backoff |

### Streams

| Aspect | Current | Ideal |
|---|---|---|
| Connection | 1 per stream or combined | Combined preferred (fewer connections) |
| Parsing | Same thread as I/O | Option to offload to parser thread |
| Buffering | None (direct handler call) | Optional bounded queue between I/O and handler |
| Backpressure | Handler blocks I/O thread | Queue overflow policy |

### Thread model options

**Option A: Single I/O thread (current)**
```
[io_thread] ── REST request ── parse ── deliver
            ── WS API write/read ── parse ── deliver
            ── Stream read ── parse ── deliver
```
Simple but all I/O serialized. Slow handler blocks everything.

**Option B: Thread per protocol**
```
[rest_thread]   ── REST I/O ── parse ── deliver
[ws_api_thread] ── WS API I/O ── parse ── deliver
[stream_thread] ── Stream I/O ── parse ── deliver
```
Isolation between protocols. Stream processing can't starve REST.

**Option C: Pipeline stages**
```
[io_thread]     ── read raw frames ──► queue ──► [parse_thread] ── parse ──► queue ──► [logic_thread]
```
Maximum throughput but complex. Useful for high-frequency stream processing.

**The library should not impose a threading model.** It provides async coroutines (Layer 3). The user composes them with their chosen executor and threading strategy.

---

## Local Order Book in Async Environment

Current `local_order_book::start()` is blocking — it runs a sync read loop internally.

Async version would be a coroutine:

```cpp
// Async local order book — user drives it
cobalt::task<void> run_order_book(client& c, symbol_t symbol) {
    auto& streams = c.streams();
    
    // Connect to diff depth stream
    co_await streams.async_connect("/ws/" + symbol.str() + "@depth@100ms");
    
    order_book_state book;
    
    while (true) {
        auto msg = co_await streams.async_read_text();
        if (!msg) break;
        
        auto event = parse<depth_stream_event_t>(*msg);
        
        if (!book.synced) {
            book.buffer.push_back(event);
            if (book.buffer.size() == 1) {
                // First event — fetch snapshot
                auto snap = co_await c.market_data.async_execute(
                    order_book_request_t{.symbol = symbol, .limit = 1000});
                book.apply_snapshot(*snap, book.buffer);
            }
        } else {
            if (!book.apply(event)) {
                // Gap detected — resync
                book.reset();
                continue;
            }
            on_update(book.snapshot());
        }
    }
}
```

This is a coroutine that the user spawns on their executor. No internal threads, no blocking, no mutex needed (single-threaded coroutine). The snapshot callback becomes just code after the apply.

---

## Summary

| Principle | Implication |
|---|---|
| Async-first | Primary API returns `cobalt::task<result<T>>` |
| No executor ownership | Client/services don't own io_thread |
| User provides executor | `cobalt::main`, `io_thread`, `thread_pool` — user's choice |
| Bridging is trivial | `io.run_sync(task)`, `spawn(exec, task, use_future)`, `spawn(exec, task, callback)` |
| Client is a container | Holds config + typed protocol objects, no execution logic |
| Layer separation | Pure functions → async I/O → protocol logic → execution policy → facade |
