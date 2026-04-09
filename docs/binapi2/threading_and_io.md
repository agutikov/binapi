# Threading, I/O, and Coroutine Architecture

## Component Map

```
binapi2::fapi::client                     Pure container — no threads, no io_context
|
|-- config                                Connection parameters, API keys
|
|-- http_client                           config-only constructor, async_request only
|
|-- rest::pipeline                        signing + encoding + http_client + decode
|
|-- rest::account_service    --|
|-- rest::convert_service    --|
|-- rest::market_data_service--|          REST services hold pipeline& (non-owning)
|-- rest::trade_service      --|          constrained async_execute per service
|-- rest::user_data_stream   --|
|
|-- websocket_api::client                 lazy-init, config-only constructor
|     `-- websocket_client (impl)         persistent: resolver, ssl_ctx, ws_stream, flat_buffer
|
|-- streams::market_streams               lazy-init, config-only constructor
|     `-- websocket_client (impl)         persistent: resolver, ssl_ctx, ws_stream, flat_buffer
|
`-- streams::user_streams                 lazy-init, config-only constructor
      `-- websocket_client (impl)         persistent: resolver, ssl_ctx, ws_stream, flat_buffer
```

**Key difference from earlier designs:** The client does not own an `io_thread`, does not
own an `io_context`, and has no `run_sync()` method. It is a pure container of config,
transport, pipeline, services, and lazy WebSocket components. All methods return
`cobalt::task` — the user provides the executor.

---

## Executor Ownership: User's Choice

The library provides async coroutines (`cobalt::task`). The user provides the execution
context. Three execution environments are supported:

### Environment 1: `cobalt::main` (recommended for async applications)

```cpp
#include <boost/cobalt/main.hpp>

boost::cobalt::main co_main(int argc, char* argv[]) {
    fapi::client c(cfg);

    // REST — direct co_await
    auto info = co_await c.market_data.async_execute(exchange_info_request{});

    // WS API
    co_await c.ws_api().async_connect();
    co_await c.ws_api().async_session_logon();
    auto order = co_await c.ws_api().async_execute(order_place_request{...});

    // Streams — generator pattern
    auto stream = c.streams().subscribe(book_ticker_subscription{.symbol = "BTCUSDT"});
    while (stream) {
        auto event = co_await stream;
        if (!event) break;
        // process event->best_bid_price ...
    }

    co_return 0;
}
```

`cobalt::main` provides the event loop. Coroutines run on its executor. This is the
natural environment for generators (`cobalt::generator`) used by stream subscriptions.

### Environment 2: `detail::io_thread` (bridging async to sync code)

```cpp
#include <binapi2/fapi/detail/io_thread.hpp>

int main() {
    fapi::detail::io_thread io;
    fapi::client c(cfg);

    // REST — sync bridge via io_thread::run_sync
    auto info = io.run_sync(c.market_data.async_execute(exchange_info_request{}));

    // WS API — sync bridge
    io.run_sync(c.ws_api().async_connect());
    io.run_sync(c.ws_api().async_session_logon());
    auto order = io.run_sync(c.ws_api().async_execute(order_place_request{...}));
}
```

`io_thread` owns an `io_context` and a background thread running `io_context::run()`.
`run_sync(task)` spawns the coroutine on the io_context and blocks the calling thread
via `std::future::get()`. Suitable for scripts, tests, and synchronous code that needs
to call async APIs.

**Limitation:** `cobalt::generator` (used by `subscribe()`) cannot be driven through
`run_sync()` because generators yield multiple values and require an event loop to
drive. Use `cobalt::main` for generator-based stream consumption, or use the low-level
`async_connect` + `async_read_text` pattern through `run_sync()` calls.

### Environment 3: Manual `io_context` / `thread_pool`

```cpp
boost::asio::io_context io;
fapi::client c(cfg);

// Spawn a cobalt task on the io_context
auto future = boost::cobalt::spawn(io, c.market_data.async_execute(exchange_info_request{}),
                                   boost::asio::use_future);

// Run the event loop (on this thread or another)
std::thread runner([&] { io.run(); });
auto info = future.get();  // blocks until result ready

io.stop();
runner.join();
```

Or with `asio::thread_pool`:

```cpp
boost::asio::thread_pool pool{4};
auto future = boost::cobalt::spawn(pool, task, boost::asio::use_future);
auto result = future.get();
pool.join();
```

This gives maximum control over threading, but the user must manage the event loop
lifetime.

