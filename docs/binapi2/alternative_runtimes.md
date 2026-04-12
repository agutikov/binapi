# Alternative async runtimes: transition analysis

This document evaluates 12 alternative async/coroutine libraries as potential
replacements for binapi2's current runtime stack. It is descriptive, not
prescriptive — the goal is to make informed decisions, not to recommend a
single winner.

## Current stack

binapi2 sits on three Boost components plus OpenSSL:

| Layer | Library | Role |
|-------|---------|------|
| Execution | **Boost.ASIO** | Executor, resolver, io_context, timers |
| Coroutines | **Boost.Cobalt** | `task<T>`, `generator<T>`, `main`, `join`, `spawn` |
| Protocol | **Boost.Beast** | HTTPS request/response, WebSocket framing |
| TLS | OpenSSL | X.509, TLS handshake, HMAC, Ed25519 |

### Binding points

Any runtime transition must replace or adapt these three Boost pieces. The
cost depends on how much of the public API is expressed in terms of Cobalt
types:

- **Public API surface:** every `async_*` method returns
  `boost::cobalt::task<result<T>>` or `boost::cobalt::generator<result<T>>`.
  54 such declarations in the public headers.
- **Transport layer:** `src/binapi2/fapi/transport/{http_client,websocket_client}.cpp`
  — localized to two files. Beast integrates with ASIO via templates, so
  replacing ASIO means replacing Beast.
- **Tests and examples:** `cobalt::main` entry points in demo CLI and sync
  demos; `io_thread::run_sync()` for sync bridging.

A transition is **small** if it preserves the ASIO executor model and only
swaps the coroutine type (`task<T>` → `some_task<T>`). It is **large** if it
replaces the executor, forcing a rewrite of transport and all services.

## Analysis table

Sorted by the three requested axes. Values are relative expectations for
binapi2's workload (Binance US-M Futures client: tens to hundreds of REST
calls per second, handful of persistent WebSocket streams, cross-platform
Linux/macOS). **Lower latency is better; higher throughput is better; lower
complexity is better.**

### Sorted by expected latency (best → worst)

| Rank | Library | Latency | Why |
|------|---------|:-------:|-----|
| 1 | **Seastar** | ★★★★★ | Shared-nothing, one thread per core, optional DPDK kernel bypass |
| 2 | **liburing4cpp** | ★★★★★ | Thin C++ wrapper over io_uring; no abstraction overhead |
| 3 | **Folly** (coro+io) | ★★★★ | io_uring backend, aggressive inlining, Facebook-scale optimizations |
| 4 | **Boost.ASIO + io_uring** (current) | ★★★★ | `BOOST_ASIO_HAS_IO_URING` available; comparable to Folly for this workload |
| 5 | **Boost.Cobalt** (current) | ★★★★ | Stackless C++20 coroutines with zero heap alloc for typical tasks |
| 6 | **CppCoro** | ★★★★ | Stackless coroutines, similar profile to Cobalt |
| 7 | **packio** | ★★★★ | Built on ASIO — inherits its latency characteristics |
| 8 | **uvw** (libuv) | ★★★ | Callback-based; libuv uses epoll, not io_uring |
| 9 | **Boost.Fibers** | ★★★ | Stackful context switches (~100 ns vs ~10 ns for stackless) |
| 10 | **bthread** | ★★★ | Baidu's work-stealing fibers; stackful overhead |
| 11 | **libco** | ★★★ | Stackful + syscall hooking; dated design |
| 12 | **async++** | ★★ | Task-parallelism library, not I/O-focused |
| 13 | **asyncio (C++ port)** | ★ | Hobby ports, not optimized |

### Sorted by expected throughput (best → worst)

| Rank | Library | Throughput | Why |
|------|---------|:----------:|-----|
| 1 | **Seastar** | ★★★★★ | Zero-copy, no locks, no malloc on hot path, DPDK-ready |
| 2 | **Folly** | ★★★★★ | Production-proven at Facebook scale, IOBuf zero-copy |
| 3 | **liburing4cpp** | ★★★★ | io_uring batched submission; limited by your stack on top |
| 4 | **Boost.ASIO + io_uring** (current) | ★★★★ | Good enough for millions of RPS with tuning |
| 5 | **Boost.Cobalt** (current) | ★★★★ | No overhead beyond ASIO |
| 6 | **bthread** | ★★★★ | brpc hits 10M+ RPS in benchmarks |
| 7 | **CppCoro** | ★★★ | Similar to Cobalt but less optimized for I/O |
| 8 | **packio** | ★★★ | ASIO-based |
| 9 | **uvw** (libuv) | ★★★ | libuv caps out around 1M ops/s |
| 10 | **Boost.Fibers** | ★★ | Stack-switching overhead dominates at high RPS |
| 11 | **libco** | ★★ | Older design, limited scalability |
| 12 | **async++** | ★★ | Not designed for I/O fan-out |
| 13 | **asyncio (C++ port)** | ★ | Not production |

