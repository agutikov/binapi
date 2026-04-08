# Refactoring Plan: Async-First Architecture

Transition from current state to the layered async-first architecture described in `docs/binapi2/async_architecture.md`.

## Current State Summary

```
client (god object)
├── io_thread_ (optional, for sync bridging)
├── cfg_
├── http_client_ (transport)
├── 5 REST services (back-ref to client)
├── ws_api_ (lazy, owns websocket_client)
├── streams_ (lazy, owns websocket_client)
└── user_streams_ (lazy, owns websocket_client)
```

Problems:
- `client` owns execution (io_thread), transport (http_client), protocol (prepare_and_send), and serves as service container
- Every REST service holds `client&` back-reference
- Transport constructors require `io_thread*` even though async methods don't use it
- `client::run_sync` duplicates `io_thread::run_sync`
- Sync wrappers exist at every layer (transport, ws_api, streams)
- 46 sync/callback method pairs in market_streams alone

## Phase 0: Rewrite tests to async

**Goal:** Convert all tests that use sync client API to async *before* starting the refactoring. This ensures tests stay green throughout all subsequent phases as we remove sync API.

### Analysis

| Test | Uses client/sync API? | Action |
|---|---|---|
| `integration/postman_mock/main.cpp` | Yes — `client(cfg)`, `c.market_data.execute()`, etc. | Rewrite with `cobalt::main`, `co_await async_execute()` |
| `response_parse_test.cpp` | No — uses `detail::decode_response()` directly | No change |
| `json_query_test.cpp` | No — pure JSON/query tests | No change |
| `signing_test.cpp` | No — pure HMAC/signing tests | No change |
| `enums_test.cpp` | No — pure enum tests | No change |
| `enum_set_test.cpp` | No — pure enum_set tests | No change |
| `decimal_test.cpp` | No — pure decimal tests | No change |
| `result_test.cpp` | No — pure result type tests | No change |
| `postman_validation_test.cpp` | No — validates endpoint metadata | No change |

Only `postman_mock/main.cpp` needs rewriting.

### Changes

Replace `main()` with `cobalt::main co_main()`. Replace all `c.market_data.execute(req)` with `co_await c.market_data.async_execute(req)`. Same for account, trade, user_data_streams. Client constructed with `async_mode` (no io_thread needed — cobalt provides executor).

### Verification

Build and run: all 308 unit tests + 18 postman mock integration tests must pass.

**Status: DONE**

Additionally fixed 38 fake async methods in REST services (account: 13, market_data: 14, trade: 7, user_data_streams: 3) — they were wrapping sync calls (`co_return sync_method()`) instead of using `co_await owner_.async_execute()`.

---

## Target State

```
Layer 1: Pure functions (signing, query, decode)
Layer 2: Async I/O (http_client, websocket_client — no io_thread dep)
Layer 3: Protocol (rest::pipeline, ws_api::session, stream::connection)
Layer 4: Execution (io_thread — user-provided, not library-owned)
Layer 5: Facade (client — container only, no execution)
```

---

## Phase 1: Extract REST pipeline from client

**Goal:** `prepare_and_send` / `async_prepare_and_send` move out of `client` into a standalone `rest::pipeline` that holds only `config&` and `http_client&`.

### Changes

**New file: `include/binapi2/fapi/rest/pipeline.hpp`**

```cpp
namespace binapi2::fapi::rest {

class pipeline {
public:
    pipeline(const config& cfg, transport::http_client& http);

    template<typename Response>
    boost::cobalt::task<result<Response>>
    async_execute(boost::beast::http::verb method, std::string path,
                  query_map query, bool signed_request);

    // Generic: request struct → endpoint_traits → async_execute
    template<class Request>
        requires has_endpoint_traits<Request>
    auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>;

private:
    const config& cfg_;
    transport::http_client& http_;
};

} // namespace binapi2::fapi::rest
```

**New file: `src/binapi2/fapi/rest/pipeline.cpp`**

Move `prepare_and_send` logic (signing, query building, routing GET/POST) here as `async_execute`. This is the only implementation — no sync variant.

**Modify: `include/binapi2/fapi/rest/service.hpp`**

