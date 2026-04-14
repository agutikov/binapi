# async-recorder

Institutional-grade market recorder for Binance USDⓈ-M Futures, built on
`binapi2::fapi`. Runs four coroutine stages on a single `cobalt::main`
executor and produces append-only, primitive-only event logs suitable
for deterministic replay, fill simulation, factor research, and
execution research — while keeping storage bounded by only recording
*detailed* data for a dynamically selected, stable set of active
instruments.

**Non-goals**: indicators, derived features, strategy logic. Only raw
primitives are persisted; everything else is reconstructible offline.

## 1. Architecture

```
 ┌─────────────┐   active set    ┌──────────────────┐  add/remove  ┌────────────────────┐
 │ 1. Screener │ ──────────────▶ │ 2. Selector      │ ────────────▶│ 3. Detail monitor  │
 │ (all-*)     │                 │ (scoring + rules)│              │ (per-symbol feeds) │
 └──────┬──────┘                 └────────┬─────────┘              └─────────┬──────────┘
        │ raw jsonl                       │ signals.jsonl                    │ tier-0..2 jsonl
        ▼                                 ▼                                  ▼
    screener/*.jsonl(.zst)         selector/signals.jsonl        detail/<SYM>/<type>.jsonl(.zst)

                                                                  ┌────────────────────┐
                                                                  │ 4. REST periodic   │
                                                                  │    sync            │
                                                                  └─────────┬──────────┘
                                                                            ▼
                                                                    rest/*.jsonl

                                     ┌─────────────────┐
                                     │ 5. status       │ — one spdlog line per tick,
                                     │    reporter     │   collects every stage's
                                     └─────────────────┘   counters
```

### Stage 1 — Screener (market-wide, always on)

Subscribes to the three all-market WS streams and archives every raw
frame to its own rotating, fsync'd, zstd-compressed JSONL under
`screener/<stream>/`. In parallel, updates a per-symbol **aggregates
map** consumed by the selector.

### Stage 2 — Selector (scoring + stability)

Periodically (`--stats-seconds`) scores every symbol in the aggregates
map, applies hysteresis / hold / cooloff / bounds / mandatory rules,
and emits add/remove signals to `selector/signals.jsonl`. Rules are
designed so the active set does not flap under noise.

### Stage 3 — Detail monitor (per-symbol deep record)

Tracks the selector's active set; on admission it `SUBSCRIBE`s the
symbol's Tier-0 feeds on a shared `dynamic_market_stream`, routes raw
frames to per-symbol rotating sinks under `detail/<SYM>/<stream>/`,
and `UNSUBSCRIBE`s + closes those sinks on eviction.

### Stage 4 — REST periodic sync

Runs its own REST client and pulls endpoints that aren't in WS streams
at their own cadences. Market-wide endpoints cover everything; scoped
endpoints loop over the selector's active set each tick.

### Stage 5 — Status reporter

App-wide periodic logger. Each of the other four stages registers a
state source; the reporter emits **one** `spdlog::info` line per tick
containing the full cross-stage state.

## 2. Data feeds — selection and prioritization

The feed universe is ranked into four tiers by the ratio of
**usefulness × alpha density / storage cost**. Tiers are recorded
*conditional on which instrument the data covers*: market-wide feeds
are cheap enough to stream every symbol, per-symbol feeds are expensive
and are only subscribed for the selector's active set.

### Tier 0 — absolutely mandatory (per active symbol)

Highest value per stored byte. Drives tape/queue/liquidation research.

| Feed               | Wire                        | Value for                                                |
|--------------------|-----------------------------|----------------------------------------------------------|
| `aggTrade`         | `<sym>@aggTrade`            | Tape replay, delta, CDV, volume impulses, burst detection |
| `bookTicker`       | `<sym>@bookTicker`          | Spread, micro pressure, queue proxy, entry modeling      |
| `markPrice @1s`    | `<sym>@markPrice@1s`        | Funding / mark / index / premium regime                  |
| `liquidation`      | `<sym>@forceOrder`          | Squeeze detection, liquidation cascades                  |