### Sorted by complexity of transition (cheapest → most expensive)

| Rank | Library | Complexity | Scope |
|------|---------|:----------:|-------|
| 1 | **packio** | ★ | *Adds* RPC on top of ASIO; no replacement needed |
| 2 | **Boost.Fibers** | ★★ | Keep ASIO + Beast; change coroutine model |
| 3 | **CppCoro** | ★★ | Swap `cobalt::task` for `cppcoro::task` (same stackless semantics) |
| 4 | **async++** | ★★★ | Swap task type; keep Beast + ASIO |
| 5 | **uvw** (libuv) | ★★★★ | Replace ASIO executor, Beast, and rewrite callback → coroutine adapters |
| 6 | **liburing4cpp** | ★★★★★ | Replace ASIO + rebuild HTTP and WebSocket from scratch |
| 7 | **Folly** | ★★★★★ | Replace everything (ASIO, Cobalt, Beast, OpenSSL integration) |
| 8 | **bthread** | ★★★★★ | Full rewrite in brpc idioms; uses its own I/O model |
| 9 | **libco** | ★★★★★ | LD_PRELOAD syscall hooking incompatible with ASIO; total rewrite |
| 10 | **asyncio (C++)** | ★★★★★ | No stable reference implementation |
| 11 | **Seastar** | ★★★★★★ | Full rewrite on Seastar runtime + shared-nothing redesign |
| — | **libufinex** | ? | **Not a recognized library.** Possibly a typo for `liburing` (already covered) or a project name I cannot identify; omitted from analysis below. |

## Per-library analysis

Each entry includes: **what you get**, **what you lose**, **transition
scope**, and **does it make sense for binapi2**.

### Folly (+ folly::coro)

**What you get:** `folly::coro::Task`, `folly::SemiFuture`, `folly::IOBuf`
(zero-copy buffer), `folly::fibers` if you want stackful, extensive thread
pool and executor primitives, io_uring backend via `folly::io::async`.
Battle-tested at Facebook scale.

**What you lose:** Boost integration. Folly's executor model is incompatible
with ASIO — switching means rewriting transport layer against
`folly::EventBase` and replacing Beast with `folly::io::async::HTTPClient` or
`proxygen`.

**Transition scope:** Large. Rewrite transport + every public signature
(`cobalt::task<T>` → `folly::coro::Task<T>`). Examples and tests all need
new entry points. Folly has ~3GB build artifacts and dozens of system
dependencies.

**Make sense?** Only if binapi2 is being absorbed into a Folly-based
codebase (e.g. a Facebook-derived trading platform). Standalone, the build
and dependency cost outweighs any latency gain.

---

### libufinex

Not a library I can identify. If the intended reference is **liburing**
(the kernel io_uring userspace library), see liburing4cpp below. If it's a
crypto-domain library I don't recognize, please clarify.

---

### Seastar

**What you get:** An entire runtime — thread-per-core shared-nothing
scheduler, `seastar::future<T>` and `seastar::coroutine::task<T>`,
lock-free inter-shard messaging, optional DPDK for kernel-bypass networking,
built-in HTTP server and client, own memory allocator. ScyllaDB achieves
millions of ops/s per core with this stack.

**What you lose:** Interop with anything not written for Seastar. Seastar
owns `main()`, owns the memory allocator, and requires code to be
"shard-aware" (thread-local state, no shared-across-threads objects). OpenSSL
is supported but Binance TLS sessions are still one per stream.

**Transition scope:** Full rewrite. The `client` / `service` layout wouldn't
survive — every method would be rephrased in terms of `seastar::future`.
Tests and demo CLI would need Seastar's app framework. Cross-platform
support is Linux-only (no macOS).

**Make sense?** Only for extreme HFT use cases where the application is
co-designed with Seastar on dedicated hardware with DPDK NICs. For a
general-purpose client library, no — you lose portability and simplicity
for latency gains that REST/WSS handshakes dominate anyway (TLS alone
costs ~1–5 ms).

---

### Boost.Fibers

**What you get:** Stackful coroutines that look like threads but cooperate.
`boost::fibers::asio::yield` integrates with ASIO. Familiar API for teams
used to threads.

**What you lose:** Stackless efficiency. Fiber context switches are
~100× slower than stackless coroutine suspension (~100 ns vs ~1 ns for the
trivial path). Each fiber owns a stack (typically 64 KB) — much more memory
per concurrent operation.

**Transition scope:** Small. Keep ASIO + Beast. Replace
`boost::cobalt::task<T>` with `void` and synchronous-looking fiber code.