Change `client& owner_` → `pipeline& pipeline_`.

```cpp
class service {
public:
    explicit service(pipeline& p) noexcept;

    template<class Request>
        requires has_endpoint_traits<Request>
    auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename endpoint_traits<Request>::response_type_t>>;

protected:
    pipeline& pipeline_;
};
```

Remove `execute()` (sync) from service. Only `async_execute()` remains.

**Modify: REST service headers (market_data, account, trade, convert, user_data_streams)**

- All named methods become async-only: `async_klines()`, `async_balances()`, etc.
- Remove sync variants (`klines()`, `balances()`, etc.)
- Constructor takes `pipeline&` instead of `client&`

**Modify: `include/binapi2/fapi/client.hpp`**

- Remove `execute()`, `async_execute()`, `prepare_and_send()`, `async_prepare_and_send()`
- Remove `run_sync()`
- Add `rest::pipeline pipeline_` member
- Services constructed with `pipeline_` instead of `*this`

### Migration

| Before | After |
|---|---|
| `c.market_data.execute(req)` | `co_await c.market_data.async_execute(req)` |
| `c.market_data.async_execute(req)` | `co_await c.market_data.async_execute(req)` (unchanged) |
| `c.execute<T>(verb, path, query, signed)` | `co_await c.pipeline.async_execute<T>(verb, path, query, signed)` |

### Files touched
- New: `rest/pipeline.hpp`, `rest/pipeline.cpp`
- Modify: `rest/service.hpp`, `rest/market_data.hpp`, `rest/account.hpp`, `rest/trade.hpp`, `rest/convert.hpp`, `rest/user_data_streams.hpp`
- Modify: `client.hpp`, `client.cpp`
- Modify: all REST service `.cpp` files (named methods drop sync variants)
- Modify: `demo-cli/cmd_*.cpp` (sync callers → need io_thread bridging)
- Modify: `demo/commands.cpp` (already async — minimal changes)

---

## Phase 2: Remove io_thread from transport constructors

**Goal:** Transport objects don't know about io_thread. Sync wrappers move out entirely.

### Changes

**Modify: `transport/http_client.hpp`**

```cpp
class http_client {
public:
    explicit http_client(config cfg);  // only constructor

    boost::cobalt::task<result<http_response>>
    async_request(verb method, std::string target, std::string body,
                  std::string content_type, std::string api_key);
    // No sync request() method
};
```

Remove: `http_client(io_thread&, config)`, `request()` sync wrapper, `io_thread*` member.

**Modify: `transport/websocket_client.hpp`**

```cpp
class websocket_client {
public:
    explicit websocket_client(config cfg);  // only constructor

    boost::cobalt::task<result<void>> async_connect(std::string host, std::string port, std::string target);
    boost::cobalt::task<result<void>> async_write_text(std::string message);
    boost::cobalt::task<result<std::string>> async_read_text();
    boost::cobalt::task<result<void>> async_close();
    // No sync wrappers, no run_read_loop
};
```

Remove: `websocket_client(io_thread&, config)`, all sync methods, `io_thread*` from impl.

**Impact:** Everything that currently calls sync transport methods must switch to async.

### Files touched
- Modify: `transport/http_client.hpp`, `transport/http_client.cpp`
- Modify: `transport/websocket_client.hpp`, `transport/websocket_client.cpp`
- Modify: `websocket_api/client.cpp` (sync `send_rpc` removed, only `async_send_rpc`)
- Modify: `streams/market_streams.cpp` (sync connect/read removed)
- Modify: `streams/user_streams.cpp` (sync connect/read removed)
- Modify: `client.cpp` (http_client constructed without io_thread)

---

## Phase 3: Remove sync API from websocket_api::client

**Goal:** WS API client is async-only.

### Changes

**Modify: `websocket_api/client.hpp`**

Remove:
- `client(io_thread&, config)` constructor
- `connect()`, `close()` sync methods
- `execute()` sync generic
- `session_logon()` sync
- `account_status()`, `account_balance()`, etc. sync methods
- `send_rpc()` sync method

Keep:
- `explicit client(config)` constructor
- All `async_*` methods (already truly async after Phase 2 of modular composition)
- `async_send_rpc` private method

