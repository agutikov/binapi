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
      markPrice/       markPrice.<ts>.<nnnn>.jsonl[.zst]        (F1)
      forceOrder/      forceOrder.<ts>.<nnnn>.jsonl[.zst]       (F1)
      depth_snapshot/  startup.<ts>.json,  resnap.<ts>.json     (F2, F4)
      depth20/         depth20.<ts>.<nnnn>.jsonl[.zst]          (F3)
      depth_diff/      depth_diff.<ts>.<nnnn>.jsonl[.zst]       (F4)
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

Lines marked `(F1)` / `(F2)` / `(F3)` / `(F4)` are the gaps still open
against the design — see §8 (implementation status) and
[`async_recorder_followups.md`](../../../docs/binapi2/plans/async_recorder_followups.md).

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

Ctrl-C or SIGTERM triggers graceful shutdown.

Unit tests for the selector are registered under ctest:

```sh
./run_tests.sh -R Selector
```

## 8. Implementation status

| Area                               | Status | Notes                                                                  |
|------------------------------------|--------|------------------------------------------------------------------------|
| **Scaffolding & config**           | ✅     | `config.{hpp,cpp}`, CLI flags, YAML loader (Glaze), `main.cpp`        |
| **`rotating_file_sink`**           | ✅     | size+time rotation, fsync, `posix_spawn` zstd; 4 unit tests           |
| **`stream_buffer` counters**       | ✅     | push/drain/occupancy/high-water/suspends; used by status reporter     |
| **`status_reporter`**              | ✅     | single periodic logger, multi-source aggregation                      |
| **Screener — all-market streams**  | ✅     | `!bookTicker` + `!markPrice@arr@1s` + `!ticker@arr`, rotating JSONL   |
| **Screener — aggregates**          | 🟡     | Only `events_total` from `!bookTicker` is populated (see F5)          |
| **Selector — logic**               | ✅     | Scoring, hysteresis, min-hold, cooloff, bounds, mandatory             |
| **Selector — unit tests**          | ✅     | 6 gtests registered under ctest                                       |
| **Selector — `signals.jsonl`**     | ✅     | Append-only ISO-8601 JSONL                                            |
| **Detail — `aggTrade`**            | ✅     | Per-symbol WS, rotating sink                                          |
| **Detail — `bookTicker`**          | ✅     | Per-symbol WS, rotating sink                                          |
| **Detail — `markPrice@1s`**        | ❌     | Not subscribed — **F1**                                               |
| **Detail — `forceOrder`**          | ❌     | Not subscribed — **F1**                                               |
| **Detail — REST depth snapshot**   | ❌     | Not fired on admission — **F2**                                       |
| **Detail — partial depth WS**      | ❌     | CLI flag present, subscription not wired — **F3**                     |
| **Detail — full diff depth + book**| ❌     | No local book reconstruction — **F4**                                 |
| **REST — fundingRate**             | ✅     | Hourly, market-wide                                                   |
| **REST — klines_1m**               | ✅     | 1 min, per-active-symbol                                              |
| **REST — openInterestHist**        | ✅     | 5 min, per-active-symbol (testnet rejects — works on `--live`)        |
| **REST — longShortRatio**          | ✅     | 15 min, per-active-symbol (testnet rejects — works on `--live`)       |
| **REST — taker buy/sell, index**   | ❌     | Not implemented (Tier 3 enrichment)                                   |
| **Observability — one-line status**| ✅     | `screener | selector | rest | detail` fields                         |
| **Observability — `stats.jsonl`**  | ❌     | Status only in spdlog; no separate file sink                          |
| **Concurrency**                    | ✅     | Single `cobalt::main`, nested 2-task joins, graceful SIGINT shutdown  |
| **Docs — design plan**             | ✅     | `docs/binapi2/plans/async_recorder.md`                                |
| **Docs — follow-up plan**          | ✅     | `docs/binapi2/plans/async_recorder_followups.md` (F1–F5)              |
| **Integration test (postman-mock)**| ❌     | Testnet smoke-tested end-to-end instead                               |

### Open follow-ups

See [`docs/binapi2/plans/async_recorder_followups.md`](../../../docs/binapi2/plans/async_recorder_followups.md)
for the full plan. Summary:

| Step   | Scope                                                                 | Effort    | Depends on |
|--------|-----------------------------------------------------------------------|-----------|------------|
| **F1** | Finish Tier-0: `markPrice@1s` + `forceOrder` per-symbol              | 1–2 h     | —          |
| **F2** | REST depth snapshot on symbol admission                               | ~2 h      | —          |
| **F3** | Partial-book depth WS (`@depth{5,10,20}@100ms`)                      | 1–2 h     | —          |
| **F4** | Full diff depth + `local_order_book` reconstruction + periodic re-anchor | 0.5–1 d   | F2, F3     |
| **F5** | Real TF aggregates for the selector (1m/5m/1h volume + trades)       | 1–2 d     | —          |

F1 + F2 form a single natural commit (both touch `detail.cpp`). F3 is
independent. F4 is the heaviest of the WS additions. F5 is orthogonal —
it changes which symbols get detailed, not what gets recorded.

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
- Follow-ups plan: [`docs/binapi2/plans/async_recorder_followups.md`](../../../docs/binapi2/plans/async_recorder_followups.md)
- Market-data guide: [`docs/binapi2/guide/02-market-data.md`](../../../docs/binapi2/guide/02-market-data.md)
- Library streams overview: [`docs/binapi2/streams.md`](../../../docs/binapi2/streams.md)