---

## How Bridging Works

All library methods return `cobalt::task<result<T>>`. Bridging to different call
models is a one-liner:

```cpp
fapi::client c(cfg);
auto task = c.market_data.async_execute(exchange_info_request{});

// True async — in a cobalt coroutine
auto r = co_await std::move(task);

// Future — from any thread with an executor
auto future = cobalt::spawn(executor, std::move(task), asio::use_future);
auto r = future.get();  // blocks caller

// Callback — fire and forget
cobalt::spawn(executor, std::move(task), [](auto r) { handle(r); });

// io_thread convenience
auto r = io.run_sync(std::move(task));
```

The library provides one method variant (`cobalt::task`). The call model is the
user's choice.

---

## Cobalt Context

**There is no separate "cobalt context."** Boost.Cobalt uses `asio::io_context` directly
as its execution facility. The threading contract is:

> No more than one kernel thread may execute within a given io_context at a time.

Cobalt coroutines are single-threaded by design within their io_context.

### How coroutines get their executor

Every async method acquires the executor from within the coroutine:

```cpp
auto executor = co_await boost::cobalt::this_coro::executor;
```

This returns the executor of the io_context that the coroutine was spawned on (via
`cobalt::spawn`) or the implicit executor from `cobalt::main`. All I/O objects
constructed with this executor dispatch completions back to the same io_context.

### `cobalt::use_op` -- the completion token

`cobalt::use_op` bridges ASIO async operations into cobalt coroutines:

```cpp
co_await resolver.async_resolve(host, port, boost::cobalt::use_op);
co_await asio::async_connect(socket, endpoints, boost::cobalt::use_op);
co_await stream.async_handshake(ssl::stream_base::client, boost::cobalt::use_op);
```

It creates an awaitable that wraps the ASIO operation directly (no extra coroutine
frame allocation, unlike `use_task`). When the operation completes, the coroutine
resumes on the same executor.

---

## Connection Lifecycle

### HTTP (REST) -- per-request, no reuse

```
async_request() coroutine:
    1. make_ssl_context()           <- new SSL context
    2. tcp::resolver{ executor }    <- new resolver
    3. ssl::stream<tcp::socket>{ executor, ssl_ctx }  <- new socket
    4. co_await async_resolve()
    5. co_await async_connect()
    6. co_await async_handshake()   <- TLS
    7. co_await async_write()       <- HTTP request
    8. co_await async_read()        <- HTTP response
    9. stream.shutdown()            <- close (errors ignored)
    All objects destroyed at coroutine exit.
```

Source: `src/binapi2/fapi/transport/http_client.cpp`

### WebSocket -- persistent, long-lived

```
async_connect() coroutine:
    1. resolver.emplace(executor)          <- lazy, stored in impl
    2. stream.emplace(executor, ssl_ctx)   <- lazy, stored in impl
    3. co_await async_resolve()
    4. co_await async_connect()
    5. install ping/pong handler
    6. co_await ssl handshake
    7. co_await ws handshake
    Objects survive in impl_ across calls.

async_write_text() / async_read_text() -- reuse same stream
async_close() -- WS close frame, sets connected=false
```

Source: `src/binapi2/fapi/transport/websocket_client.cpp`

---

## Can we have multiple threads on one io_context?

**Not with Cobalt.** Cobalt's design assumes single-threaded execution per io_context.
Its synchronization primitives (channels, select, etc.) avoid locking specifically
because of this guarantee.

Calling `io_context::run()` from multiple threads would violate this invariant and
cause undefined behavior in cobalt coroutines.

**What about strands?** Strands are compatible with cobalt when used with explicit
executor passing via `asio::executor_arg_t`:

```cpp
cobalt::promise<void> work(int arg,
                           asio::executor_arg_t,
                           cobalt::executor exec);
```

Without this, cobalt uses the thread-local default executor, which breaks when a
strand switches threads.

## Can we have multiple io_contexts in one thread?

**Technically yes** (alternate polling), but it defeats the purpose. If you need
multiple event loops, use multiple io_threads or separate `cobalt::spawn` calls
on different executors.

---

## Connection Pooling

### Current: per-request connections

- **HTTP (REST):** Every `async_request()` call performs the full cycle: DNS resolve,
  TCP connect, TLS handshake, HTTP write, HTTP read, shutdown. No connection reuse.
- **WebSocket:** Inherently persistent -- one connect, many reads/writes. Pooling is
  not applicable.