Remove `io_context_` member (already unused).

### Files touched
- Modify: `websocket_api/client.hpp`, `websocket_api/client.cpp`
- Modify: `demo-cli/cmd_ws_api.cpp` (switch to async)

---

## Phase 4: Remove sync API from streams

**Goal:** market_streams and user_streams are async-only. Remove all 46 sync/callback method pairs from market_streams and all sync methods from user_streams.

### Changes

**Modify: `streams/market_streams.hpp`**

Remove all sync `connect_*()` methods (21 methods).
Remove all `void_callback` overloads (21 methods).
Remove all sync `read_*_loop()` methods (21 methods).
Remove all `void_callback` read loop overloads (21 methods).
Remove sync `connect_combined()`, `close()` + callback overloads.
Remove `subscribe()`, `unsubscribe()`, `list_subscriptions()` sync methods.
Remove `io_context_*` member.
Remove `void_callback` typedef.

Keep:
- `explicit market_streams(config)`
- `async_connect(target)`, `async_read_text()`, `async_close()`
- `configuration()` accessor

Add:
- `async_subscribe(streams)`, `async_unsubscribe(streams)`, `async_list_subscriptions()`

The 21 typed connect methods are replaced by a single `async_connect(target)` that takes the full target path. Target path construction moves to either:
- Free functions: `std::string book_ticker_target(const config&, symbol_t)` 
- Or the caller builds it (demo already does this)

**Modify: `streams/user_streams.hpp`**

Same pattern: remove sync methods, keep async.
Remove `io_context_*` member, `void_callback` typedef.
Remove sync `connect()`, `read_loop()`, `close()`.
Keep `async_connect()`, `async_read_text()`, `async_close()`.

**Modify: `streams/market_streams.cpp`**

Delete ~700 lines of sync connect/read/callback implementations.
Keep async methods.
Move `subscribe`/`unsubscribe`/`list_subscriptions` to async (use `co_await transport_.async_write_text/async_read_text`).

**Modify: `streams/user_streams.cpp`**

Delete sync connect/read/callback implementations.
Keep async methods.

### Files touched
- Modify: `streams/market_streams.hpp`, `streams/market_streams.cpp`
- Modify: `streams/user_streams.hpp`, `streams/user_streams.cpp`
- Modify: `demo-cli/cmd_stream.cpp` (rewrite to async)
- Modify: `demo-cli/cmd_user_stream.cpp` (rewrite to async)

---

## Phase 5: Remove io_thread from client

**Goal:** `client` is a pure container. No execution policy.

### Changes

**Modify: `client.hpp`**

```cpp
class client {
public:
    explicit client(config cfg);

    config& configuration() noexcept;
    const config& configuration() const noexcept;

    rest::pipeline& rest() noexcept;
    websocket_api::client& ws_api();
    streams::market_streams& streams();
    streams::user_streams& user_streams();

    // Service accessors (convenience, delegate to pipeline)
    rest::account_service account;
    rest::convert_service convert;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

private:
    config cfg_;
    transport::http_client http_;
    rest::pipeline pipeline_;
    std::unique_ptr<websocket_api::client> ws_api_;
    std::unique_ptr<streams::market_streams> streams_;
    std::unique_ptr<streams::user_streams> user_streams_;
};
```

Remove: `io_thread_`, `run_sync()`, `has_io_thread()`, `async_mode_t`, `transport()` accessor.
Remove: `execute()`, `async_execute()` (moved to pipeline).

**Modify: `client.cpp`**

Single constructor. No io_thread. Lazy init creates objects with `config` only.

### Files touched
- Modify: `client.hpp`, `client.cpp`
- Delete: `async_mode_t` (no longer needed — client is always async)

---

## Phase 6: Rewrite local_order_book as coroutine

**Goal:** `local_order_book::start()` becomes a coroutine instead of a blocking method with internal mutex.

### Changes

**Modify: `streams/local_order_book.hpp`**