**Make sense?** No. C++20 coroutines (the foundation of Cobalt) were
designed specifically to replace stackful fibers in the C++ world because
they're faster and cheaper. Moving backward would regress both latency and
memory.

---

### CppCoro

**What you get:** Lewis Baker's original coroutine library — `task<T>`,
`generator<T>`, `async_mutex`, `async_scope`, cancellation. Predates
`std::coroutine`.

**What you lose:** Active maintenance. CppCoro has been essentially frozen
since 2020 because the standard library ate most of its concepts.
No integrated networking.

**Transition scope:** Small. CppCoro's `task<T>` is semantically close to
Cobalt's. You'd keep ASIO + Beast and change the task type alias.

**Make sense?** No. Cobalt is the actively-maintained Boost successor to
CppCoro's ideas, and it's already integrated with ASIO. You'd be moving to
a less-maintained equivalent.

---

### liburing4cpp

**What you get:** Raw io_uring access with RAII wrappers. Batched submission,
kernel-side polling (SQPOLL), zero-syscall I/O on the hot path. Linux kernel
5.1+ only.

**What you lose:** *Everything above the syscall layer.* No HTTP, no TLS,
no WebSocket. You write the protocol stack yourself — TLS bytes in, HTTP
bytes out, WebSocket framing, JSON parsing. Cross-platform support: Linux
only.

**Transition scope:** Very large — effectively a rewrite of the transport
layer plus protocol reconstruction. Beast disappears. OpenSSL integration
becomes manual (you feed bytes to `SSL_read`/`SSL_write` and shepherd them
to/from io_uring buffers).

**Make sense?** Only if latency is the single dominant concern and you have
engineers comfortable maintaining a custom HTTP/TLS/WS stack. For a client
library, **no** — and binapi2 already benefits from io_uring via
`BOOST_ASIO_HAS_IO_URING`, which gets you 80 % of the gain with none of
the rewrite.

---

### libco

**What you get:** Tencent's stackful coroutines with LD_PRELOAD-based
syscall hooking — socket calls transparently cooperate. Used in some WeChat
backends.

**What you lose:** Compatibility with modern C++ and with any library that
uses its own I/O loop (Beast, ASIO). The syscall hook model actively
conflicts with ASIO's epoll loop.

**Transition scope:** Large. ASIO and Beast don't work with libco. You'd
rewrite the transport against raw BSD sockets and let libco's hooks turn
them into cooperative operations. No C++23 idioms, C-style API.

**Make sense?** No. libco is designed for a different era — server-side
C code that wants "threads without threads". For a C++23 client library,
it's the wrong abstraction layer.

---

### bthread

**What you get:** Baidu's brpc fiber library. Work-stealing scheduler,
high throughput, integrated with brpc's RPC framework. Fibers are
lightweight (not OS threads) but stackful.

**What you lose:** Standalone usability. bthread is tightly coupled to brpc
— extracting it is painful. Stackful cost, similar to Boost.Fibers.

**Transition scope:** Very large. Essentially a rewrite against brpc's
synchronous-looking API. No standard coroutine integration.

**Make sense?** No. bthread is optimal for its native environment (brpc
RPC servers) but offers no advantage for a Binance client. You'd pay all
the rewrite cost for no latency gain and lose C++23 ergonomics.

---

### uvw

**What you get:** A modern C++ wrapper around libuv (the Node.js event
loop). Cross-platform, mature, callback-based API with optional coroutine
adapters.

**What you lose:** ASIO executor model. libuv uses epoll/kqueue, not
io_uring. Beast doesn't work without ASIO — you'd replace it with a libuv
HTTP client (there are several: `nghttp2`, `uvw`'s own facility is limited
to basic sockets, you'd pair it with `llhttp`).

**Transition scope:** Large. Rewrite transport against libuv handles,
glue coroutines to libuv callbacks, provide your own HTTP and WebSocket
parsing.

**Make sense?** No. libuv is excellent for Node.js-style applications but
for a C++23 coroutine-first library, ASIO is the more natural fit and
already provides the same cross-platform support.

---

### async++

**What you get:** Amanieu's task-parallelism library — `task<T>`,
`when_all`, `parallel_for`. Continuation-based, not coroutine-based.
Good for fan-out compute, not async I/O.

**What you lose:** Integrated I/O. async++ has no networking layer; you'd
still need Beast or similar to do the actual HTTP/WS work, then wrap each
call as an async++ task.

**Transition scope:** Medium if you keep Beast + ASIO and only change the
wrapper type. But you'd lose the `co_await` idiom — async++ uses
`.then(...)` continuations.

**Make sense?** No. For binapi2's workload (sequential request/response +
event streams), C++20 coroutines are strictly better than continuation
chains. You'd regress the API ergonomics.

---

### asyncio (C++ port)

