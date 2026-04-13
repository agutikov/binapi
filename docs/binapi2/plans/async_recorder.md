# async-recorder — design and plan

Location: `examples/binapi2/fapi/async-recorder`
Source brief: `async-recorder.md` (repo root)

## 1. Goal

An institutional-grade screener + recorder for Binance USDⓈ-M Futures that
produces append-only, primitive-only event logs suitable for deterministic
replay, fill simulation, factor research, and execution research — while
keeping storage bounded by only recording *detailed* data for a dynamically
selected, stable set of **active** instruments.

Non-goals: indicators, derived features, strategy logic. Only raw primitives
are persisted; everything else is reconstructible offline.

## 2. Three-tier pipeline

The recorder is organized into three independent stages, each running on its
own cobalt coroutine and communicating through in-process channels.

```
 ┌─────────────┐   active set    ┌──────────────────┐  symbol add/remove   ┌────────────────────┐
 │ 1. Screener │ ──────────────▶ │ 2. Selector      │ ────────────────────▶│ 3. Detail monitor  │
 │ (all-*)     │                 │ (scoring + rules)│                      │ (per-symbol feeds) │
 └──────┬──────┘                 └────────┬─────────┘                      └─────────┬──────────┘
        │ raw jsonl                       │ signals.jsonl                            │ tier-0..2 jsonl
        ▼                                 ▼                                          ▼
      screener/*.jsonl(.zst)         selector/signals.jsonl              detail/<SYM>/<type>.jsonl(.zst)
```

All three stages share the existing building blocks:

- `streams::dynamic_market_stream` — subscribe/unsubscribe at runtime, needed
  for Stage 3 as the active set mutates.
- `streams::stream_recorder` + `sinks::file_sink` — async file writing on the
  recorder's own `io_thread`, with backpressure via `threadsafe_stream_buffer`.
- `client` services for the REST periodic syncs.

### Stage 1 — Screener (market-wide, always on)

Purpose: cheap market-wide observation to feed the selector, plus a permanent
low-cost archive of the "all" streams for post-hoc symbol selection debugging and analysis.

Subscribes to the **all-market** WebSocket streams (connection per stream):

| Stream            | Used for                                  |
|-------------------|-------------------------------------------|
| `!bookTicker`     | per-symbol spread, best-of-book updates   |
| `!markPrice@arr@1s` | funding, mark, index, premium regime    |
| `!ticker@arr`     | 24h rolling volume, price change, trades  |
| `!miniTicker@arr` | (optional — redundant with `!ticker@arr`) |

Outputs:

1. **Raw archive** — one JSONL file per stream, written through `file_sink`.
   Rotation policy:
   - size-based (e.g. 512 MiB) **or** time-based (hourly), whichever first;
   - closed segments are compressed with `zstd` off the hot path (spawn a
     detached task that calls `zstd -q --rm <file>`);
   - filename pattern: `screener/<stream>/<yyyymmddTHHMMSSZ>.jsonl[.zst]`.
2. **Live aggregates** — rolling, in-memory per-symbol stats consumed by
   Stage 2 (no persistence):
   - 1m/5m/1h/4h/1d bucketed volume, trade count, range, NATR;
   - latest best bid/ask, spread, mark, funding, next-funding-time.

Rotation/compression is implemented as a thin wrapper around `file_sink`
(`rotating_file_sink`) that atomically swaps the underlying `stream_file`
when the threshold is hit and enqueues the just-closed path onto a compressor
task running on the same `io_thread`.

### Stage 2 — Instrument selector

Purpose: pick N "interesting" symbols with a *stable* set — no flapping —
and emit add/remove events to Stage 3.

Inputs: Stage 1 rolling aggregates (pulled on a 1s timer).

Scoring (main TFs 1m/5m/1h/4h/1d):
- volume z-score
- trade count z-score
- normalized range (separately tracked for gainers vs losers)
- NATR

Each factor contributes weighted points; final score is a weighted sum.
Weights live in a TOML/JSON config file so the selector can be re-tuned
without recompiling.