```cpp
class local_order_book {
public:
    using snapshot_callback = std::function<void(const order_book_snapshot&)>;

    local_order_book(streams::market_streams& streams, rest::pipeline& rest, config cfg);

    // Async: runs until error or stop()
    boost::cobalt::task<result<void>> async_run(symbol_t symbol, int depth_limit);
    void stop();

    // Thread-safe snapshot read
    order_book_snapshot snapshot() const;
    void set_snapshot_callback(snapshot_callback cb);

private:
    // ...
};
```

Change `client&` dependency to `rest::pipeline&` (only needs REST for snapshot fetch).

The `async_run` coroutine replaces the blocking `start()`:
1. `co_await streams.async_connect(target)`
2. Buffer events in a local vector (no mutex — single coroutine)
3. `co_await rest.async_execute(order_book_request)` for snapshot
4. Apply buffered events
5. Loop: `co_await streams.async_read_text()`, parse, apply, callback

Mutex only needed for `snapshot()` (cross-thread read) and `stop()` flag.

### Files touched
- Modify: `streams/local_order_book.hpp`, `streams/local_order_book.cpp`
- Modify: `demo-cli/cmd_order_book.cpp`

---

## Phase 7: Clean up transport constructors

**Goal:** Remove dual-constructor pattern now that io_thread is fully decoupled.

After Phases 1-6, the `(io_thread&, config)` constructors are unused everywhere. Remove them.

### Changes

- `transport/http_client.hpp` — only `http_client(config)` remains
- `transport/websocket_client.hpp` — only `websocket_client(config)` remains
- `websocket_api/client.hpp` — only `client(config)` remains
- `streams/market_streams.hpp` — only `market_streams(config)` remains
- `streams/user_streams.hpp` — only `user_streams(config)` remains
- Remove `#include <binapi2/fapi/detail/io_thread.hpp>` from all transport/stream headers

---

## Phase 7: Unified traits and typed async_execute across all APIs

**Goal:** Every API component (REST services, WS API, streams) follows the same pattern:
1. Request types inherit a service tag
2. Traits map request → endpoint metadata + response type
3. A single constrained `async_execute` template handles all dispatch
4. No named methods except for truly custom serialization

### Design: Service tag inheritance

Each request type inherits an empty tag that identifies which service it belongs to:

```cpp
// Tag base types (in types/request_tags.hpp)
struct rest_market_data_tag {};
struct rest_account_tag {};
struct rest_trade_tag {};
struct rest_convert_tag {};
struct rest_user_data_stream_tag {};
struct ws_api_tag {};
struct stream_tag {};
```

Request types inherit the appropriate tag:

```cpp
struct ping_request_t : rest_market_data_tag {};
struct exchange_info_request_t : rest_market_data_tag { ... };
struct new_order_request_t : rest_trade_tag { ... };
struct convert_quote_request_t : rest_convert_tag { ... };

// WS API
struct websocket_api_book_ticker_request_t : ws_api_tag { ... };

// Streams
struct book_ticker_subscription : stream_tag { symbol_t symbol; };
```

### Design: Concepts per service

```cpp
template<class T>
concept is_market_data_request = std::derived_from<T, rest_market_data_tag>
                              && rest::has_endpoint_traits<T>;

template<class T>
concept is_account_request = std::derived_from<T, rest_account_tag>
                          && rest::has_endpoint_traits<T>;

// etc.
```

### Design: Constrained service async_execute

```cpp
class market_data_service : public service {
public:
    using service::service;

    template<class Request>
        requires is_market_data_request<Request>
    [[nodiscard]] auto async_execute(const Request& request)
        -> boost::cobalt::task<result<typename rest::endpoint_traits<Request>::response_type_t>>
    {
        co_return co_await pipeline_.async_execute(request);
    }

    // Type aliases for discoverability
    using ping_request = types::ping_request_t;
    using server_time_request = types::server_time_request_t;
    using exchange_info_request = types::exchange_info_request_t;
    // ... all request types belonging to this service
};
```

### Design: Eliminate named REST methods

**Parameterless endpoints** get their own request types:

```cpp
// Before: named method because no request type existed
async_balances() → pipeline_.async_execute<vector<balance_t>>(endpoint...)

// After: request type with traits
struct balances_request_t : rest_account_tag {};

template<> struct endpoint_traits<balances_request_t> {
    using response_type_t = std::vector<futures_account_balance_t>;
    static constexpr auto& endpoint = account_balances_endpoint;
};

// Usage: c.account.async_execute(balances_request_t{})
```