Futures does not expose a per-trade (non-aggregated) stream — `aggTrade`
**is** the tape for this venue, no alternative.

### Tier 1 — strongly recommended (per active symbol, opt-in)

The execution layer. Expensive on disk (20–80 GB/day raw for BTC at
`@depth@100ms`), so opt-in behind `--with-depth`.

| Feed                        | Wire                                | Default behaviour                                          |
|-----------------------------|-------------------------------------|------------------------------------------------------------|
| **Partial L2 snapshots**    | `<sym>@depth{5,10,20}@100ms`        | Default when `--with-depth` is passed; `--depth-levels` selects 5/10/20 |
| **Full L2 diff**            | `<sym>@depth@100ms`                 | Only with `--with-depth --full-depth`; requires local book reconstruction |
| **Depth snapshot (REST)**   | `/fapi/v1/depth?limit=1000`         | Fired on symbol admission + periodic re-anchor (anchor for the diff stream) |

Price-band depth filtering (e.g. ±2%) is **not** a native Binance
stream; it's only achievable as a post-reconstruction filter on top of
the full diff stream. Band filtering is an analysis-side concern, not
a recording concern.

### Tier 2 — valuable historical enrichment

| Feed                         | Surface | Cadence  | Scope          | Output                    |
|------------------------------|---------|----------|----------------|---------------------------|
| 1m klines (per symbol)       | REST    | 1 min    | Active set     | `rest/klines_1m/<SYM>.jsonl` |
| Funding rate history         | REST    | 1 hour   | Market-wide    | `rest/fundingRate/fundingRate.jsonl` |
| Open interest history        | REST    | 5 min    | Active set     | `rest/openInterestHist/<SYM>.jsonl` |

Raw (non-aggregated) trades are *intentionally skipped*: alpha gain over
`aggTrade` is small and storage cost is large.

### Tier 3 — advanced enrichment

| Feed                          | Surface | Cadence | Scope       |
|-------------------------------|---------|---------|-------------|
| Global long/short ratio       | REST    | 15 min  | Active set  |
| Taker buy/sell volume         | REST    | 15 min  | Active set  |
| Premium / index price history | REST    | 15 min  | Active set  |

Moderate alpha, mostly useful as regime / positioning filters.

### Tier 4 — optional research only

`miniTicker`, contract info, insurance fund, ADL stats: redundant with
better-ranked primitives in most workloads. Not recorded by default.

### Institutional rule

> **Never store indicators. Store primitives only. Indicators are
> disposable; raw events are permanent.**

Event classification (`TRADE / QUOTE / MARK / LIQ / DEPTH / STATE`) is
implicit in the on-disk path — the file name *is* the metadata.

## 3. Selector — scoring and stability

Inputs: the screener's per-symbol aggregates map (rolling TF windows
for volume, trades, range, NATR on 1m / 5m / 1h / 4h / 1d).

**Scoring** — each factor contributes weighted points, final score is
a weighted sum. Weights are loaded at startup from a YAML file (see
`selector.yaml`) so the selector can be re-tuned without recompiling.

**Stability rules** (`selector::tick`):

1. **Hysteresis** — `add_score` > `remove_score`. A symbol is admitted
   only when its score ≥ `add_score`; once in, it's evicted only when
   the score falls below `remove_score` for at least `cooloff_seconds`.
2. **Min hold** — once admitted, a symbol stays at least
   `min_hold_seconds` regardless of score.
3. **Bounds** — `min_active ≤ |active| ≤ max_active`. Below
   `min_active`, the top-scored non-active symbols are promoted even
   below `add_score` (keeps the detail monitor fed in quiet markets).
4. **Mandatory set** — listed symbols (e.g. `BTCUSDT`, `ETHUSDT`) are
   pre-admitted and never subject to eviction.
5. **Ranking** — within bounds, keep the top-N by score.

### `selector.yaml` template