Stability rules (all enforced in one `selector` coroutine):
- **Hysteresis**: two thresholds — `add_score` > `remove_score`. A symbol
  only enters when score ≥ `add_score`; it only exits when score falls below
  `remove_score` for at least `cooloff_duration` (e.g. 10 min).
- **Min duration**: once admitted, a symbol stays at least `min_hold`
  (e.g. 30 min) regardless of score, unless mandatory cap is exceeded.
- **Bounds**: `min_active ≤ |active| ≤ max_active`.
- **Mandatory set**: always includes BTCUSDT, ETHUSDT (configurable), and
  they are never subject to remove rules.
- **Ranking**: within bounds, keep the top-N by score.

Output: `selector/signals.jsonl` — one JSONL record per add/remove decision
with the symbol, direction, score, the reason(s) fired, and the full score
breakdown. This file is itself a primitive log and feeds research into the
selector's behavior.

Emits add/remove to Stage 3 through a `cobalt::channel<selector_event>`.

### Stage 3 — Detail monitor

Purpose: record the full institutional-grade per-symbol feed, but only for
the current active set.

For each symbol in the active set, subscribes via the **single shared**
`dynamic_market_stream` to the **Tier 0** + (optionally) **Tier 1** streams
from `async-recorder.md`:

| Wire name                 | Tier | Notes                                   |
|---------------------------|------|-----------------------------------------|
| `<sym>@aggTrade`          | 0    | tape, delta, CDV                        |
| `<sym>@bookTicker`        | 0    | spread / top-of-book                    |
| `<sym>@markPrice@1s`      | 0    | funding / mark / index / premium        |
| `<sym>@forceOrder`        | 0    | liquidations                            |
| `<sym>@depth@100ms`       | 1    | L2 diff book (gated by `--with-depth`)  |
| `<sym>@kline_1m`          | 2    | baseline klines                         |

When a symbol is added, the recorder:
1. Sends a SUBSCRIBE for its enabled streams.
2. Opens per-stream sinks under `detail/<SYM>/<stream>/<timestamp>.jsonl`.
3. If `@depth@100ms` is enabled, kicks a REST `/fapi/v1/depth?limit=1000`
   snapshot and writes it to `detail/<SYM>/depth_snapshot/<ts>.json` *before*
   the first diff is recorded, so offline reconstruction has a well-defined
   starting point. Subsequent snapshots are taken on a timer (e.g. every
   10 min) as resync anchors.

When a symbol is removed:
1. Sends UNSUBSCRIBE.
2. Flushes and closes its per-stream sinks (which triggers compression of the
   final segments).
3. The symbol directory is left in place; it becomes immutable history.

A per-symbol `monitor_state` holds the open sinks and the last-known snapshot
deadline. The outer coroutine `std::visit`s selector events against a
`std::map<symbol_t, monitor_state>`.

### REST periodic sync (cross-stage)

A fourth, tiny coroutine polls REST endpoints at fixed intervals and writes
their responses as primitive JSONL using the same rotating sink. These are
market-wide except where noted:

| Endpoint                                | Period | Scope              |
|-----------------------------------------|--------|--------------------|
| `/fapi/v1/depth?limit=1000`             | 10m    | active set only    |
| `/fapi/v1/fundingRate`                  | 1h     | all symbols        |
| `/futures/data/openInterestHist`        | 5m     | active set only    |
| `/futures/data/globalLongShortAccountRatio` | 15m | active set only    |
| `/fapi/v1/klines?interval=1m`           | 1m     | active set only    |

All REST calls reuse the existing `client` services and their `result<T>`
returns. A failed call logs and retries on the next tick — never blocks the
main loop.

## 3. Storage layout

```
<root>/
  screener/
    bookTicker/<ts>.jsonl.zst
    markPriceArr/<ts>.jsonl.zst
    tickerArr/<ts>.jsonl.zst
  selector/
    signals.jsonl
  detail/
    BTCUSDT/
      aggTrade/<ts>.jsonl.zst
      bookTicker/<ts>.jsonl.zst
      markPrice/<ts>.jsonl.zst
      forceOrder/<ts>.jsonl.zst
      depth/<ts>.jsonl.zst
      depth_snapshot/<ts>.json
      kline_1m/<ts>.jsonl.zst
    ETHUSDT/
      ...
  rest/
    fundingRate/<ts>.jsonl
    openInterestHist/<ts>.jsonl
    longShortRatio/<ts>.jsonl
```