**Shared request types** (same struct used by multiple endpoints) get distinct wrapper types:

```cpp
// Before: kline_request_t shared by klines, mark_price_klines, premium_index_klines
// Three named methods with different endpoints

// After: distinct request types wrapping the same fields
struct klines_request_t : rest_market_data_tag {
    symbol_t symbol; kline_interval_t interval; ...
};
struct mark_price_klines_request_t : rest_market_data_tag {
    symbol_t symbol; kline_interval_t interval; ...
};
struct premium_index_klines_request_t : rest_market_data_tag {
    symbol_t symbol; kline_interval_t interval; ...
};

// Each gets its own endpoint_traits → generic execute handles all three
```

**Custom serialization** (batch_orders, cancel_batch_orders, session_logon) — these remain as named methods on the service because the query construction is non-standard.

### Design: Stream traits

```cpp
// Stream traits map subscription → target path + event type
template<class Subscription>
struct stream_traits;

template<>
struct stream_traits<types::book_ticker_subscription> {
    using event_type = types::book_ticker_stream_event_t;
    static std::string target(const config& cfg, const types::book_ticker_subscription& sub) {
        std::string s = sub.symbol.str();
        std::ranges::transform(s, s.begin(), ::tolower);
        return cfg.stream_base_target + "/" + s + "@bookTicker";
    }
};

template<class T>
concept has_stream_traits = requires {
    typename stream_traits<T>::event_type;
};
```

Typed stream methods on `market_streams`:

```cpp
class market_streams {
public:
    // Generic typed connect
    template<class Subscription>
        requires has_stream_traits<Subscription>
    [[nodiscard]] boost::cobalt::task<result<void>>
    async_connect(const Subscription& sub)
    {
        return transport_.async_connect(cfg_.stream_host, cfg_.stream_port,
                                       stream_traits<Subscription>::target(cfg_, sub));
    }

    // Generic typed read
    template<class Event>
    [[nodiscard]] boost::cobalt::task<result<Event>>
    async_read_event()
    {
        auto msg = co_await transport_.async_read_text();
        if (!msg) co_return result<Event>::failure(msg.err);
        Event event{};
        glz::context ctx{};
        if (auto ec = glz::read<detail::json_read_opts>(event, *msg, ctx))
            co_return result<Event>::failure({error_code::json, 0, 0, glz::format_error(ec, *msg), *msg});
        co_return result<Event>::success(std::move(event));
    }

    // Low-level access remains
    boost::cobalt::task<result<void>> async_connect(std::string target);
    boost::cobalt::task<result<std::string>> async_read_text();
    boost::cobalt::task<result<void>> async_close();
};
```

Usage:

```cpp
// Typed — trait-driven
co_await streams.async_connect(types::book_ticker_subscription{.symbol = "BTCUSDT"});
auto event = co_await streams.async_read_event<types::book_ticker_stream_event_t>();

// Low-level — raw text
co_await streams.async_connect("/ws/btcusdt@bookTicker");
auto msg = co_await streams.async_read_text();
```

### Design: WS API traits

Same pattern. Named methods like `async_account_status` become request types:

```cpp
struct ws_account_status_request_t : ws_api_tag {};

template<> struct endpoint_traits<ws_account_status_request_t> {
    using response_type_t = account_information_t;
    static constexpr auto& method = account_status_method;
};

// session_logon stays as named method (custom auth flow)
```

### Changes summary

| Component | Current named methods | After | Remaining named |
|---|---|---|---|
| REST market_data | 14 | 0 | 0 (all via traits) |
| REST account | 13 | 0 | 0 |
| REST trade | 7 | 3 | batch_orders, modify_batch_orders, cancel_batch_orders |
| REST convert | 0 | 0 | 0 |
| REST user_data_streams | 3 | 0 | 0 |
| WS API | 8 named | 1 | session_logon (custom auth) |
| Streams | 0 typed | 0 named | typed via stream_traits |

### New files
- `types/request_tags.hpp` — tag base types
- `streams/stream_traits.hpp` — stream subscription → target + event mapping

