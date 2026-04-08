# Threading, I/O, and Coroutine Architecture

## Component Map

```
binapi2::fapi::client
|
|-- io_thread                          1 thread, 1 io_context
|     |-- io_context                   event loop (persistent, never recreated)
|     |-- work_guard                   keeps io_context::run() alive
|     `-- std::thread                  background thread running io_context::run()
|
|-- http_client                        uses io_thread&
|     `-- (per-request, no state)      each request creates: resolver, ssl_ctx, ssl_stream<tcp::socket>
|
|-- rest::account_service    --|
|-- rest::convert_service    --|
|-- rest::market_data_service--|       all REST services hold client& (non-owning)
|-- rest::trade_service      --|       they delegate to client::execute -> http_client
|-- rest::user_data_stream   --|
|
|-- websocket_api::client              lazy-init, uses io_thread&
|     `-- websocket_client (impl)      persistent: resolver, ssl_ctx, ws_stream, flat_buffer
|
|-- streams::market_streams            lazy-init, uses io_thread&
|     `-- websocket_client (impl)      persistent: resolver, ssl_ctx, ws_stream, flat_buffer
|
`-- streams::user_streams              lazy-init, uses io_thread&
      `-- websocket_client (impl)      persistent: resolver, ssl_ctx, ws_stream, flat_buffer
```

---

## Thread ↔ io_context ↔ I/O Object Mapping

### Current architecture: 1 thread, 1 io_context, N sockets

```
                        ┌─────────────────────────────────────────────┐
 User thread            │  io_thread (background)                     │
 (main/caller)          │                                             │
                        │  io_context::run()                          │
 client.execute(req)    │    ┌─────────────────────────────────┐      │
      │                 │    │ cobalt coroutine (HTTP #1)       │      │
      │ run_sync()      │    │   resolver ──► tcp::socket       │      │
      │ ──spawn──►──────│───►│   ssl::stream ──► TLS            │      │
      │    blocks       │    │   async_resolve/connect/write/   │      │
      │    on future    │    │   read then destroyed            │      │
      │◄──future.get()──│────│                                  │      │
      │                 │    └─────────────────────────────────┘      │
                        │                                             │
 ws_api().execute(req)  │    ┌─────────────────────────────────┐      │
      │ run_sync()      │    │ cobalt coroutine (WS write/read)│      │
      │ ──spawn──►──────│───►│   persistent ws_stream           │      │
      │◄──future.get()──│────│                                  │      │
      │                 │    └─────────────────────────────────┘      │
                        │                                             │
 streams().read_loop()  │    ┌─────────────────────────────────┐      │
      │ run_sync()      │    │ cobalt coroutine (WS read)      │      │
      │ ──spawn──►──────│───►│   persistent ws_stream           │      │
      │    blocks       │    │   loops until handler returns    │      │
      │    (long)       │    │   false or error                 │      │
      │◄──future.get()──│────│                                  │      │
      │                 │    └─────────────────────────────────┘      │
                        └─────────────────────────────────────────────┘
```

**Mapping table:**

| Component | io_context | Thread | Sockets | Lifetime |
|---|---|---|---|---|
| `io_thread` | owns 1 | owns 1 background | — | same as `client` |
| `http_client` | borrows from `io_thread` | runs on io_thread | 1 per request | per-request (created, used, destroyed) |
| `websocket_api::client` | borrows from `io_thread` | runs on io_thread | 1 persistent WS | from `connect()` to `close()` |
| `market_streams` | borrows from `io_thread` | runs on io_thread | 1 persistent WS | from `connect_*()` to disconnect |
| `user_streams` | borrows from `io_thread` | runs on io_thread | 1 persistent WS | from `connect()` to disconnect |

**Key invariant:** All I/O objects (resolvers, sockets, SSL streams) are created on the io_thread's executor. All completions dispatch on the io_thread's single background thread.

---

## Cobalt Context

**There is no separate "cobalt context."** Boost.Cobalt uses `asio::io_context` directly as its execution facility. The threading contract is:

> No more than one kernel thread may execute within a given io_context at a time.

This is why `io_thread` runs a single background thread. Cobalt coroutines are single-threaded by design within their io_context.

### How coroutines get their executor

Every async method acquires the executor from within the coroutine:

```cpp
// http_client.cpp:71, websocket_client.cpp:71
auto executor = co_await boost::cobalt::this_coro::executor;
```

This returns the executor of the io_context that the coroutine was spawned on (via `cobalt::spawn`). All I/O objects constructed with this executor dispatch completions back to the same io_context.

### How sync wraps async

```cpp
// io_thread.hpp
template<typename T>
T run_sync(boost::cobalt::task<T> task)
{
    auto future = boost::cobalt::spawn(io_, std::move(task), boost::asio::use_future);
    return future.get();  // blocks calling thread until coroutine completes
}
```

