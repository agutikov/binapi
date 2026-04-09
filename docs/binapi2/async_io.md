# Async I/O: Console, Logging, File Recording

## Implementation

In the async-first architecture, all coroutines run on a single event loop thread.
Blocking I/O -- `std::cout`, synchronous `spdlog::info(...)`, `ofstream::write()` --
would stall the entire event loop, delaying network reads and writes.

The solution: route all output through **spdlog async mode**.

---

## Current Design: spdlog Async Logger

All output in the async-demo-cli uses spdlog's built-in async mode: a thread pool
drains a lock-free queue to sinks.

### Initialization

```cpp
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sink_mt.h>

// Init (once at startup):
spdlog::init_thread_pool(8192, 1);  // queue size 8192, 1 worker thread
auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
auto logger = std::make_shared<spdlog::async_logger>("", sink, spdlog::thread_pool());
spdlog::set_default_logger(logger);
```

### Usage

All output goes through `demo::out()` which returns the async logger:

```cpp
// In coroutine code -- non-blocking (enqueues and returns immediately):
demo::out()->info("server_time: {}", time);
demo::out()->info("{} bid={} ask={}", event.symbol, event.best_bid_price, event.best_ask_price);
```

`demo::out()` replaces direct `std::cout` usage. This ensures the coroutine event loop
is never blocked by terminal I/O.

### Stream Recorder

The stream recorder uses a dedicated spdlog logger with raw output format:

```cpp
auto rec_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(record_file);
rec_sink->set_pattern("%v");  // raw payload, no timestamp prefix
auto rec_logger = std::make_shared<spdlog::async_logger>("rec", rec_sink, spdlog::thread_pool());

cfg.stream_recorder = [rec_logger](const std::string& payload) {
    rec_logger->info("{}", payload);  // non-blocking: enqueues to shared thread pool
};
```

Both the console logger and the recorder share the same spdlog thread pool, so there
is only one extra thread for all output I/O.

---

## Why spdlog Async

| Property | spdlog async |
|---|---|
| Blocking from coroutine? | No -- `info()` enqueues to lock-free queue and returns |
| Extra threads | +1 (spdlog thread pool worker) |
| Queue overflow policy | Configurable: `block` or `overrun_oldest` |
| Covers console? | Yes (stdout_color_sink_mt) |
| Covers file recording? | Yes (basic_file_sink_mt with raw pattern) |
| Custom code needed? | None -- spdlog built-in |

### Alternative approaches considered

| Approach | Verdict |
|---|---|
| `asio::posix::stream_descriptor` | Does not help for regular files on Linux (always reports ready). Only useful for pipes/terminals. |
| `cobalt::channel` + writer thread | Works but reinvents what spdlog async already provides. |
| Custom `async_writer` class | Same -- spdlog's MPSC queue + worker thread does the same thing. |

The spdlog-only approach was chosen because it requires zero custom async_writer code,
uses a single thread pool for all I/O, and provides a lock-free queue with configurable
overflow.

---

## `fmt::formatter<decimal_t>`

A `fmt::formatter<decimal_t>` specialization is provided so that decimal values can be
logged directly through spdlog/fmt:

```cpp
demo::out()->info("price: {}", event.price);  // decimal_t formatted correctly
```

---

## Sync Demo

The sync-demo (`examples/binapi2/fapi/sync-demo/`) demonstrates bridging patterns for
non-async code. It does not use spdlog async because blocking I/O is acceptable when
there is no event loop to stall. It shows:

- `io_thread` + `run_sync()` for blocking REST/WS API calls
- `std::async` + `cobalt::spawn(io, task, use_future)` for future-based access
- Manual `io_context` management

---

## Source Reference

| File | Role |
|---|---|
| `examples/binapi2/fapi/async-demo-cli/logging.hpp` | `demo::out()`, async logger initialization |
| `examples/binapi2/fapi/async-demo-cli/main.cpp` | `cobalt::main` entry, spdlog thread pool init |
| `include/binapi2/fapi/types/detail/decimal.hpp` | `fmt::formatter<decimal_t>` |
| `include/binapi2/fapi/config.hpp` | `stream_recorder` callback type |