### Modified files
- All request type headers — add tag inheritance
- `rest/endpoint_traits.hpp` — add traits for new parameterless request types
- `websocket_api/endpoint_traits.hpp` — add traits for new parameterless ws request types
- All REST service headers — replace named methods with constrained `async_execute` + using declarations
- `websocket_api/client.hpp` — replace named methods with constrained `async_execute`
- `streams/market_streams.hpp` — add typed `async_connect(sub)` and `async_read_event<Event>()`
- All REST service `.cpp` files — delete named method implementations (most become empty)
- `websocket_api/client.cpp` — delete named method implementations

### Files touched
- New: `types/request_tags.hpp`, `streams/stream_traits.hpp`
- Modify: `types/common.hpp`, `types/market_data.hpp`, `types/trade.hpp`, `types/account.hpp`, `types/websocket_api.hpp`, `types/subscriptions.hpp`
- Modify: `rest/endpoint_traits.hpp`, `websocket_api/endpoint_traits.hpp`
- Modify: `rest/market_data.hpp`, `rest/account.hpp`, `rest/trade.hpp`, `rest/convert.hpp`, `rest/user_data_streams.hpp`
- Modify: `rest/service.hpp`
- Modify: `websocket_api/client.hpp`, `websocket_api/client.cpp`
- Modify: `streams/market_streams.hpp`, `streams/market_streams.cpp`
- Modify: `rest/account.cpp`, `rest/market_data.cpp`, `rest/trade.cpp`, `rest/user_data_streams.cpp` (most shrink to near-empty)

---

## Phase 8: Add sync bridging tests

**Goal:** Add tests that verify all sync bridging variants work correctly: `io_thread::run_sync`, future via `cobalt::spawn`, callback via `cobalt::spawn`.

### Changes

**New file: `tests/binapi2/fapi/sync_bridging_test.cpp`**

Tests:
- `io_thread::run_sync(async_execute(...))` — blocks, returns result
- `cobalt::spawn(io.context(), task, use_future)` — returns future, `.get()` blocks
- `cobalt::spawn(io.context(), task, callback)` — callback receives result
- All three against the postman mock server (REST only — sufficient to verify bridging)

These tests exercise `detail::io_thread` and the bridging patterns independently of the library's internal sync wrappers (which are being removed).

### Files touched
- New: `tests/binapi2/fapi/sync_bridging_test.cpp`
- Modify: `tests/binapi2/fapi/CMakeLists.txt`

---

## Phase 9: Rename and restructure examples

**Goal:** Two example apps with clear roles:
- `async-demo-cli` — the full-featured CLI, uses `cobalt::main`, pure async
- `sync-demo` — minimal examples of every non-async call model and execution environment

### Changes

**Rename: `examples/binapi2/fapi/demo-cli/` → `examples/binapi2/fapi/async-demo-cli/`**

- CMake target: `binapi2-fapi-demo-cli` → `binapi2-fapi-async-demo-cli`
- Entry point: replace `main()` with `cobalt::main co_main()`
- Remove `io_thread` creation from main — `co_main` provides the executor
- All commands become coroutines (same pattern as `demo/commands.cpp`)
- All command functions: `cobalt::task<int> cmd_X(client&, const args_t&)`
- Client constructed without io_thread (just `client c(cfg)`)

```cpp
// async-demo-cli/main.cpp
boost::cobalt::main co_main(int argc, char* argv[])
{
    // ... parse args, init logging ...
    client c(make_config());
    co_return co_await cmd.fn(c, sub_args);
}
```

**Rename: `examples/binapi2/fapi/demo/` → `examples/binapi2/fapi/sync-demo/`**

- CMake targets: `demo-sync`, `demo-async` → just `binapi2-fapi-sync-demo`
- Delete `async_main.cpp` (async is now the main demo-cli)
- Delete `sync_main.cpp` (replaced with bridging examples)
- Replace `commands.cpp` with focused bridging examples

**New: `examples/binapi2/fapi/sync-demo/main.cpp`**

Demonstrates every non-async bridging variant:

