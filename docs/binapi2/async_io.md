# Async I/O Design: Console, Logging, File Recording

## Problem

In the async-first architecture, all coroutines run on a single event loop thread.
Any blocking I/O — `std::cout`, `spdlog::info(...)`, `ofstream::write()` — stalls
the entire event loop, delaying network reads and writes.

```
Event loop thread:
  [network read] [cout blocks 2ms] [network read delayed] [spdlog blocks 1ms] ...
```

For a CLI demo this is tolerable. For production (high-frequency streams, order execution),
console and file writes must not block the coroutine.

---

## Three I/O targets, one pattern

| Target | Current | Problem |
|---|---|---|
| `std::cout` / `std::cerr` | Direct write from coroutine | Blocks on terminal/pipe |
| `spdlog` sinks | Sync stdout + sync file sinks | Both block |
| Stream recorder (`ofstream`) | `stream_recorder` callback writes synchronously | Blocks per frame |

All three share the same shape: **coroutine produces text → something writes it to a fd**.

---

## Design Options

### Option A: spdlog async logger (minimal change)

spdlog has built-in async mode: a thread pool drains a lock-free queue to sinks.

```cpp
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sink_mt.h>

// Init (once):
spdlog::init_thread_pool(8192, 1);  // queue size, 1 worker thread
auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
auto logger = std::make_shared<spdlog::async_logger>("", sink, spdlog::thread_pool());
spdlog::set_default_logger(logger);

// Usage (unchanged):
spdlog::info("server_time: {}", time);  // non-blocking: enqueues, returns immediately
```

**Pros:** One-liner change. spdlog handles thread pool, queue, overflow.
**Cons:** Only covers spdlog. `std::cout` and recorder still block. Overflow policy: block or drop (configurable).

### Option B: asio stream_descriptor (true async fd I/O)

Wrap stdout/stderr/file fds in `asio::posix::stream_descriptor`, use `co_await async_write`.

```cpp
// Async console writer
class async_console {
    asio::posix::stream_descriptor out_;
public:
    async_console(asio::any_io_executor exec)
        : out_(exec, ::dup(STDOUT_FILENO)) {}

    cobalt::task<void> write(std::string msg) {
        co_await asio::async_write(out_, asio::buffer(msg), cobalt::use_op);
    }
};

// Usage from coroutine:
co_await console.write("server_time: 12345\n");
```

Same pattern for file recording:

```cpp
class async_file_writer {
    asio::posix::stream_descriptor fd_;
public:
    async_file_writer(asio::any_io_executor exec, const std::string& path)
        : fd_(exec, ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) {}

    cobalt::task<void> write(std::string data) {
        co_await asio::async_write(fd_, asio::buffer(data), cobalt::use_op);
    }
};
```

**Pros:** True non-blocking I/O. Integrates with the event loop (yields on write stall).
**Cons:** Only works on POSIX (Linux/macOS). `stream_descriptor` for regular files on Linux uses epoll which doesn't actually work for regular files (always reports ready). **Regular files on Linux are always "ready" — async_write on a file fd completes synchronously anyway.** This only helps for pipes and terminals.

### Option C: cobalt::channel (coroutine-native buffered queue)

Decouple producers (coroutines) from consumers (writer thread/task) via a cobalt channel.

```cpp
template<typename T>
using channel = boost::cobalt::channel<T>;

// Shared channel
channel<std::string> log_chan;

// Producer (any coroutine):
co_await log_chan.push("server_time: 12345\n");

// Consumer (spawned once):
cobalt::task<void> drain_to_stdout(channel<std::string>& ch) {
    while (auto msg = co_await ch.pop()) {
        std::cout << *msg;  // blocks, but on dedicated consumer — not on main event loop
    }
}
```

Wait — the consumer also runs on the event loop. To truly not block:

**Channel + dedicated writer thread:**

```cpp
class async_writer {
    std::thread thread_;
    std::queue<std::string> queue_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool done_{false};

public:
    async_writer(std::ostream& out) : thread_([this, &out] { run(out); }) {}
    ~async_writer() { close(); if (thread_.joinable()) thread_.join(); }

    // Called from any thread (including coroutine thread) — non-blocking.
    void write(std::string msg) {
        { std::lock_guard lk(mu_); queue_.push(std::move(msg)); }
        cv_.notify_one();
    }

    void close() {
        { std::lock_guard lk(mu_); done_ = true; }
        cv_.notify_one();
    }

private:
    void run(std::ostream& out) {
        while (true) {
            std::unique_lock lk(mu_);
            cv_.wait(lk, [this] { return !queue_.empty() || done_; });
            while (!queue_.empty()) {
                auto msg = std::move(queue_.front());
                queue_.pop();
                lk.unlock();
                out << msg;
                lk.lock();
            }
            if (done_ && queue_.empty()) break;
        }
        out.flush();
    }
};
```

