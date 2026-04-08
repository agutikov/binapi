# Unified Client Refactoring Plan

## Goal

Replace three separate top-level objects with a single `fapi::client` that owns all transports. Fix the `cobalt::run()` crash for WebSocket connections.

```cpp
// Before: three separate objects, three io_context refs, three configs
asio::io_context io;
fapi::client rest{ io, cfg };
streams::market_streams streams{ io, cfg };
websocket_api::client ws_api{ io, cfg };

// After: one client, everything accessible as sub-objects
fapi::client client{ cfg };
client.market_data.execute(price_ticker_request{...});       // REST
client.streams.connect_book_ticker({.symbol = "BTCUSDT"});   // WS stream
client.ws_api.execute(order_place_request{...});              // WS API
```

## Current Architecture

```
fapi::client (REST only, misleading name)
  ├── http_client http_
  ├── io_context& (unused by cobalt::run)
  ├── config cfg_
  └── services: market_data, trade, account, convert, user_data_streams
      └── each holds client& back-reference

websocket_api::client (standalone)
  ├── websocket_client transport_
  ├── io_context& (unused)
  └── config cfg_

streams::market_streams (standalone)
  ├── websocket_client transport_
  ├── io_context& (used for async post)
  └── config cfg_
```

## Critical Bug: `cobalt::run()` per-call

`cobalt::run(task)` creates a throwaway `io_context`, drives the coroutine, then destroys it. For WebSocket:
- `connect()` → cobalt::run creates io_context A, stream binds executor refs to A, A destroyed
- `read_text()` → cobalt::run creates io_context B, stream tries to use A's executor → SIGSEGV

REST is unaffected because each request creates fresh I/O objects inside the coroutine.

## Target Architecture

```
fapi::client
  ├── detail::io_thread (owns io_context + background thread)
  ├── config cfg_
  ├── http_client http_
  ├── unique_ptr<websocket_client> ws_api_transport_  (lazy)
  ├── unique_ptr<websocket_client> stream_transport_  (lazy)
  ├── REST services: market_data, trade, account, convert, user_data_streams
  ├── websocket_api::ws_api_service ws_api
  └── streams::stream_service streams
      └── all services hold client& back-reference
```

## Phases

### Phase 0: Preparatory Bug Fixes (no API changes)

**0.1** Extract `json_read_opts` into `include/binapi2/fapi/detail/json_opts.hpp`
- Currently defined in `client.hpp` detail namespace
- Needed by: `client.hpp`, `market_streams.cpp`, `user_streams.cpp`, `websocket_api/client.cpp`

**0.2** Fix all `glz::read_json` calls to use `json_read_opts`:
- `src/binapi2/fapi/streams/market_streams.cpp` line 80 — `read_stream_loop`
- `src/binapi2/fapi/streams/user_streams.cpp` — all `glz::read_json(event, payload)` calls
- `src/binapi2/fapi/websocket_api/client.cpp` — `decode_rpc_response`
- `src/binapi2/fapi/streams/market_streams.cpp` line 807 — `list_subscriptions`

### Phase 1: io_thread Infrastructure

**1.1** Two construction modes for `client`:

```cpp
// Mode 1: Self-contained — client owns io_context + thread
fapi::client client{ cfg };

// Mode 2: Component — borrows caller's io_context, no internal thread
asio::io_context io;
fapi::client client{ io, cfg };
```

Implementation:
```cpp
class client {
    // Owned mode only (null in component mode)
    std::unique_ptr<asio::io_context> owned_io_;
    std::unique_ptr<std::jthread> thread_;
    std::unique_ptr<asio::executor_work_guard<...>> work_;

    // Always valid: points to owned_io_ or caller's reference
    asio::io_context& io_;
    // ...
};
```

Constructor 1 creates `owned_io_`, `work_`, starts `thread_` running `owned_io_->run()`.
Constructor 2 sets `io_` to caller's reference, leaves unique_ptrs null.

**1.2** `run_sync` — only available in owned mode:

```cpp
template<typename T>
T client::run_sync(cobalt::task<T> task) {
    assert(owned_io_ && "sync methods require self-contained client");
    std::promise<T> p;
    auto f = p.get_future();
    asio::co_spawn(io_, std::move(task),
        [&p](std::exception_ptr ep, T val) {
            if (ep) p.set_exception(ep);
            else p.set_value(std::move(val));
        });
    return f.get(); // blocks caller, io_ thread drives completion
}
```

