# Plan: stream_buffer<T> and stream_recorder

## Context

The current `stream_recorder` is a simple wrapper around `cobalt::channel<string>` with a sink callback. It has ownership and lifecycle problems: the caller must manage the recorder instance, attach it to the connection via raw pointer, and run `async_drain()` concurrently using `cobalt::join`. The user wants a proper layered architecture:

1. A generic `stream_buffer<T>` — reusable channel wrapper with multiple consumption interfaces
2. A self-contained `stream_recorder` — owns its own `io_thread`, records multiple streams to files, works standalone

## Architecture

```
stream_buffer<T>  (detail/)         — generic typed channel wrapper, single-executor
    ↑ used by
stream_connection (streams/)        — holds optional stream_buffer<string>*, feeds frames into it
    ↑ used by
stream_recorder   (streams/)        — multi-stream file recorder with own io_thread
    creates buffers on its executor
    attaches them to connections
    drains all via single cobalt::race coroutine
```

### Cross-executor design

`stream_buffer<T>` wraps `cobalt::channel<T>` which is single-executor. The recorder's `io_thread` runs a different executor than the stream connection. The `async_push()` method uses executor-hopping (`asio::post(channel_executor, use_op)`) to safely write from any executor with backpressure preserved.

### drain_all via race

`cobalt::race(vector<channel_reader<string>>)` returns `pair<size_t, string>` — the index of which channel produced data + the frame. The single drain coroutine loops: build readers for all open channels, race them, dispatch the result to the correct sink by index.

## Files to create/modify

### New: `include/binapi2/fapi/detail/stream_buffer.hpp`

```cpp
namespace binapi2::fapi::detail

template<typename T>
class stream_buffer {
public:
    explicit stream_buffer(std::size_t buffer_size);

    // Producer (safe from any executor — hops to channel's executor and back)
    cobalt::task<void> async_push(T value);
    void close();
    bool is_open() const;

    // Consumer interfaces (must run on channel's executor)
    // 1. Pull single
    cobalt::task<result<T>> async_read();
    // 2. Pull batch
    cobalt::task<result<std::vector<T>>> async_read_batch(std::size_t max_count);
    // 3. Sink + drain
    cobalt::task<void> async_drain(std::function<void(T)> sink);
    // 4. channel_reader for use with cobalt::race
    cobalt::channel_reader<T> reader();

private:
    cobalt::channel<T> channel_;
};
```

`async_push` implementation (executor hopping):
```cpp
cobalt::task<void> async_push(T value) {
    auto caller_exec = co_await cobalt::this_coro::executor;
    co_await asio::post(channel_.get_executor(), cobalt::use_op);
    // Now on channel's executor — safe to write
    co_await channel_.write(std::move(value));
    // Hop back to caller's executor
    co_await asio::post(caller_exec, cobalt::use_op);
}
```

### Modify: `include/binapi2/fapi/streams/stream_connection.hpp`

- Replace `stream_recorder* recorder_` with `detail::stream_buffer<std::string>* buffer_`
- Replace `set_recorder(stream_recorder&)` with `attach_buffer(detail::stream_buffer<std::string>&)` / `detach_buffer()`
- In `async_read_text()`: `co_await buffer_->async_push(*msg)` instead of `co_await recorder_->async_record(*msg)`

### Rewrite: `include/binapi2/fapi/streams/stream_recorder.hpp`

The recorder is a `basic_stream_recorder<Sink>` template, parameterized on sink type.
The sink concept requires `void operator()(const std::string&)` or
`cobalt::task<void> operator()(const std::string&)` for async sinks.

```cpp
namespace binapi2::fapi::streams

template<typename Sink>
class basic_stream_recorder {
public:
    explicit basic_stream_recorder(std::size_t buffer_size_per_stream);
    ~basic_stream_recorder();  // stops, closes all channels, joins io_thread

    // Create a buffer on recorder's executor. Returns ref to attach to connection.
    detail::stream_buffer<std::string>& add_stream(Sink sink);

    void start();  // spawns drain coroutine on io_thread
    void stop();   // closes all channels, drain finishes

private:
    struct stream_entry {
        std::unique_ptr<detail::stream_buffer<std::string>> buffer;
        Sink sink;
    };

    detail::io_thread io_;
    std::size_t buffer_size_;
    std::vector<stream_entry> streams_;

    cobalt::task<void> async_drain_all();
};
```

`async_drain_all` — single coroutine using `cobalt::race`:
```cpp
cobalt::task<void> async_drain_all() {
    while (true) {
        std::vector<cobalt::channel_reader<std::string>> readers;
        std::vector<std::size_t> idx_map;
        for (std::size_t i = 0; i < streams_.size(); ++i) {
            if (streams_[i].buffer->is_open()) {
                readers.push_back(streams_[i].buffer->reader());
                idx_map.push_back(i);
            }
        }
        if (readers.empty()) break;

        auto rv = co_await cobalt::as_result(cobalt::race(readers));
        if (!rv) break;
        auto [pos, value] = *rv;
        // Call sink — sync or async depending on Sink type
        if constexpr (returns_task<Sink>) {
            co_await streams_[idx_map[pos]].sink(value);
        } else {
            streams_[idx_map[pos]].sink(value);
        }
    }
}
```

### Sink implementations

Three sink types, each usable as the `Sink` template parameter for `basic_stream_recorder`:

#### 1. Generic callback sink (sync) — `include/binapi2/fapi/streams/sinks/callback_sink.hpp`

```cpp
namespace binapi2::fapi::streams::sinks

class callback_sink {
public:
    using callback_fn = std::function<void(const std::string&)>;
    explicit callback_sink(callback_fn fn);
    void operator()(const std::string& frame);
private:
    callback_fn fn_;
};

// Type alias for convenience
using stream_recorder = basic_stream_recorder<callback_sink>;
```

#### 2. Async file sink — `include/binapi2/fapi/streams/sinks/file_sink.hpp`

Uses `boost::asio::stream_file` for truly async file I/O. Each frame is written
as a JSONL line (frame + `'\n'`). The sink's `operator()` returns `cobalt::task<void>`
so the drain coroutine `co_await`s the write.

```cpp
namespace binapi2::fapi::streams::sinks

class file_sink {
public:
    // Opens file for write (truncate). The io_context must be the recorder's.
    file_sink(boost::asio::io_context& ctx, const std::filesystem::path& path);

    cobalt::task<void> operator()(const std::string& frame);

private:
    boost::asio::stream_file file_;
};

// Type alias
using file_stream_recorder = basic_stream_recorder<file_sink>;
```

Implementation:
```cpp
cobalt::task<void> file_sink::operator()(const std::string& frame) {
    std::string line = frame + '\n';
    co_await asio::async_write(file_, asio::buffer(line), cobalt::use_op);
}
```

The `file_sink` needs a reference to the recorder's `io_context` to open the file.
`basic_stream_recorder` exposes `io_context&` from its `io_thread` for this purpose:
```cpp
// In basic_stream_recorder:
boost::asio::io_context& io_context() noexcept { return io_.context(); }

// Usage:
file_stream_recorder recorder(4096);
auto& buf = recorder.add_stream(sinks::file_sink(recorder.io_context(), "btcusdt.jsonl"));
```

#### 3. spdlog sink — `include/binapi2/fapi/streams/sinks/spdlog_sink.hpp`

Wraps a `shared_ptr<spdlog::logger>`. Sync — spdlog's own async thread pool handles
the actual I/O. Useful when the demo CLI already has spdlog infrastructure.

```cpp
namespace binapi2::fapi::streams::sinks

class spdlog_sink {
public:
    explicit spdlog_sink(std::shared_ptr<spdlog::logger> logger);
    void operator()(const std::string& frame);
private:
    std::shared_ptr<spdlog::logger> logger_;
};

// Type alias
using spdlog_stream_recorder = basic_stream_recorder<spdlog_sink>;
```

Implementation:
```cpp
void spdlog_sink::operator()(const std::string& frame) {
    logger_->info("{}", frame);
}
```

### Modify: `examples/binapi2/fapi/async-demo-cli/`

- Revert `run_with_recorder` wrapper from common.hpp/common.cpp
- Revert cmd_stream.cpp and cmd_user_stream.cpp to original form
- In main.cpp: if `--record` flag set, create a `spdlog_stream_recorder` using the
  existing `"rec"` spdlog logger (already set up in `init_logging()`), pass buffer refs
  to stream commands
- Each stream command accepts an optional `stream_buffer<string>*` and calls `attach_buffer`
  if non-null

Usage in main.cpp:
```cpp
// After init_logging() and before dispatching commands:
std::unique_ptr<streams::spdlog_stream_recorder> recorder;
detail::stream_buffer<std::string>* rec_buffer = nullptr;
if (auto rec_logger = spdlog::get("rec")) {
    recorder = std::make_unique<streams::spdlog_stream_recorder>(4096);
    rec_buffer = &recorder->add_stream(streams::sinks::spdlog_sink(rec_logger));
    recorder->start();
}

// Pass rec_buffer to stream commands...
// On shutdown:
if (recorder) recorder->stop();
```

### New: `tests/binapi2/fapi/stream_buffer_test.cpp`

- Same-executor tests: push/read, push/drain, push/batch, push/generator
- Backpressure test (small buffer)
- Close-while-reading test
- Empty buffer test

### Modify: `tests/binapi2/fapi/stream_test.cpp`

- Update existing StreamRecorder tests to use new stream_buffer + attach_buffer API
- Add cross-executor test using stream_recorder with io_thread

### Modify: `tests/binapi2/fapi/CMakeLists.txt`

- Add `stream_buffer_test` target

## Implementation order

1. `stream_buffer<T>` — the foundation
2. Modify `stream_connection` — swap recorder pointer for buffer pointer
3. Update existing stream tests — verify nothing breaks
4. `basic_stream_recorder<Sink>` — multi-stream with io_thread and race-based drain
5. `callback_sink` — generic sync sink, type alias `stream_recorder`
6. `file_sink` — async file I/O via `asio::stream_file`
7. `spdlog_sink` — sync sink wrapping spdlog async logger
8. Stream recorder tests — same-executor and cross-executor, all sink types
9. Update demo CLI — revert `run_with_recorder`, use `spdlog_stream_recorder`
10. Build and run full test suite

## Verification

```bash
./build.sh && ./run_tests.sh
./_build/tests/binapi2/fapi/stream_buffer_test
./_build/tests/binapi2/fapi/stream_test --gtest_filter='StreamRecorder*'
```