`cobalt::spawn(io_, task, use_future)` posts the coroutine onto `io_` and returns an `std::future`. The background thread (running `io_.run()`) drives the coroutine. The calling thread blocks on `future.get()`.

### `cobalt::use_op` — the completion token

`cobalt::use_op` bridges ASIO async operations into cobalt coroutines:

```cpp
co_await resolver.async_resolve(host, port, boost::cobalt::use_op);
co_await asio::async_connect(socket, endpoints, boost::cobalt::use_op);
co_await stream.async_handshake(ssl::stream_base::client, boost::cobalt::use_op);
```

It creates an awaitable that wraps the ASIO operation directly (no extra coroutine frame allocation, unlike `use_task`). When the operation completes, the coroutine resumes on the same executor.

---

## Connection Lifecycle

### HTTP (REST) — per-request, no reuse

```
request() ──► async_request() coroutine:
    1. make_ssl_context()           ← new SSL context
    2. tcp::resolver{ executor }    ← new resolver
    3. ssl::stream<tcp::socket>{ executor, ssl_ctx }  ← new socket
    4. co_await async_resolve()
    5. co_await async_connect()
    6. co_await async_handshake()   ← TLS
    7. co_await async_write()       ← HTTP request
    8. co_await async_read()        ← HTTP response
    9. stream.shutdown()            ← close (errors ignored)
    All objects destroyed at coroutine exit.
```

Source: `src/binapi2/fapi/transport/http_client.cpp:56-162`

### WebSocket — persistent, long-lived

```
connect() ──► async_connect() coroutine:
    1. resolver.emplace(executor)          ← lazy, stored in impl
    2. stream.emplace(executor, ssl_ctx)   ← lazy, stored in impl
    3. co_await async_resolve()
    4. co_await async_connect()
    5. install ping/pong handler
    6. co_await ssl handshake
    7. co_await ws handshake
    Objects survive in impl_ across calls.

write_text() / read_text() ──► reuse same stream
run_read_loop() ──► blocking loop of read_text() calls
close() ──► async_close(), sets connected=false
```

Source: `src/binapi2/fapi/transport/websocket_client.cpp:65-253`

---

## Can we have multiple threads on one io_context?

**Not with Cobalt.** Cobalt's design assumes single-threaded execution per io_context. Its synchronization primitives (channels, select, etc.) avoid locking specifically because of this guarantee.

Calling `io_context::run()` from multiple threads would violate this invariant and cause undefined behavior in cobalt coroutines.

**What about strands?** Strands are incompatible with cobalt's thread-local executor model. If you use a strand, you must explicitly pass the executor via `asio::executor_arg_t`:

```cpp
cobalt::promise<void> work(int arg,
                           asio::executor_arg_t,
                           cobalt::executor exec);
```

Without this, cobalt uses the thread-local default executor, which breaks when a strand switches threads.

## Can we have multiple io_contexts in one thread?

**Technically yes** (alternate polling), but it defeats the purpose. The io_thread pattern gives each io_context its own dedicated thread. If you need multiple event loops, use multiple io_threads.

In binapi2 there is exactly one io_context (owned by `io_thread`) and one background thread per `client` instance.

---

## Connection Pooling

### What it means in our context

Connection pooling would mean reusing TCP+TLS connections across multiple HTTP requests instead of opening a fresh connection for each one. Currently:

- **HTTP (REST):** Every `request()` call performs the full cycle: DNS resolve → TCP connect → TLS handshake → HTTP write → HTTP read → shutdown. No connection reuse.
- **WebSocket:** Inherently persistent — one connect, many reads/writes. Pooling is not applicable.

### Current cost per REST request

```
resolve     ~1-5ms   (DNS lookup, typically cached by OS)
tcp_connect ~10-50ms (depends on network latency to Binance)
tls_handshake ~20-80ms (TLS 1.3 full handshake)
http_write  ~1ms
http_read   ~10-100ms (server processing + response transfer)
shutdown    ~1ms
```

The TLS handshake dominates. With connection reuse (HTTP keep-alive), subsequent requests on the same connection skip resolve + connect + TLS, saving 30-130ms per request.

### When would we need it?

| Scenario | Pooling needed? |
|---|---|
| Occasional market data queries | No — latency overhead acceptable |
| Burst of REST requests (e.g., batch account queries) | Beneficial — saves repeated TLS handshakes |
| High-frequency order placement via REST | Yes — or better, use WebSocket API instead |
| WebSocket API (already persistent) | N/A — single connection, already optimal |
| Market streams (already persistent) | N/A |

### How it would work

```
http_client would maintain:
    pool: map<(host,port), idle_connection>
    idle_connection: { ssl_stream, last_used_time }

async_request():
    if pool has idle connection for (host, port):
        reuse it (skip resolve/connect/handshake)
        on error: discard, create new
    else:
        create new connection (current behavior)
    after response:
        if keep-alive: return to pool
        else: close

reaper thread/timer:
    close connections idle > N seconds
```