```yaml
w_volume: 1.0
w_trades: 1.0
w_range: 1.0
w_natr: 1.5

add_score: 2.0
remove_score: 1.0

min_hold_seconds: 1800      # 30 min
cooloff_seconds: 600        # 10 min

min_active: 5
max_active: 25

mandatory:
  - BTCUSDT
  - ETHUSDT
```

**Unit tests** (`async_recorder_selector_test`) cover: mandatory
pre-admission, `max_active` cap, `min_active` floor, hysteresis
dead-band (no flapping), and the `min_hold + cooloff` eviction timing.

## 4. Concurrency and I/O model

**Single thread, single executor.** The whole recorder is a
`cobalt::main` on one OS thread. WebSocket reads, REST calls, stream
parsing, aggregates, selector, sinks, file I/O, and timers all run as
cooperating coroutines on that executor. No `io_thread`, no
cross-thread buffers, no locks on the aggregates map.

This works because:

- every sink is bound by disk / io_uring, not CPU;
- the producer (WebSocket framing) and the consumer (file write) share
  a page cache and a context, so there is no thread hand-off cost;
- the selector is low-frequency and scans O(number of symbols) —
  trivially cheap.

### Recorder type

Uses `basic_async_stream_recorder<Sink>` — single-executor,
`stream_buffer` backend. `basic_threaded_stream_recorder` is explicitly
**not** used anywhere in this pipeline: we do not want an extra thread.
All producers push via `co_await buf.async_push(frame)`, which
provides natural backpressure — if a sink can't keep up, the WebSocket
read loop suspends, the TCP window closes, and Binance slows the
stream.

### Buffered recording

Every recorded feed goes through a `stream_buffer<string>` attached to
the async recorder, never directly into a sink. The buffer serves
three purposes:

1. **Decoupling** — the WS read loop returns to reading the next frame
   as soon as the push completes.
2. **Absorbing bursts** — depth streams can burst well above average
   rate; a per-stream buffer smooths those bursts into steady writes.
3. **Backpressure** — when the sink falls behind and the buffer fills,
   `async_push` suspends (the correct failure mode: drop nothing, slow
   down the source).

### Rotating file sink: rotation, fsync, zstd

All screener and detail sinks use `rotating_file_sink`:

1. Open a new segment `<basename>.<ts>.<nnnn>.jsonl` in the target dir.
2. When either `--rotate-size` bytes or `--rotate-seconds` age is
   reached, **fsync(2)** the fd, **close(2)** it, then
   `posix_spawnp("zstd", "-q", "--rm", "<path>")` so compression runs
   out-of-process. `--rm` replaces the `.jsonl` atomically with
   `.jsonl.zst`.
3. On destructor (shutdown), the current segment is fsync'd, closed,
   and a final compressor is spawned; the destructor then
   `waitpid(..., 0)`s every outstanding child so compression has
   finished before the process exits.

The explicit `fsync` before `close` matters: `asio::stream_file::close()`
is only `close(2)`, which on Linux leaves dirty pages in the page
cache. A crash immediately after rotation would bake corruption into
the `.zst`.

### Observability

The `stream_buffer<T>` carries cheap counters that every sink and
producer updates on the same executor:

- `pushed_total`, `drained_total`, `push_suspends_total`
- `occupancy()`, `capacity()`, `high_water()`

The status reporter ticks on `--stats-seconds` (default 10 s) and
emits **one** `spdlog::info` line containing a snapshot from every
registered source. Example:

```
status: screener{bt[push=1524 drn=1524 occ=0/16384 hw=32 susp=0]
                 mp[push=27   drn=27   occ=0/16384 hw=1  susp=0]
                 tk[push=13   drn=13   occ=0/16384 hw=1  susp=0]}
      | selector{active=25/25 signals=23}
      | rest{fund=1/0 kln=5/0 oi=0/5 lsr=0/5 active=25}
      | detail{subs=25/25 buf=[push=343 drn=343 occ=0 hw=23]}
```