Each JSONL line is the **raw frame** as received, unparsed — this is the
"store primitives only" rule from the brief. Event classification
(`TRADE/QUOTE/MARK/LIQ/DEPTH/STATE`) is implicit in the file path; no
indicator, no derived field is ever persisted to disk.

## 4. Concurrency model

**Single thread, single executor.** The whole recorder is a `cobalt::main`
on one OS thread. Everything — WebSocket reads, REST calls, stream parsing,
aggregates, selector, sinks, file I/O, timers — runs as cooperating
coroutines on that thread. No `io_thread`, no cross-thread buffers, no
locks on the aggregates map. This is the cheapest model that still scales
to full-market tier-0 recording because:

- every sink is bound by disk / io_uring, not CPU;
- the producer (websocket framing) and the consumer (file write) share a
  page cache and a context, so there is no thread hand-off cost;
- the selector is 1 Hz and scans O(number of symbols) — trivially cheap.

**Recorder type.** Uses `basic_async_stream_recorder<Sink>` (single-
executor, `stream_buffer` backend). `basic_threaded_stream_recorder` is
*not* used anywhere in this pipeline — we do not want an extra thread.
All producers push via `co_await buf.async_push(frame)`, which provides
natural backpressure: if a sink can't keep up, the websocket read loop
suspends, the TCP window closes, and Binance slows the stream.

### Coroutines on the single executor

- `screener_run()` — owns the all-* dynamic stream, its
  `async_stream_recorder`, the rotating sinks, and the rolling aggregates
  map.
- `selector_run()` — 1 Hz timer, reads aggregates, writes `signals.jsonl`,
  emits add/remove through an in-process `cobalt::channel`.
- `detail_run()` — owns the per-symbol dynamic stream and its
  `async_stream_recorder`; consumes the channel and mutates
  subscriptions.
- `rest_sync_run()` — staggered timers, one REST call per tick, writes
  into a dedicated `async_stream_recorder` with one buffer per endpoint.
- `stats_run()` — periodic buffer/sink stats logger (see §4.2).

### 4.1 Buffered recording for every stream

Every recorded stream goes through a `stream_buffer<string>` attached to
an `async_stream_recorder`, never directly into a sink. The buffer serves
three purposes:

1. **Decoupling** — the websocket read loop returns to reading the next
   frame as soon as the push completes, which under normal load is
   immediate (buffer not full); the drain coroutine handles the sink
   I/O asynchronously.
2. **Absorbing bursts** — Binance depth streams can burst well above
   average rate; a per-stream buffer smooths those bursts into steady
   disk writes.
3. **Backpressure** — when the sink falls behind and the buffer fills,
   `async_push` suspends; the websocket read loop stops consuming from
   the socket, which is the correct failure mode (drop nothing, slow
   down the source).

Buffer sizing (defaults, overridable per stream in config):

| Stream class                        | Size  | Rationale                   |
|-------------------------------------|-------|-----------------------------|
| `!bookTicker`                       | 16384 | high rate, market-wide      |
| `!markPrice@arr@1s`, `!ticker@arr`  |  4096 | 1 Hz array events           |
| per-symbol `aggTrade`, `bookTicker` |  4096 | moderate                    |
| per-symbol `depth@100ms`            | 32768 | bursty, large payload       |
| per-symbol `forceOrder`, `kline_1m` |  1024 | low rate                    |
| REST endpoints                      |   256 | periodic, low volume        |

### 4.2 Buffer / sink observability

A `stats_run()` coroutine wakes on a 10 s timer and logs a single JSON
line per tick to `stats.jsonl` (and, at a lower frequency, to spdlog at
info) describing each buffer's state. Per buffer it records:

- name (stream key or endpoint),
- current occupancy and capacity,
- a high-water mark since last tick (reset after logging),
- total frames pushed and total frames drained since start,
- total push-suspensions since start (number of times `async_push`
  had to wait — a direct backpressure signal),
- current rotating sink filename and bytes written.

To support this, `stream_buffer<T>` gets two cheap counters
(`pushed_total_`, `drained_total_`, `push_suspends_total_`) and an
`occupancy()`/`capacity()`/`high_water()` accessor set. Reads are
unsynchronised because everything lives on one executor.

### 4.3 fsync on rotation

`asio::stream_file::close()` only issues the `close(2)` syscall — it
does **not** imply `fsync(2)`, and on Linux a closed file may still have
dirty pages in the page cache. That means a crash immediately after
rotation can leave the just-rotated segment corrupt, and compressing it
afterwards would bake the corruption into the `.zst`.

Rotation sequence in `rotating_file_sink`:

1. Stop writing new frames to the current `stream_file` (atomically
   swap in a new one so the next push goes to the new segment).
2. On the old handle: `co_await async_write` any queued bytes, then
   call `fsync(fd)` — via a small helper that posts a blocking `::fsync`
   onto a short-lived worker. (There is no async `fsync` in Boost.Asio
   for `stream_file`; the syscall is cheap enough at rotation cadence
   that doing it inline is acceptable, but posting it keeps the main
   thread responsive if the underlying disk is slow. The compression
   step already happens via `posix_spawn`, so reusing the same
   out-of-thread pattern is natural.)
3. `close()` the old `stream_file`.
4. Enqueue the closed path for compression: `posix_spawn zstd --rm
   <path>`. `zstd --rm` only deletes the input after a successful write;
   the `.zst` atomically replaces the segment.
5. Log the rotation to `stats.jsonl`.

Only step 2 is new relative to today's `file_sink`; the rest mirrors
what the design already had.

### 4.4 Shutdown

On SIGINT/SIGTERM, a single `cobalt::cancellation_signal` is raced
against every read loop. Each stage, on cancellation, closes its
recorder (`recorder.close()`), awaits the drain via `co_await
recorder.run()`'s completion, forces an fsync+rotation on every open
segment, and lets its `stats.jsonl` tick fire one last time to capture
final counters. `cobalt::main` returns 0 only after all four stages
have drained.

## 5. Implementation plan

Build incrementally — each step is independently runnable and testable.

### Step 1 — scaffolding

- Create `examples/binapi2/fapi/async-recorder/` with `CMakeLists.txt`,
  `main.cpp`, `config.hpp` (CLI arg parsing via the same helpers used by
  `async-demo-cli`).
- `config` struct: root dir, rotation size/time, selector weights file path,
  enable flags (`--with-depth`, `--with-klines`, ...).
- Wire into `examples/binapi2/fapi/CMakeLists.txt`.

### Step 2 — rotating file sink

- New header `include/binapi2/fapi/streams/sinks/rotating_file_sink.hpp`
  wrapping `file_sink`:
  - size and time limits;
  - `close_current_and_rotate()` coroutine;
  - detached `zstd` compression task (call `posix_spawn` `zstd --rm`, log on
    failure — never abort the recorder).
- Unit test: write N MiB, assert file count and `.zst` appearance.

### Step 3 — screener stage

- New file `async-recorder/screener.{hpp,cpp}`.
- Subscribes `!bookTicker`, `!markPrice@arr@1s`, `!ticker@arr` via one
  `dynamic_market_stream`.
- Writes each event's raw frame to the matching `rotating_file_sink`.
- In parallel, updates `aggregates_t` (rolling windows per symbol for
  1m/5m/1h/4h/1d — implement as a ring of bucketed counters indexed by
  `symbol_t`, protected by a `std::shared_mutex` since the selector reads
  from a separate coroutine but the same thread — so the mutex is only
  needed if the selector ever moves off the io_thread; start on-thread,
  drop the mutex).
- Manual smoke test via testnet: `scripts/testnet/` style launcher.

### Step 4 — selector stage

