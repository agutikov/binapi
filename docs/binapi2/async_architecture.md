# Async-First Architecture

This document describes the current async-first architecture of binapi2.

---

## API Types

### REST -- request/response over HTTP

- Each request: build query -> sign -> HTTP request -> HTTP response -> parse JSON
- Authentication: API key header + HMAC-SHA256 signature per request
- Connection: one fresh TCP+TLS connection per request (no pooling)
- Stateless: no session, no login

### WebSocket API -- request/response over persistent WebSocket

- Connect once, then logon once (session auth)
- Subsequent requests: JSON-RPC envelope -> send frame -> read frame -> parse response
- Connection: single persistent WebSocket, reused for all requests
- Stateful: session logon, request ID correlation
- Auth mode dispatched per request via `ws_auth_mode` enum in traits

### Streams -- connect and receive

- Connect to endpoint, then receive-only (no request after connect)
- Generator pattern: `subscribe(subscription)` returns `cobalt::generator<result<Event>>`
- User data stream: `subscribe(listen_key)` returns `cobalt::generator<result<user_stream_event_t>>`
  where `user_stream_event_t` is a `std::variant` of 10 event types

---

## Execution Variants

The core implementation is always **async coroutine** (`cobalt::task`).
Everything else is a wrapper that provides the result to the caller differently.

### Matrix: request/response APIs (REST, WS API)

These are single-shot: one request in, one response out.

| Call model | REST | WS API |
|---|---|---|
| **true async** | `co_await async_execute(req)` -> `result<T>` | `co_await async_execute(req)` -> `result<T>` |
| **future** | `spawn(exec, task, use_future)` -> `future.get()` | same |
| **callback** | `spawn(exec, task, callback)` | same |
| **sync** | `io.run_sync(async_execute(req))` -> blocks | same |

### Matrix: streams

Streams produce a continuous sequence of events. The generator pattern is primary.

| Call model | How it works |
|---|---|
| **generator** (recommended) | `auto g = streams.subscribe(sub); while (g) { auto e = co_await g; }` |
| **async loop** | `co_await async_connect(sub); while (...) { auto e = co_await async_read_event<E>(); }` |
| **raw text** | `co_await async_connect(target); while (...) { auto msg = co_await async_read_text(); }` |
| **future** | **N/A** -- generators cannot be driven via `use_future` |

### What each call model needs

| Model | Executor | Blocking? | Thread |
|---|---|---|---|
| true async | Caller provides (cobalt::main, io_context) | No | Runs on caller's event loop |
| future | Executor to drive task | Caller blocks on `future.get()` | Task runs on executor thread |
| callback | Executor to post completion | No | Callback invoked on executor thread |
| sync | `io_thread` (executor + thread) | Yes | Dedicated background thread |

**Key insight:** All models need the same thing -- an executor to run the async task.
They differ only in **how the caller receives the result**. Streams add a second
dimension: how the caller **consumes** multiple results over time.

---

## Architecture Layers

### Layer 1: Pure functions (no I/O, no state)

```
signing.hpp     -- hmac_sha256_hex, sign_query, inject_auth_query, build_query_string
query.hpp       -- to_query_map<T> (reflection-based serialization)
json_opts.hpp   -- parse options
decode.hpp      -- decode_response<T> (JSON -> typed result)
```

No executor, no transport, no config. Pure CPU.

### Layer 2: Async I/O primitives (coroutines, no execution policy)

```
transport::http_client        -- config-only constructor, async_request only
transport::websocket_client   -- config-only constructor, async_connect, async_read_text,
                                 async_write_text, async_close
```

These are `cobalt::task`-returning coroutines. They use `co_await this_coro::executor`
to get their executor from whoever drives them. **They don't own an executor.**

### Layer 3: Protocol logic (coroutines, uses Layer 1 + 2)

```
rest::pipeline                -- sign -> build query -> http_client::async_request -> decode
                                 Generic async_execute<Request>() resolves endpoint_traits.

websocket_api::client         -- async_connect -> async_session_logon -> async_execute(req)
                                 JSON-RPC envelope, auth mode dispatch via if constexpr,
                                 4 auth modes: inject, none, signed_base, api_key_only.

streams::market_streams       -- subscribe(subscription) -> generator<result<Event>>
                                 stream_traits<Subscription>::target() builds WS URL,
                                 stream_traits<Subscription>::event_type is the parsed type.

streams::user_streams         -- subscribe(listen_key) -> generator<result<user_stream_event_t>>
                                 Detects event type from "e" field, parses into variant.

streams::local_order_book     -- async_run(symbol, depth_limit) -> task<result<void>>
                                 Takes market_streams& + rest::pipeline& references.
```

All return `cobalt::task` or `cobalt::generator`. No executor ownership. No sync
wrappers. **This is the true async API.**

### Layer 4: Execution policy (user-provided, NOT library-owned)