Any non-zero `susp` means the corresponding sink has backpressured the
WebSocket read loop — the primary debugging signal.

### Shutdown

SIGINT / SIGTERM triggers a graceful unwind: every stage's periodic
timer returns with a cancellation, the selector/detail/rest_sync loops
exit, the screener's generators close, every rotating sink `fsync`s
and compresses its final segment, and `cobalt::main` returns 0.

## 5. Storage layout

```
<root>/
  screener/
    bookTicker/        bookTicker.<ts>.<nnnn>.jsonl[.zst]
    markPriceArr/      markPriceArr.<ts>.<nnnn>.jsonl[.zst]
    tickerArr/         tickerArr.<ts>.<nnnn>.jsonl[.zst]
  selector/
    signals.jsonl                  (append-only add/remove log)
  detail/
    BTCUSDT/
      aggTrade/        aggTrade.<ts>.<nnnn>.jsonl[.zst]
      bookTicker/      bookTicker.<ts>.<nnnn>.jsonl[.zst]
      markPrice/       markPrice.<ts>.<nnnn>.jsonl[.zst]
      forceOrder/      forceOrder.<ts>.<nnnn>.jsonl[.zst]
      depth_snapshot/  startup.<ts>.json        (on admission)
                       resnap.<ts>.json         (periodic, full-depth mode)
      depth20/         depth20.<ts>.<nnnn>.jsonl[.zst]         (--with-depth --depth-levels 20)
      depth_diff/      depth_diff.<ts>.<nnnn>.jsonl[.zst]      (--with-depth --full-depth)
    ETHUSDT/
      …
    …one directory per currently-active symbol…
  rest/
    fundingRate/       fundingRate.jsonl           (one line per fetch, market-wide)
    klines_1m/         <SYMBOL>.jsonl              (one line per fetch, active set)
    openInterestHist/  <SYMBOL>.jsonl              (one line per fetch, active set)
    longShortRatio/    <SYMBOL>.jsonl              (one line per fetch, active set)
```

Every JSONL line is the **raw frame** as received, unparsed. REST
fetches are wrapped as `{"t":"<ISO8601>","d":<raw response>}` so the
ingest time is captured inline per line.

## 6. CLI flags

| Flag                          | Default    | Purpose                                                           |
|-------------------------------|------------|-------------------------------------------------------------------|
| `--root <dir>`                | `./data`   | Output root directory.                                            |
| `--selector <file.yaml>`      | built-in   | Selector config YAML (weights, thresholds, hold, bounds).         |
| `--rotate-size <bytes>`       | 100 MiB    | Rotation size for the screener / detail JSONL sinks.              |
| `--rotate-seconds <s>`        | 900        | Rotation time for the same sinks.                                 |
| `--depth-levels {5,10,20}`    | 20         | Partial-book depth levels (detail monitor; see F3).               |
| `--full-depth`                | off        | Use full diff depth instead of partial (see F4).                  |
| `--with-depth`                | off        | Record depth at all (gates F3 / F4).                              |
| `--no-klines`                 | on         | Disable per-symbol 1m kline recording.                            |
| `--stats-seconds <s>`         | 10         | Status reporter / selector / detail / rest_sync tick interval.    |
| `--loglevel <lvl>`            | `trace`    | spdlog level: trace/debug/info/warn/error/critical/off.           |
| `--logfile <path>`            | —          | Also write logs to this file (truncated on start).                |
| `--debug-stream <name>`       | (off)      | Run single-stream debug screener against `bookTicker` / `markPriceArr` / `tickerArr`. |
| `--live`                      | off (testnet) | Use production endpoints.                                      |
| `--testnet`                   | on         | Use testnet endpoints (default).                                  |
| `--print-config`              | —          | Parse flags + YAML, dump resolved config, exit.                   |
| `-h`, `--help`                | —          | Print help and exit.                                              |

## 7. Building and running

The example is built by the project's top-level CMake.