- `async-recorder/selector.{hpp,cpp}`.
- `selector_config` (weights, thresholds, bounds, mandatory, hold/cooloff).
- `selector_run()` timer loop: compute score per symbol, apply hysteresis +
  min-hold + bounds + mandatory, emit diffs.
- Signals writer: plain append-only JSONL (no rotation needed — tiny file).
- Unit tests with synthetic aggregates: verify no flapping under noisy
  scores, mandatory symbols always present, min/max bounds obeyed.

### Step 5 — detail monitor stage

- `async-recorder/detail.{hpp,cpp}`.
- Second `dynamic_market_stream` dedicated to per-symbol Tier-0 streams.
- `monitor_state` per symbol: map of stream-name → rotating_file_sink.
- Handles add/remove events from the selector channel; SUBSCRIBE /
  UNSUBSCRIBE calls; sink lifecycle.
- Integration test against the Postman mock (`compose/postman-mock/`):
  feed a scripted add, expect a SUBSCRIBE frame and a new directory on
  disk; then a remove, expect UNSUBSCRIBE and closed sinks.

### Step 6 — REST periodic sync

- `async-recorder/rest_sync.{hpp,cpp}`.
- Independent staggered timers per endpoint.
- Reuses existing `market_data`, `trade` service calls; writes raw JSON.
- Depth snapshot integration: invoked *on demand* by the detail stage when
  a symbol is added, in addition to the periodic timer.

### Step 7 — end-to-end

- `main.cpp` composes all four coroutines under a single `cobalt::main`,
  installs signal handling for graceful shutdown, and races a
  `cancellation_signal` across them.
- Long-running smoke run on testnet: 30 min, check on-disk layout,
  rotation, compression, signals history, active-set stability.

### Step 8 — docs

- Update `docs/binapi2/guide/02-market-data.md` with a pointer to the
  recorder example.
- Add a short `examples/binapi2/fapi/async-recorder/README.md` describing
  CLI flags and the output directory layout.

## 6. Resolved decisions

1. **Trades stream.** Use `@aggTrade` only. USD-M Futures does not expose
   a per-trade (non-aggregated) stream — `@aggTrade` is the authoritative
   tape for this venue, so there is nothing to compare against. No flag,
   no alternative.
2. **Depth stream selection.**
   - **Default:** `<sym>@depth20@100ms` — ready-made top-20 bid+ask
     snapshots, cheap on the wire and on disk, sufficient for
     imbalance / microprice / queue research.
   - **Cutoff option:** `--depth-levels {5,10,20}` to pick the partial
     book stream. Binance does not offer 50/100 as streams, so the
     option is bounded to the set it actually supports.
   - **Full research mode:** `--full-depth` switches to
     `<sym>@depth@100ms` (diff stream) + local order book
     reconstruction. Price-band filtering (e.g. ±2%) is **not** a native
     Binance stream; it only exists as a post-reconstruction filter on
     top of full diff depth. Implement this later only if a concrete
     research need appears — v1 records the raw diff stream and leaves
     band filtering to the replay/analysis side.
3. **Aggregates durability.** In-memory only. No disk checkpoint, no
   recovery path. A restart loses at most ~1 day of warm-up; the short
   TFs are rebuilt from the live stream within minutes.
4. **Selector config format.** YAML via Glaze (`glz::read_yaml` /
   `glz::write_yaml`). Glaze is already a dependency and supports YAML
   directly. Single config file
   `examples/binapi2/fapi/async-recorder/selector.yaml` with the
   selector weights, thresholds, hold/cooloff durations, bounds, and
   mandatory set.
5. **Segment retention.** K = 0 — compress every segment immediately on
   rotation, keep no uncompressed tail. Live data is always readable
   from the ongoing segment; hot replay use cases, if they appear, can
   stream-decompress on demand.
6. **Stats destination.** File + spdlog only. `stats.jsonl` is the
   primary sink (one JSON line per 10 s tick); the same record is also
   emitted at spdlog `info` level. No HTTP endpoint, no Prometheus
   exporter. Grafana / external dashboards, if they appear later, can
   tail `stats.jsonl`.