```
detail::io_thread            -- User creates; owns io_context + background thread.
                                run_sync(task<T>) -> T blocks caller via future.get().

cobalt::main                 -- Cobalt provides executor. User co_awaits directly.
                                Required for generator pattern (streams).

thread_pool                  -- Multiple threads, strand per task.
                                spawn(pool, task, use_future) -> future.

manual io_context            -- User creates + runs io_context.
                                spawn(io, task, use_future) -> future.
```

### Layer 5: Convenience facade

```
client {
    config cfg;
    http_client http;
    rest::pipeline pipeline;

    // 5 REST services (hold pipeline&):
    account_service account;
    convert_service convert;
    market_data_service market_data;
    trade_service trade;
    user_data_stream_service user_data_streams;

    // Lazy WebSocket components:
    websocket_api::client& ws_api();
    streams::market_streams& streams();
    streams::user_streams& user_streams();
}
```

The client is a **pure container** -- it holds config, transport, pipeline, and typed
accessors. It does not own execution. Single constructor: `client(config)`.

---

## Separation: Execution Context vs Call Model

### Execution context (who provides the event loop)

| Context | Who creates io_context | Who runs it | Lifetime |
|---|---|---|---|
| `cobalt::main` | cobalt runtime | cobalt runtime | program lifetime |
| `io_thread` | user | dedicated background thread | object lifetime |
| `thread_pool` | user | N worker threads | user-controlled |
| `io_context` (manual) | user | user calls `io_context.run()` | user-controlled |

### Call model (how caller gets the result)

| Model | Signature | Caller does |
|---|---|---|
| true async | `task<result<T>> async_execute(req)` | `co_await` |
| future | spawn on executor -> `std::future<result<T>>` | `future.get()` (blocks) |
| callback | spawn on executor, post callback on completion | register callback, continue |
| sync | create io_thread, spawn, block on future | call and wait |

### The library provides Layer 3 (true async) as the primary API.

Bridging (Layer 4) is a one-liner:

```cpp
// True async -- in a coroutine
auto r = co_await pipeline.async_execute(req);

// Future -- from any thread
auto future = cobalt::spawn(executor, pipeline.async_execute(req), use_future);
auto r = future.get();

// Callback -- from any thread
cobalt::spawn(executor, pipeline.async_execute(req), [](auto r) { handle(r); });

// Sync -- convenience
auto r = io_thread.run_sync(pipeline.async_execute(req));
```

The library provides one method variant (`cobalt::task`) and the user provides the
execution environment.

---

## Production Setup Considerations

### REST

| Aspect | Current | Ideal |
|---|---|---|
| Connections | 1 per request (no reuse) | Connection pool with keep-alive |
| Threading | User-provided executor | Configurable: single thread or pool |
| Priority | None | Separate connections for priority orders |
| Rate limiting | None (server-side only) | Client-side rate limiter |

### WebSocket API

| Aspect | Current | Ideal |
|---|---|---|
| Connection | 1 persistent | 1 persistent + spare for failover |
| Request pipelining | Sequential (write -> read) | Concurrent with ID correlation |
| Reconnection | None | Automatic with backoff |

### Streams

| Aspect | Current | Ideal |
|---|---|---|
| Connection | 1 per stream or combined | Combined preferred (fewer connections) |
| Parsing | Same coroutine as I/O | Option to offload to parser task |
| Buffering | None (generator yields directly) | Optional bounded channel between I/O and consumer |
| Backpressure | Consumer pace controls I/O | Channel overflow policy |

### Thread model options

**Option A: Single event loop (cobalt::main)**
```
cobalt::main ── REST request ── parse ── deliver
             ── WS API write/read ── parse ── deliver
             ── Stream generator ── parse ── yield
```
Simple. All I/O serialized on one thread. Natural for generators.

**Option B: Thread per protocol**
```
[rest_io]       ── REST I/O ── parse ── deliver
[ws_api_io]     ── WS API I/O ── parse ── deliver
[stream_io]     ── Stream I/O ── parse ── deliver
```
Isolation between protocols. Stream processing can't starve REST.

**Option C: Pipeline stages**
```
[io_task]       ── read raw frames ──> channel ──> [parse_task] ── parse ──> channel ──> [logic_task]
```
Maximum throughput but complex. Useful for high-frequency stream processing.

**The library does not impose a threading model.** It provides async coroutines
(Layer 3). The user composes them with their chosen executor and threading strategy.

---

## Summary

| Principle | Implication |
|---|---|
| Async-first | Primary API returns `cobalt::task<result<T>>` or `cobalt::generator<result<T>>` |
| No executor ownership | Client/services don't own io_thread or io_context |
| User provides executor | `cobalt::main`, `io_thread`, `thread_pool` -- user's choice |
| Bridging is trivial | `io.run_sync(task)`, `spawn(exec, task, use_future)`, `spawn(exec, task, callback)` |
| Client is a container | Holds config + typed protocol objects, no execution logic |
| Layer separation | Pure functions -> async I/O -> protocol logic -> execution policy -> facade |
| Generator for streams | `subscribe()` returns `cobalt::generator`, natural async iteration |
| Auth mode dispatch | WS API uses `ws_auth_mode` enum in traits, resolved via `if constexpr` |