```cpp
#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/detail/io_thread.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/asio/use_future.hpp>
#include <future>
#include <iostream>

using namespace binapi2::fapi;

// The core async operation — same for all call models
boost::cobalt::task<result<types::server_time_response_t>>
get_server_time(client& c)
{
    co_return co_await c.market_data.async_execute(types::server_time_request_t{});
}

// ---------------------------------------------------------------------------
// Execution environments
// ---------------------------------------------------------------------------

// 1. io_thread — library-provided background thread
void example_io_thread(client& c)
{
    detail::io_thread io;

    // --- sync: block on result ---
    auto r = io.run_sync(get_server_time(c));
    std::cout << "sync: " << r->serverTime.value << "\n";

    // --- future: get std::future, block when ready ---
    auto future = boost::cobalt::spawn(
        io.context(), get_server_time(c), boost::asio::use_future);
    auto r2 = future.get();
    std::cout << "future: " << r2->serverTime.value << "\n";

    // --- callback: fire and forget, result delivered to lambda ---
    std::promise<void> done;
    boost::cobalt::spawn(io.context(), get_server_time(c),
        [&](std::exception_ptr, result<types::server_time_response_t> r3) {
            std::cout << "callback: " << r3->serverTime.value << "\n";
            done.set_value();
        });
    done.get_future().wait();
}

// 2. Manual io_context — user runs the event loop
void example_manual_io_context(client& c)
{
    boost::asio::io_context io;

    std::future<result<types::server_time_response_t>> future;
    future = boost::cobalt::spawn(io, get_server_time(c), boost::asio::use_future);

    io.run();  // drive the event loop on this thread

    auto r = future.get();
    std::cout << "manual io_context: " << r->serverTime.value << "\n";
}

// 3. std::async — run io_context in a detached thread
void example_std_async(client& c)
{
    boost::asio::io_context io;
    auto guard = boost::asio::make_work_guard(io);

    auto io_future = std::async(std::launch::async, [&] { io.run(); });

    auto future = boost::cobalt::spawn(io, get_server_time(c), boost::asio::use_future);
    auto r = future.get();
    std::cout << "std::async: " << r->serverTime.value << "\n";

    guard.reset();
    io_future.get();
}

// ---------------------------------------------------------------------------
// Stream bridging example
// ---------------------------------------------------------------------------

void example_stream_sync(client& c)
{
    detail::io_thread io;

    // Wrap async stream reading into a sync blocking loop
    auto loop_task = [](client& c) -> boost::cobalt::task<void> {
        auto& streams = c.streams();
        auto cfg = c.configuration();
        co_await streams.async_connect(cfg.stream_base_target + "/btcusdt@bookTicker");

        for (int i = 0; i < 5; ++i) {
            auto msg = co_await streams.async_read_text();
            if (!msg) break;
            std::cout << "stream frame: " << msg->substr(0, 80) << "...\n";
        }
        co_await streams.async_close();
    };

    io.run_sync(loop_task(c));
}

// ---------------------------------------------------------------------------

int main()
{
    client c(config::testnet_config());

    std::cout << "=== io_thread ===\n";
    example_io_thread(c);

    std::cout << "\n=== manual io_context ===\n";
    example_manual_io_context(c);

    std::cout << "\n=== std::async ===\n";
    example_std_async(c);

    std::cout << "\n=== stream (sync bridge) ===\n";
    example_stream_sync(c);
}
```

**Delete: `examples/binapi2/fapi/async-demo/`** — prototype, superseded by async-demo-cli

**Modify: `examples/binapi2/fapi/CMakeLists.txt`**

```cmake
add_subdirectory(async-demo-cli)
add_subdirectory(sync-demo)
```

### Summary of example apps after this phase

| App | Entry point | Executor | Purpose |
|---|---|---|---|
| `binapi2-fapi-async-demo-cli` | `cobalt::main co_main()` | cobalt runtime | Full CLI, production-style, pure async |
| `binapi2-fapi-sync-demo` | `main()` | varies per example | Demonstrates bridging: io_thread, manual io_context, std::async, callbacks, futures |