```sh
./build.sh
./_build/examples/binapi2/fapi/async-recorder/binapi2-fapi-async-recorder \
    --root ./data --stats-seconds 5 \
    --selector examples/binapi2/fapi/async-recorder/selector.yaml
```

For a **production run with every supported feed enabled** (live
endpoints, Tier-0 + full diff depth + periodic REST snapshots +
REST sync), use the wrapper script at the repo root:

```sh
./run_recorder.sh
```

It creates a timestamped data directory `./data/run-<UTCts>/`, drops
a `recorder.log` alongside it, sets `--live --with-depth --full-depth
--depth-resnap-seconds 600`, and `exec`s the binary so Ctrl-C stops
it cleanly. Anything you pass after the script name is forwarded to
the binary, e.g.:

```sh
./run_recorder.sh --depth-levels 10 --stats-seconds 5
```

Override the root dir with `ASYNC_RECORDER_ROOT=/mnt/fast ./run_recorder.sh`.

**Storage warning**: `--full-depth` can emit 20–80 GB/day per major
symbol. Drop `--full-depth` (partial mode writes ~2 orders of magnitude
less) or drop `--with-depth` entirely for a lighter run.

Ctrl-C or SIGTERM triggers graceful shutdown.

Unit tests for the selector and TF windows are registered under ctest:

```sh
./run_tests.sh -R 'Selector|TfWindow'
```

## 8. Implementation status

| Area                               | Status | Notes                                                                  |
|------------------------------------|--------|------------------------------------------------------------------------|
| **Scaffolding & config**           | ✅     | `config.{hpp,cpp}`, CLI flags, YAML loader (Glaze), `main.cpp`        |
| **`rotating_file_sink`**           | ✅     | size+time rotation, fsync, `posix_spawn` zstd; 4 unit tests           |
| **`stream_buffer` counters**       | ✅     | push/drain/occupancy/high-water/suspends; used by status reporter     |
| **`status_reporter`**              | ✅     | single periodic logger, multi-source aggregation                      |
| **Screener — all-market streams**  | ✅     | `!bookTicker` + `!markPrice@arr@1s` + `!ticker@arr`, rotating JSONL   |
| **Screener — aggregates**          | ✅     | `events_total` (bookTicker) + 1m/5m/1h volume+trades windows (`!ticker@arr`) |
| **Selector — logic**               | ✅     | Scoring, hysteresis, min-hold, cooloff, bounds, mandatory             |
| **Selector — unit tests**          | ✅     | 6 gtests registered under ctest                                       |
| **Selector — `signals.jsonl`**     | ✅     | Append-only ISO-8601 JSONL                                            |
| **Detail — `aggTrade`**            | ✅     | Per-symbol WS, rotating sink                                          |
| **Detail — `bookTicker`**          | ✅     | Per-symbol WS, rotating sink                                          |
| **Detail — `markPrice@1s`**        | ✅     | 1 Hz per-symbol via `mark_price_subscription{every_1s=true}`          |
| **Detail — `forceOrder`**          | ✅     | Per-symbol liquidation stream (often empty on testnet)                |
| **Detail — REST depth snapshot**   | ✅     | Fires on each admission → `depth_snapshot/startup.<ts>.json`          |
| **Detail — partial depth WS**      | ✅     | `--with-depth --depth-levels {5,10,20}` → `detail/<SYM>/depth<N>/`    |
| **Detail — full diff depth + book**| ✅ (partial) | `--full-depth` subscribes `@depth@100ms` + periodic REST resnaps; in-memory `local_order_book` deferred |
| **REST — fundingRate**             | ✅     | Hourly, market-wide                                                   |
| **REST — klines_1m**               | ✅     | 1 min, per-active-symbol                                              |
| **REST — openInterestHist**        | ✅     | 5 min, per-active-symbol (testnet rejects — works on `--live`)        |
| **REST — longShortRatio**          | ✅     | 15 min, per-active-symbol (testnet rejects — works on `--live`)       |
| **REST — taker buy/sell, index**   | ❌     | Not implemented (Tier 3 enrichment)                                   |
| **Observability — one-line status**| ✅     | `screener | selector | rest | detail` fields                         |
| **Observability — `stats.jsonl`**  | ❌     | Status only in spdlog; no separate file sink                          |
| **Concurrency**                    | ✅     | Single `cobalt::main`, nested 2-task joins, graceful SIGINT shutdown  |
| **Docs — design plan**             | ✅     | `docs/binapi2/plans/async_recorder.md`                                |
| **Integration test (postman-mock)**| ❌     | Testnet smoke-tested end-to-end instead                               |