In component mode (caller provides io_context), only async methods are available.
Sync methods assert/throw — if you provide your own io_context, you're writing async code.

**1.3** Update `websocket_client` sync methods to accept a `run_sync` function instead of calling `cobalt::run` directly.

**1.4** Update `http_client` sync method similarly (optional — HTTP works today, but unifies the pattern).

### Phase 2: Transport Cleanup

**2.1** Remove unused `io_context&` parameter from `websocket_client` constructor (already ignored).

**2.2** Remove `io_context_` member from `market_streams` — will come from the parent client.

### Phase 3: Unified Client

**3.1** Restructure `include/binapi2/fapi/client.hpp` — add WS API and stream service members, lazy transports.

**3.2** Create `include/binapi2/fapi/websocket_api/service.hpp` — convert `websocket_api::client` into a service holding `client&`.

**3.3** Create `include/binapi2/fapi/streams/stream_service.hpp` — convert `market_streams` into a service holding `client&`.

**3.4** Implement lazy transport creation in unified client:
```cpp
websocket_client& client::ws_api_transport() {
    if (!ws_api_transport_)
        ws_api_transport_ = make_unique<websocket_client>(io_thread_.context(), cfg_);
    return *ws_api_transport_;
}
```

### Phase 4: Fix Sync Wrappers

**4.1** All sync methods route through `io_thread::run_sync` instead of `cobalt::run`. This fixes the WebSocket crash.

### Phase 5: Migration

**5.1** Keep old class names as deprecated aliases for one release cycle.

**5.2** Update demo-cli — each command uses unified `fapi::client`.

**5.3** Update `local_order_book` — takes single `client&` instead of separate `market_streams&` + `client&`.

**5.4** Update user_streams — becomes a service on the unified client.

### Phase 6: Build & Tests

**6.1** Update CMakeLists.txt — new source files.

**6.2** Update integration tests.

**6.3** Add io_thread unit tests.

## Risks

1. **`io_thread` lifetime** — background thread must join cleanly. If a coroutine is mid-flight during destruction → UB. Mitigation: `~io_thread()` stops work guard, lets pending work drain, then joins.

2. **`cobalt::task` + `co_spawn` compatibility** — verify `co_spawn` works with cobalt tasks on a plain `io_context`. May need `cobalt::spawn` instead. Test early in Phase 1.

3. **Lazy transport thread safety** — `ws_api_transport()` creation must be safe if called from multiple threads. Use `std::call_once` or ensure it's only called from the io_thread.

4. **Two construction modes** — `client(config)` owns io_context + thread (sync + async); `client(io_context&, config)` borrows for pure async only. Sync methods assert in component mode — clear contract, no silent deadlock. The caller chooses at construction time, mode is fixed for the object's lifetime.

5. **Circular headers** — `ws_api_service` and `stream_service` forward-declare `client`, define template methods in `client.hpp` or `client_impl.hpp`. Same pattern as existing REST services.

## Implementation Order

Phase 0 → 1 → 2 → 3 → 4 → 5 → 6

Phase 0 is independently shippable. Phases 1-4 are the core fix and should land together. Phase 5-6 are migration.

## Status

- **Phase 0**: Done — `json_read_opts` extracted, all raw `glz::read_json` calls fixed.
- **Phase 1**: Done — `io_thread` created, `websocket_client` dual constructor, stream crash fixed.
- **Phase 2**: Done — transport cleanup (kept legacy constructors for backward compat).
- **Phase 3**: Done — unified `client` with self-contained (`client(config)`) and component (`client(io_context&, config)`) constructors. Lazy `ws_api()`, `streams()`, `user_streams()` accessors.
- **Phase 4**: Done — sync wrappers route through `io_thread::run_sync` in owned mode.
- **Phase 5**: Done — demo-cli migrated (`cmd_ws_api.cpp`, `cmd_order_book.cpp` use unified client; `cmd_stream.cpp` uses `io_thread` directly).
- **Phase 6**: Done — all 308 unit tests pass, 18 postman mock integration tests pass.

Additionally fixed:
- TLS cert generation for mock server (added `CA:TRUE` basic constraint).
- Added `ca_cert_file` config option for custom CA certificates.
