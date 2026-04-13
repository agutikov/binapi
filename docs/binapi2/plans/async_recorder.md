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

- One `cobalt::thread` / `io_thread` for all WebSocket traffic + file sinks,
  matching the existing `stream_recorder` model.
- Coroutines running on that executor:
  - `screener_run()` — owns the all-* dynamic stream + rotating sinks + the
    aggregates map.
  - `selector_run()` — 1s timer, reads aggregates, writes signals, emits
    add/remove through a channel.
  - `detail_run()` — owns the per-symbol dynamic stream + per-symbol sinks;
    consumes the channel.
  - `rest_sync_run()` — staggered timers, one REST service call each tick.
- Clean shutdown via a single `cobalt::cancellation_signal` racing the main
  read loops (same pattern already used in `stream_recorder`).

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

## 6. Open questions

1. `aggTrade` vs raw `trade` — brief recommends aggTrade; confirm aggTrade
   is sufficient before committing to it permanently (design keeps raw
   trades as an easy add since the sink layer is stream-agnostic).
2. Depth compression: 20–80 GB/day for BTC `@depth@100ms` is expensive even
   after zstd. Consider a downsampling sink that keeps only top-20 levels
   for the default build and gate full raw depth behind `--full-depth`.
3. Where does the aggregates map live long-term — purely in memory, or
   periodically checkpointed for crash recovery? Start in memory; add
   checkpointing only if restart frequency becomes painful.
4. Selector config format — TOML (new dep) vs JSON via Glaze (already in
   the tree). Default to Glaze/JSON.