### Follow-up history (F1–F5)

After the initial 8-step build, the recorder only had **2 of 4**
Tier-0 WS streams per admitted symbol (`aggTrade`, `bookTicker`) and a
placeholder scoring function that used `!bookTicker` event count as a
proxy for volume. Five follow-up steps closed the detail-recording
gap and the selector-quality gap. All five have landed; below is a
condensed record of what each one did.

| Step | Scope                                               | Status                |
|------|-----------------------------------------------------|-----------------------|
| F1   | Tier-0 completion (`markPrice@1s` + `forceOrder`)   | ✅                     |
| F2   | REST depth snapshot on admission                    | ✅                     |
| F3   | Partial-book depth WS                               | ✅                     |
| F4   | Full diff depth + periodic re-anchor                | ✅ (in-memory book deferred) |
| F5   | TF windows for selector aggregates                  | ✅                     |

#### F1 — Finish Tier-0: `markPrice@1s` + `forceOrder`

Per-symbol subscriptions for the mark-price / index / funding feed
(`<sym>@markPrice@1s`, 1 Hz) and the liquidation feed
(`<sym>@forceOrder`, bursty) so every admitted symbol has the
complete Tier-0 quadruple on disk. Implementation (`detail/subs.cpp`
+ `detail/drain.cpp`):

- `per_symbol_sinks` grew `mark_price` / `force_order` slots.
- `async_subscribe(...)` admission pack widened to four subscription
  types (`aggregate_trade_subscription`, `book_ticker_subscription`,
  `mark_price_subscription{.every_1s=true}`,
  `liquidation_order_subscription`). Symmetric `async_unsubscribe`.
- `drain_loop` routing gained two branches:
  `type.starts_with("markPrice")` (prefix — the combined endpoint
  emits `<sym>@markPrice` or `<sym>@markPrice@1s` depending on
  cadence) and exact `type == "forceOrder"`.
- Topic parsing by `parse_stream_header` is deliberately a cheap
  substring scan, not a JSON parse, so routing is trivial.

**Testnet note**: `forceOrder` rarely fires on testnet — empty
segments for most symbols are expected, not a bug.

#### F2 — REST depth snapshot on admission

When the detail monitor admits a symbol, immediately pull
`/fapi/v1/depth?limit=1000` via REST and persist the response as a
one-shot JSON file. This is the anchor F4's diff stream needs for
offline reconstruction, and it's useful on its own for any per-symbol
replay work. Implementation (`detail/snapshot.{hpp,cpp}` +
`detail/subs.cpp`):

- `detail_monitor_run` now creates its own
  `futures_usdm_api` + `rest::client` — an independent HTTP connection
  for the detail stage, same pattern as `rest_sync_run`. Two clients
  doubles the TLS setup cost but keeps the stage lifetimes clean.
- New `fetch_depth_snapshot(client, root, symbol, tag)` runs
  `order_book_request_t{.symbol, .limit=1000}`, serialises via glaze,
  wraps as `{"t":"<iso>","d":<response>}`, writes to
  `detail/<SYM>/depth_snapshot/<tag>.<UTCts>.json`.
- Admission fires the snapshot (`tag = "startup"`) **before** the WS
  subscribe. On failure it logs warn and the symbol still gets its
  non-depth streams.
- `detail_state` gained `snaps_ok` / `snaps_err` counters, surfaced
  in the status line as `detail{… snaps=N/E …}`.