### Current cost per REST request

```
resolve     ~1-5ms   (DNS lookup, typically cached by OS)
tcp_connect ~10-50ms (depends on network latency to Binance)
tls_handshake ~20-80ms (TLS 1.3 full handshake)
http_write  ~1ms
http_read   ~10-100ms (server processing + response transfer)
shutdown    ~1ms
```

The TLS handshake dominates. With connection reuse (HTTP keep-alive), subsequent
requests on the same connection skip resolve + connect + TLS, saving 30-130ms per
request.

### When would we need it?

| Scenario | Pooling needed? |
|---|---|
| Occasional market data queries | No -- latency overhead acceptable |
| Burst of REST requests (e.g., batch account queries) | Beneficial -- saves repeated TLS handshakes |
| High-frequency order placement via REST | Yes -- or better, use WebSocket API instead |
| WebSocket API (already persistent) | N/A -- single connection, already optimal |
| Market streams (already persistent) | N/A |

### Recommendation

Not needed now. The WebSocket API (`ws_api()`) already provides persistent low-latency
access for trading. For REST-heavy workloads, connection pooling would be a targeted
optimization when profiling shows TLS handshake overhead is significant.

---

## Thread Pool

### Can Cobalt use a thread pool?

**Yes.** Cobalt supports `asio::thread_pool` via explicit executor passing:

```cpp
boost::asio::thread_pool pool{4};
auto strand = boost::asio::make_strand(pool.get_executor());
cobalt::spawn(strand, cpu_heavy_task(), detached);
```

The key requirement: each cobalt coroutine must run on at most one thread at a time
(the strand guarantees this).

### Benefits

| Benefit | When it matters |
|---|---|
| Concurrent REST requests | Multiple independent API calls in parallel |
| CPU-bound JSON parsing off I/O | Large responses (exchange_info: ~100KB) |
| Non-blocking stream reads while REST is in flight | Prevents market data stalls during REST bursts |

### Example: concurrent REST queries

```cpp
// With cobalt::main + cobalt::join:
auto [positions, balances, orders] = co_await cobalt::join(
    c.trade.async_execute(position_info_v3_request{}),
    c.account.async_execute(balances_request{}),
    c.trade.async_execute(all_open_orders_request{.symbol = "BTCUSDT"})
);
```

### Recommendation

A single executor (via `cobalt::main` or `io_thread`) is correct for most use cases.
Thread pool becomes relevant when:

1. You need concurrent REST requests (parallel queries at startup, bulk operations)
2. Stream reads must not block on REST calls (latency-sensitive trading)
3. Profiling shows the single event loop is a bottleneck

---

## Summary Table

| Question | Answer |
|---|---|
| Threads per client | 0 -- client owns no threads |
| io_contexts per client | 0 -- user provides executor |
| Cobalt context? | No -- cobalt uses io_context directly |
| Sockets per REST request | 1 (created and destroyed per request) |
| Sockets per WS component | 1 persistent (ws_api, market_streams, user_streams each own one) |
| Max concurrent sockets | N REST (limited by executor) + 3 WS = N+3 |
| Connection pooling | Not implemented; not needed (WS API covers low-latency) |
| Thread pool | Supported by cobalt; user's choice when concurrency needed |
| Generator pattern | Requires `cobalt::main` or equivalent event loop |
| io_thread bridging | Works for single-shot tasks; not for generators |

---

## Source References

| File | Role |
|---|---|
| `include/binapi2/fapi/detail/io_thread.hpp` | User-provided background thread + `run_sync()` |
| `include/binapi2/fapi/client.hpp` | Pure container facade -- no executor ownership |
| `src/binapi2/fapi/client.cpp` | Client construction, lazy WS init |
| `src/binapi2/fapi/transport/http_client.cpp` | Per-request connection lifecycle |
| `src/binapi2/fapi/transport/websocket_client.cpp` | Persistent WS connection, pimpl |
| `include/binapi2/fapi/rest/pipeline.hpp` | REST execution pipeline: sign, encode, send, decode |
| `include/binapi2/fapi/rest/service.hpp` | Service base, holds pipeline& |
| `src/binapi2/fapi/websocket_api/client.cpp` | WS API RPC dispatch, auth injection |
| `src/binapi2/fapi/streams/market_streams.cpp` | Stream connect + typed generators |
| `src/binapi2/fapi/streams/user_streams.cpp` | Variant event dispatch generator |