**Pros:** Truly non-blocking from coroutine perspective. Works for stdout, stderr, and files.
**Cons:** Extra thread per writer. Queue memory growth if consumer is slow. Need overflow policy.

### Option D: spdlog custom sink + async_writer (unified)

Use spdlog as the single output facility. Create a custom sink backed by `async_writer`:

```cpp
class async_writer_sink : public spdlog::sinks::base_sink<std::mutex> {
    async_writer writer_;
public:
    async_writer_sink(std::ostream& out) : writer_(out) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t buf;
        formatter_->format(msg, buf);
        writer_.write(std::string(buf.data(), buf.size()));
    }
    void flush_() override {}
};
```

Then replace both console and file sinks:

```cpp
auto console_sink = std::make_shared<async_writer_sink>(std::cout);
auto file_sink = std::make_shared<async_writer_sink>(file_stream);
auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{console_sink, file_sink});
```

For the stream recorder, use `async_writer` directly:

```cpp
async_writer recorder(record_file_stream);
cfg.stream_recorder = [&recorder](const std::string& payload) {
    recorder.write(payload + "\n");
};
```

**Pros:** Single pattern for all three targets. spdlog API unchanged. Non-blocking from coroutine.
**Cons:** Extra thread(s). Not truly async I/O (thread blocks on write), but the coroutine doesn't.

---

## Recommended Design

### Layer 1: `async_writer` — generic non-blocking writer

A thread-safe queue + dedicated writer thread. The `write()` method is non-blocking
(just enqueues). The background thread drains to the target ostream.

```
coroutine ──write()──► queue ──► writer thread ──► ostream (cout/file)
            (fast)     (lock-free or mutex)         (blocks here, not on event loop)
```

Configuration:
- `max_queue_size`: bounded queue (prevent OOM)
- `overflow_policy`: `block` (back-pressure), `drop_oldest`, `drop_newest`

### Layer 2: spdlog integration

Replace sync sinks with `async_writer`-backed sinks. Or just use spdlog's built-in async
(Option A) — it has the same architecture internally.

### Layer 3: Stream recorder

`stream_recorder` callback wraps an `async_writer` to a file.

### Implementation plan

| Component | Change | Effort |
|---|---|---|
| `async_writer` class | New: `detail/async_writer.hpp` | ~50 lines |
| `init_logging()` in demo | Replace sync sinks with spdlog async or async_writer sinks | ~10 lines |
| `stream_recorder` in demo | Wrap `async_writer` around ofstream | ~5 lines |
| `std::cout` in commands | Replace with spdlog calls (already partially done) or async_writer | ~20 lines |

### Alternative: spdlog async only (simplest)

If we're OK with spdlog being the only output path (no raw `std::cout`):

1. Switch to `spdlog::async_logger` in `init_logging()` — 3-line change
2. Route all output through spdlog (replace `std::cout << ...` with `spdlog::info(...)`)
3. Recorder: use spdlog file sink with custom pattern (raw payload, no timestamp)

This avoids writing `async_writer` entirely. spdlog's async mode uses a lock-free MPSC queue
with a configurable overflow policy (`block` or `overrun_oldest`).

---

## Decision matrix

| Approach | Blocking? | Threads | Complexity | Covers |
|---|---|---|---|---|
| A: spdlog async | No | +1 (spdlog pool) | Minimal | spdlog only |
| B: stream_descriptor | No* | 0 | Medium | stdout/pipe only, not files |
| C: channel + thread | No | +1 per target | Medium | all |
| D: async_writer + spdlog sink | No | +1 shared | Medium | all |
| spdlog-only (all output via spdlog) | No | +1 (spdlog pool) | Minimal | all (if we route cout through spdlog) |

\* stream_descriptor doesn't help for regular files on Linux

**Recommendation:** Route everything through spdlog async. For the stream recorder, use a
dedicated spdlog logger with a raw-output file sink. This gives us:
- One thread pool (spdlog's) for all I/O
- Lock-free queue with configurable overflow
- Zero custom async_writer code
- Existing spdlog formatters for console, file, recorder

```cpp
// Console + file log
spdlog::init_thread_pool(8192, 1);
auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
auto logger = std::make_shared<spdlog::async_logger>("", console, spdlog::thread_pool());

// Stream recorder — separate logger with raw format
auto rec_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(record_file);
rec_sink->set_pattern("%v");  // raw payload, no prefix
auto rec_logger = std::make_shared<spdlog::async_logger>("rec", rec_sink, spdlog::thread_pool());
cfg.stream_recorder = [rec_logger](const std::string& payload) {
    rec_logger->info("{}", payload);
};
```