**Known trade-off**: a stalled REST call blocks admission for that
tick. Acceptable at 10 s detail cadence; can be moved to a
`cobalt::spawn`-and-forget future later if it starts mattering.

#### F3 — Partial-book depth WS

Wired the existing `--with-depth` + `--depth-levels {5,10,20}` flags
to `partial_book_depth_subscription{.symbol, .levels, .speed="100ms"}`.
Implementation (`detail/subs.cpp`, `detail/types.hpp`,
`detail/drain.cpp`):

- `per_symbol_sinks` gained a `depth` slot, opened as
  `detail/<SYM>/depth<N>/` when `cfg.with_depth &&
  cfg.depth_mode == partial`.
- `drain_loop` routes any `type.starts_with("depth")` → `psink.depth`.
  The same slot is reused by F4's full diff mode (different sink dir).

**Rate-limit gotcha**: a first attempt issued a **separate** SUBSCRIBE
message for depth on top of the 4-way Tier-0 pack. That doubled the
control-message rate to the broker and tripped Binance's 5
messages-per-second cap — Beast answered with `Operation canceled`
and dropped the connection. The fix: **combine** depth into the same
single `async_subscribe(...)` call as the four Tier-0 topics
(variadic pack handles different arities cleanly). One control
message per admission → 25 symbols admitted in the first tick still
stays under the cap.

#### F4 — Full diff depth + periodic re-anchor

The heavy-storage, research-grade mode. `--full-depth` switches from
the partial stream to the full diff stream
(`<sym>@depth@100ms`) and periodically re-fires the REST snapshot
so offline reconstruction has multiple anchor points inside a long
run. Implementation (`config.{hpp,cpp}`, `detail/types.hpp`,
`detail/subs.cpp`):

- New config: `depth_resnap_seconds{600}` + CLI flag
  `--depth-resnap-seconds`.
- `per_symbol_sinks::next_resnap_at` (`steady_clock::time_point`)
  tracks each symbol's next-due re-anchor deadline. `time_point::max()`
  for symbols not in full-depth mode.
- Admission branches three ways on depth mode:
  - **partial**: existing F3 path, `depth<N>` sink.
  - **full**: opens `detail/<SYM>/depth_diff/`, subscribes
    `diff_book_depth_subscription{.speed="100ms"}`, sets
    `next_resnap_at = now + depth_resnap_seconds`.
  - **off**: no depth slot, no subscription.
- All three modes combine the depth subscription into the single
  admission-call pack (same 5-per-second avoidance as F3).
- New housekeeping pass at the end of each `manage_subs_loop` tick
  (full mode only): walks `st.sinks`, fires
  `fetch_depth_snapshot(..., "resnap")` for any symbol whose deadline
  has passed, advances the deadline. Re-snaps are staggered naturally
  by admission time.
- Eviction symmetric.

**Deferred — in-memory `local_order_book`**: the library already
ships a `binapi2::fapi::order_book::local_order_book` class that
does the full buffer-diffs-until-snapshot-arrives protocol, but it
owns its own `market_stream` per symbol — which would create one WS
connection per active symbol and undermine the "single shared
`dynamic_market_stream`" architecture the detail monitor is built
around. A hand-rolled per-symbol book fed out of the shared drain is
straightforward but only justified when there's a concrete use case
(live trading, live verification). The recorder already captures the
raw diff stream + all snapshots, which is everything an offline
rebuild needs — live reconstruction is replay-side scope.

**Storage reminder**: `--full-depth` on BTCUSDT alone is 20–80 GB/day
raw. The zstd rotating sink cuts that substantially but it's still
the heaviest data source the recorder emits. Drop `--full-depth` to
get partial mode (~1–2 orders of magnitude less), or drop
`--with-depth` entirely for a light run.

#### F5 — Real aggregates: TF windows for the selector