### Files touched
- Rename: `demo-cli/` → `async-demo-cli/`, update CMake target name
- Rewrite: `async-demo-cli/main.cpp` (cobalt::main entry), all cmd_*.cpp (coroutines)
- Rename: `demo/` → `sync-demo/`
- Rewrite: `sync-demo/main.cpp` (bridging examples), `sync-demo/CMakeLists.txt`
- Delete: `async-demo/` directory
- Delete: `sync-demo/sync_main.cpp`, `sync-demo/async_main.cpp`, `sync-demo/commands.cpp`, `sync-demo/commands.hpp`
- Modify: `examples/binapi2/fapi/CMakeLists.txt`

---

## Dependency Graph: Target State

```
signing.hpp ─────────────────────────────┐
query.hpp ───────────────────────────────┤
json_opts.hpp ───────────────────────────┤
                                         ▼
                                   rest::pipeline ◄── rest::service (base)
                                     │                  ▲   ▲   ▲   ▲   ▲
                                     │                  │   │   │   │   │
                              transport::http_client  mkt acct trade cvt uds
                                     │
                                     ▼
                              (TCP + TLS + HTTP)

                              websocket_api::client
                                     │
                              transport::websocket_client
                                     │
                                     ▼
                              (TCP + TLS + WebSocket)

                              streams::market_streams
                              streams::user_streams
                                     │
                              transport::websocket_client

                              local_order_book
                                     │
                              streams::market_streams + rest::pipeline

                              client (facade)
                                 │
                                 ├── config
                                 ├── http_client
                                 ├── rest::pipeline
                                 ├── 5 REST services
                                 ├── ws_api (lazy)
                                 ├── market_streams (lazy)
                                 └── user_streams (lazy)

                              io_thread (user-owned, not in library graph)
```

---

## Phase Order and Dependencies

```
Phase 0 ─── Rewrite tests to async              ✅ DONE
  │
  Phase 1 ─── Extract REST pipeline              ✅ DONE
    │
    Phase 2+3+4 ─── Remove sync from all layers  ✅ DONE (combined)
      │
      Phase 5 ─── Remove io_thread from client    ✅ DONE
        │
        Phase 6 ─── Rewrite local_order_book      ✅ DONE
          │
          Phase 7 ─── Unified traits across all APIs
          │
          Phase 8 ─── Add sync bridging tests
          │
          Phase 9 ─── Rename and restructure examples
```

Build and run tests after every phase. Examples may be temporarily disabled (comment out `add_subdirectory` in CMake) during Phases 1-7 since they use sync API that's being removed. Tests (converted in Phase 0) stay green throughout.

Phases 3, 4 can run in parallel after Phase 2.
Phase 5 requires 2, 3, 4.
Phases 6, 7 can run in parallel after Phase 5.
Phase 8 adds sync bridging tests after library is fully async.
Phase 9 is the final step — examples restructured last.

---

## Risk Assessment

| Risk | Impact | Mitigation |
|---|---|---|
| Tests break during refactoring | No safety net | Phase 0 converts tests to async first — they stay green |
| Examples break during Phases 1-7 | Build failures | Disable examples in CMake; re-enable in Phase 9 |
| Postman mock tests use sync API | 18 integration tests break | Phase 0 rewrites them to async before anything else |
| local_order_book mutex removal | Race conditions if misused | Keep mutex for snapshot() cross-thread access |
| Named REST methods (35 pairs) all lose sync | Large diff | Mechanical: delete sync, keep async |
| market_streams loses 84 methods | Massive simplification but breaking | Replace with generic async_connect + target helpers |

## Lines of Code Estimate

| Phase | Added | Removed | Net |
|---|---|---|---|
| 0: Tests to async | ~50 | ~50 | 0 |
| 1: REST pipeline | ~150 | ~100 | +50 |
| 2: Transport cleanup | ~10 | ~80 | -70 |
| 3: WS API sync removal | ~0 | ~200 | -200 |
| 4: Streams sync removal | ~30 | ~700 | -670 |
| 5: Client cleanup | ~10 | ~60 | -50 |
| 6: Local order book | ~80 | ~120 | -40 |
| 7: Unified traits | ~300 | ~500 | -200 |
| 8: Sync bridging tests | ~80 | ~0 | +80 |
| 9: Examples restructure | ~300 | ~500 | -200 |
| **Total** | **~960** | **~2310** | **-1350** |