There is no canonical C++ port of Python's asyncio. Several research or
hobby implementations exist but none are production-ready. I cannot
recommend any for a trading client.

**Transition scope:** Uncharted.

**Make sense?** No.

---

### packio

**What you get:** A client/server RPC library built on ASIO, supporting
msgpack-rpc, JSON-RPC, and nljson-rpc with coroutine (`asio::awaitable`)
or future API. Plays nicely with Cobalt via asio interop.

**What you lose:** Nothing — it's an *addition*, not a replacement.

**Transition scope:** None. You'd add it alongside the existing stack if
you want to expose a JSON-RPC interface on top of binapi2 (for example to
let a Python or Node frontend talk to a C++ trading daemon).

**Make sense?** Maybe, for a **different** purpose. packio doesn't replace
anything — it's a complementary RPC layer for building a C++ trading
service on top of binapi2. Worth adding if you're building such a service;
irrelevant if you're using binapi2 directly.

---

### liburing4cpp vs Boost.ASIO's io_uring backend

Worth emphasizing: Boost.ASIO already has an io_uring backend that binapi2
uses in production. Defining `BOOST_ASIO_HAS_IO_URING` at build time
switches ASIO's reactor to io_uring, capturing most of the kernel-side
latency improvement without abandoning the ASIO abstraction.

The marginal gain from liburing4cpp over ASIO+io_uring comes from:
- Batched SQE submission across multiple concurrent requests (ASIO submits
  one at a time by default).
- SQPOLL mode (kernel thread polls submission queue with no userspace
  syscalls).
- IORING_FEAT_FAST_POLL (poll-less short reads).

For binapi2's workload — persistent TLS connections sending signed REST
requests at tens of Hz, plus WebSocket frames — these optimizations
contribute at the microsecond level, while TLS handshake and signing
dominate at the millisecond level. The economic case for dropping ASIO is
weak.

---

## Summary recommendation

**For binapi2 specifically, no transition is justified today.** The current
stack (Boost.ASIO + Boost.Cobalt + Boost.Beast) is:

1. **The best fit for C++23 coroutines.** Cobalt is the actively-maintained
   Boost coroutine library and integrates natively with ASIO and Beast.
2. **Already io_uring-capable.** ASIO picks it up when
   `BOOST_ASIO_HAS_IO_URING` is defined.
3. **Cross-platform.** Linux, macOS, Windows — important for a library
   users deploy on varied environments.
4. **Well-documented and well-supported.** Every competing runtime has
   either less documentation, tighter platform constraints, or a different
   philosophical model that would force a rewrite.

**Scenarios where a transition might still be considered:**

| Scenario | Candidate | Rationale |
|----------|-----------|-----------|
| Building a C++ trading service with RPC frontend | packio (add, don't replace) | Natural extension of current stack |
| Deploying on dedicated HFT hardware with DPDK NICs | Seastar (full rewrite) | Only runtime that meaningfully beats ASIO+io_uring |
| Integrating into an existing Folly codebase | Folly | Reduces cross-runtime glue code |
| Needing raw io_uring tunables (SQPOLL, fixed buffers) | liburing4cpp (transport layer only) | Keep public API, rewrite transport |
| Team strongly prefers stackful coroutines | Boost.Fibers | Keeps Boost ecosystem |

**Scenarios where the answer is unambiguously no:**
CppCoro, libco, bthread, uvw, async++, asyncio-ports, Boost.Fibers (as
anything other than a team preference), liburing4cpp as a whole-stack
replacement.

## Transition cost estimate (hypothetical)

If a transition were pursued, rough engineering cost in staff-weeks
(assuming one engineer familiar with binapi2, no new features added during
the port):

| Target | Transport rewrite | Public API change | Tests + examples | Total (staff-weeks) |
|--------|:-:|:-:|:-:|:-:|
| packio (addition) | 0 | 0 | 1 | **1** |
| Boost.Fibers | 1 | 3 | 2 | **6** |
| CppCoro | 0 | 2 | 1 | **3** |
| async++ | 1 | 3 | 2 | **6** |
| uvw | 4 | 3 | 2 | **9** |
| Folly | 6 | 4 | 3 | **13** |
| liburing4cpp (transport only) | 8 | 0 | 2 | **10** |
| Seastar | 10 | 6 | 4 | **20** |

These numbers exclude the ongoing maintenance cost of tracking upstream
changes in the new runtime — an important consideration because Boost is
released on a predictable cadence and the alternative runtimes vary from
actively maintained (Folly, Seastar) to dormant (CppCoro, libco, libufinex?).

## See also

- [DESIGN.md](DESIGN.md) — current architecture and layer model
- [threading_and_io.md](threading_and_io.md) — executor ownership contract
- [async_architecture.md](async_architecture.md) — why async-first
- [streams.md](streams.md) — stream component design