HTTP/1.1 keep-alive requires sending `Connection: keep-alive` and checking the response header. Binance servers support it.

### Recommendation

Not needed now. The WebSocket API (`ws_api()`) already provides persistent low-latency access for trading. For REST-heavy workloads, connection pooling would be a targeted optimization when profiling shows TLS handshake overhead is significant.

---

## Thread Pool

### Can Cobalt use a thread pool?

**Yes.** Cobalt supports `asio::thread_pool` via explicit executor passing:

```cpp
boost::asio::thread_pool pool{4};

// Spawn a cobalt task on a strand of the pool
auto strand = boost::asio::make_strand(pool.get_executor());
cobalt::spawn(strand, cpu_heavy_task(), detached);
```

The key requirement: each cobalt coroutine must run on at most one thread at a time (the strand guarantees this).

### How it would apply to binapi2

```
Current:
    io_thread { 1 io_context, 1 thread }
    all coroutines serialized on that thread

Thread pool variant:
    thread_pool { 1 io_context, N threads }
    each coroutine runs on a strand
    coroutines can run concurrently on different strands
```

### Benefits

| Benefit | When it matters |
|---|---|
| Concurrent REST requests | Multiple independent API calls in parallel (e.g., query positions + query balances + query open orders simultaneously) |
| CPU-bound JSON parsing off the I/O thread | Large responses (exchange_info: ~100KB, all-tickers: ~150KB) |
| Non-blocking stream reads while REST is in flight | Prevents market data stalls during REST bursts |

### Current limitation

With the single-threaded `io_thread`, operations are serialized:

```
Time ──►
Thread: [REST req1: resolve+connect+tls+write+read] [REST req2: resolve+connect+tls+write+read]
                                                     ↑ blocked until req1 completes
```

With a thread pool + strands:

```
Time ──►
Strand1: [REST req1: resolve+connect+tls+write+read]
Strand2: [REST req2: resolve+connect+tls+write+read]  ← concurrent
Strand3: [WS stream: read...read...read...]            ← unblocked
```

### Implementation sketch

```cpp
class io_pool
{
    boost::asio::thread_pool pool_;
public:
    explicit io_pool(int threads = 1) : pool_(threads) {}
    ~io_pool() { pool_.join(); }

    auto get_executor() { return pool_.get_executor(); }

    template<typename T>
    T run_sync(boost::cobalt::task<T> task)
    {
        auto strand = boost::asio::make_strand(pool_.get_executor());
        auto future = boost::cobalt::spawn(strand, std::move(task),
                                           boost::asio::use_future);
        return future.get();
    }
};
```

### Recommendation

The single-threaded `io_thread` is correct for now. Thread pool becomes relevant when:

1. You need concurrent REST requests (parallel queries at startup, bulk operations)
2. Stream reads must not block on REST calls (latency-sensitive trading)
3. Profiling shows the single io_thread is a bottleneck

The migration would be minimal: replace `io_thread` internals with `thread_pool` + strands, keeping the same `run_sync()` interface.

---

## Summary Table

| Question | Answer |
|---|---|
| Threads per client | 1 background (io_thread) + caller thread |
| io_contexts per client | 1 (owned by io_thread) |
| Cobalt context? | No — cobalt uses io_context directly |
| Sockets per REST request | 1 (created and destroyed per request) |
| Sockets per WS component | 1 persistent (ws_api, market_streams, user_streams each own one) |
| Max concurrent sockets | 1 REST (serialized) + 3 WS = 4 |
| Multiple threads on 1 io_context? | Not with cobalt (single-threaded invariant) |
| Multiple io_contexts on 1 thread? | Possible but not used |
| Connection pooling | Not implemented; not needed (WS API covers low-latency) |
| Thread pool | Supported by cobalt; future optimization when concurrency needed |

---

## Source References

| File | Role |
|---|---|
| `include/binapi2/fapi/detail/io_thread.hpp` | Thread + io_context ownership, `run_sync()` |
| `include/binapi2/fapi/client.hpp` | Unified client, owns io_thread and all components |
| `src/binapi2/fapi/client.cpp` | Client construction, lazy WS init |
| `src/binapi2/fapi/transport/http_client.cpp` | Per-request connection lifecycle |
| `src/binapi2/fapi/transport/websocket_client.cpp` | Persistent WS connection, pimpl, read loop |
| `include/binapi2/fapi/transport/session_base.hpp` | Shared config base for both transports |
| `src/binapi2/fapi/websocket_api/client.cpp` | WS API RPC dispatch, auth injection |
| `src/binapi2/fapi/streams/market_streams.cpp` | Stream connect + typed read loops |
| `src/binapi2/fapi/streams/user_streams.cpp` | Multiplexed event dispatch |
| `include/binapi2/fapi/rest/service.hpp` | Service base, delegates to client |