Orthogonal to F1–F4 — this step changes **which** symbols get the
detail treatment, not what gets recorded. Before F5 the selector
scored symbols on total `!bookTicker` event count since startup,
which is monotonic and reduces to "alphabetical-ish noise" after the
first minute. F5 replaces that with bucketed rolling TF windows fed
by the 1 Hz `!ticker@arr` stream. Implementation
(`aggregates.hpp`, `screener.cpp`, `selector.cpp`,
`selector_test.cpp`):

- **`tf_window`** class: fixed-size ring buffer of `{vol, trades}`
  buckets, `update(now, dv, dt)` slides the head forward (zero-
  filling skipped buckets), `vol_sum()` / `trades_sum()` return the
  rolling totals. A gap bigger than the window wipes all buckets
  (the next sample is the only one that counts).
- **`symbol_agg`** now carries `win_1m` (60 × 1 s), `win_5m` (60 × 5 s),
  and `win_1h` (60 × 60 s), plus `last_quote_volume` /
  `last_trade_count` / `primed` for delta accounting.
- **`update_aggregates`** gained an overload for
  `std::vector<ticker_stream_event_t>` (the all-market 1 Hz array):
  for each entry, reads `quote_volume.to_double()` and `trade_count`,
  computes the delta against the cached 24 h totals, feeds all three
  windows. First sample for a symbol is a **warm-up tick** — cached
  but not credited to the windows.
- **Midnight rollover**: Binance's 24 h totals wrap around 00:00 UTC.
  When the new total is *lower* than the cached one, the delta is
  the new total itself (treat the post-wrap value as "this tick's
  contribution"), not a negative number.
- **`selector::score_symbol`** became a weighted sum:
  `w_volume * (vol_1m + vol_5m + vol_1h) +`
  `w_trades * (trades_1m + trades_5m + trades_1h) +`
  `w_volume * events_total` (last term retained as a warm-up
  liveness fallback before `!ticker@arr` has primed the windows).
  `w_range` and `w_natr` are still placeholders — neither
  component has a populated data source on the live feed yet.
- **Four new gtests** under
  [`selector_test.cpp`](selector_test.cpp):
  `TfWindow.AccumulatesAcrossBuckets`, `TfWindow.SlidesOutOldBuckets`,
  `TfWindow.BigGapWipesWindow`, `Selector.ScoresFromTfWindows`.

**Live smoke result**: testnet top-5 went from alphabetical-ish
filler to plausible majors — `BNBUSDC` at score 721, `BNBUSDT` at
242, `DOGEUSDT` at 152, `JTOUSDT` at 112 — after ~30 s of warm-up.
The score spread (95 → 721) is an order of magnitude compared to
the pre-F5 uniform score of 2.00.

## 9. Resolved design decisions

These are locked in and inform the code; re-opening any of them should
come with a design note.

1. **Trades stream** — `aggTrade` only. Futures has no non-aggregated
   trade stream.
2. **Depth stream selection** — default `@depth20@100ms`;
   `--depth-levels {5,10,20}` to pick partial granularity;
   `--full-depth` for the diff stream + local book. Price-band filters
   are replay-side, not recording-side.
3. **Aggregates durability** — in-memory only. A restart loses ~1 day
   of warm-up; short TFs rebuild within minutes.
4. **Selector config format** — YAML via Glaze.
5. **Segment retention** — `K = 0`, compress every segment immediately
   on rotation. Live data is always readable from the ongoing segment.
6. **Stats destination** — `spdlog` only (no HTTP endpoint, no
   Prometheus exporter). External dashboards can tail the log file.

## 10. References

- Design plan: [`docs/binapi2/plans/async_recorder.md`](../../../docs/binapi2/plans/async_recorder.md)
- Market-data guide: [`docs/binapi2/guide/02-market-data.md`](../../../docs/binapi2/guide/02-market-data.md)
- Library streams overview: [`docs/binapi2/streams.md`](../../../docs/binapi2/streams.md)

The follow-up plan that tracked F1–F5 lived at
`docs/binapi2/plans/async_recorder_followups.md` while the work was
in-flight; it has since been folded into §8 of this README and
removed.
