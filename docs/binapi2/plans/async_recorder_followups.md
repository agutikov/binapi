# async-recorder — follow-up plan: closing the detail-recording gaps

Baseline is the 8-step plan in `async_recorder.md`, implemented across
commits `scaffolding` → `periodic REST sync`. What's currently recorded
per selected symbol is only **2 of 4** Tier-0 WS streams (`aggTrade` +
`bookTicker`), plus three REST endpoints scoped to the active set
(`klines_1m`, `openInterestHist`, `longShortRatio`) and a market-wide
`fundingRate` pull. This doc plans the five concrete follow-up steps
required to bring the recorder up to the full design.

Each step lists scope, files, work items, risks, and a definition of
done. Steps are ordered by dependency and effort — do them top-down.

---

## F1 — Finish Tier-0: `markPrice@1s` + `forceOrder`

Fill in the two remaining Tier-0 per-symbol WS feeds so every symbol the
selector admits has the complete Tier-0 quadruple on disk.

### Scope

Add per-symbol subscriptions for:
- `<sym>@markPrice@1s` — mark price, index price, funding rate, next
  funding time. 1 Hz cadence.
- `<sym>@forceOrder` — individual liquidations on that symbol. Event
  rate is bursty; often silent.

### Files touched

- `examples/binapi2/fapi/async-recorder/detail.cpp` (only file needed)

### Work items

1. Extend `per_symbol_sinks`:
   ```cpp
   struct per_symbol_sinks {
       std::unique_ptr<rfs> agg_trade;
       std::unique_ptr<rfs> book_ticker;
       std::unique_ptr<rfs> mark_price;   // NEW
       std::unique_ptr<rfs> force_order;  // NEW
       std::size_t frames_routed{0};
   };
   ```
2. In `manage_subs_loop` admission branch:
   - Open 2 more sinks under `detail/<SYM>/markPrice/` and
     `detail/<SYM>/forceOrder/`.
   - Widen `async_subscribe(...)` to pass four subscriptions:
     `aggregate_trade_subscription`, `book_ticker_subscription`,
     `mark_price_subscription{.symbol=sym}`,
     `liquidation_order_subscription{.symbol=sym}`.
3. Mirror the 4-way list in the `async_unsubscribe(...)` eviction path.
4. Extend `drain_loop` routing. The `parse_stream_header` split on the
   first `@` yields `type = "markPrice@1s"` for the mark-price topic,
   so compare with a prefix check (`type.starts_with("markPrice")`)
   rather than equality. `forceOrder` has no suffix — equality works.
5. Bump the status source to include per-sink push counts if cheap
   (optional — the shared input buffer already shows the aggregate).

### Risks

- **Testnet sparseness.** `forceOrder` rarely fires on testnet; expect
  empty segments for most symbols. Not a bug.
- **Topic split.** The combined endpoint delivers `btcusdt@markPrice@1s`
  — make sure `parse_stream_header` returns the right pieces. If it
  only splits once, `type` contains the trailing `@1s`; the prefix
  match handles that cleanly.

### Definition of done

- A testnet smoke run shows `detail/BTCUSDT/{aggTrade,bookTicker,markPrice,forceOrder}/`
  all created, and at least `markPrice/` accumulates bytes (bookTicker
  and markPrice are guaranteed to emit on any live symbol).
- No regressions on the existing 393 tests.

### Effort

**~1–2 hours.** Two sink slots, two subscription types, one routing
branch — and the subscribe/unsubscribe arity change is mechanical.

---

## F2 — REST depth snapshot on admission

When the detail monitor admits a symbol, immediately pull
`/fapi/v1/depth?limit=1000` via REST and persist it as a one-shot
snapshot under that symbol's directory. This is the Tier-1 prerequisite
for reconstructing an order book from the diff stream in F4, and it's
useful on its own for any per-symbol replay tooling.

### Scope

- Fire one `order_book_request_t{.symbol=sym, .limit=1000}` REST
  call per admission, **before** sending the WS `SUBSCRIBE` for the
  depth stream (when F3/F4 add it).
- Write the response as a single JSON file at
  `detail/<SYM>/depth_snapshot/startup.<ts>.json`.
- On failure, log a warning and continue — the symbol still subscribes
  to its non-depth WS streams.

### Files touched

- `examples/binapi2/fapi/async-recorder/detail.cpp`
- Possibly a small helper header if the REST-snapshot logic is shared
  with F4's periodic re-anchoring.

### Work items

1. Give the detail monitor a REST client. Create its own
   `futures_usdm_api` + `rest::client` inside `detail_monitor_run` (the
   same pattern `rest_sync_run` uses — independent HTTP connection,
   independent lifetime).
2. Add a `fetch_depth_snapshot(client, root, symbol)` free function:
   - Build the request.
   - `co_await client.market_data.async_execute(req)`.
   - On success: `glz::write_json`, wrap as
     `{"t":"<iso>","d":<response>}`, write to the new file.
   - On failure: `spdlog::warn` with the error message.
3. Call it from the admission branch of `manage_subs_loop`, *before*
   the `async_subscribe` for the symbol. Run it as a plain `co_await`
   — the call is fast and serial-per-symbol is fine.
4. Stats: add an `oi_snapshots_ok` / `oi_snapshots_err` pair under the
   `detail` status source so the line reveals how many snapshots have
   landed.

### Risks

- **Request hang.** If the REST call stalls, admission is blocked for
  that symbol — subsequent admissions wait behind it. Acceptable for
  v1 because the detail monitor's periodic tick is 10 s by default,
  so a stall hurts only when Binance is unhealthy. Can be moved to a
  `cobalt::spawn`-and-forget future later if it becomes a problem.
- **Two clients.** `rest_sync_run` already has a REST client. Having
  two concurrent clients is fine with the binapi2 pipeline but doubles
  the TLS setup cost. An alternative is to share one client via a
  thread-safe wrapper — deferred until it matters.

### Definition of done

- After a 20 s smoke run with `--stats-seconds 5`, every admitted
  symbol has a non-empty `detail/<SYM>/depth_snapshot/startup.*.json`
  file (or a warn line if testnet rejected the depth call, which it
  won't for common symbols).
- Status line shows `detail{… snaps=N/E …}`.

### Effort

**~2 hours.** Mostly plumbing the REST client into the detail stage.

---

## F3 — Partial-book depth WS stream

Wire the CLI switches that already exist (`--with-depth`,
`--depth-levels {5,10,20}`) to an actual per-symbol partial-book
depth subscription. This is the cheap depth mode: ~1 KB per frame,
~10 fps per symbol, usable for imbalance / microprice / queue features.

### Scope

When `cfg.with_depth && cfg.depth_mode == depth_mode_t::partial`:
- Subscribe each admitted symbol to `<sym>@depth<N>@100ms` where
  `N ∈ {5,10,20}` from `cfg.depth_levels`.
- Record raw frames to `detail/<SYM>/depth<N>/*.jsonl[.zst]`.

### Files touched

- `examples/binapi2/fapi/async-recorder/detail.cpp`

### Work items

1. Extend `per_symbol_sinks` with a `std::unique_ptr<rfs> depth;`
   slot (used when `cfg.with_depth` is true).
2. In the admission branch, when depth is enabled:
   - Open `detail/<SYM>/depth<N>/` with basename `depth<N>`.
   - Add `partial_book_depth_subscription{.symbol=sym, .levels=cfg.depth_levels, .speed="100ms"}`
     to the `async_subscribe` pack.
3. Routing: `type` for partial depth is `depth<N>@100ms` (e.g.
   `depth20@100ms`). Match with `type.starts_with("depth")`.
4. Symmetric `async_unsubscribe`.
5. Status source bumps an `depth_frames` tally if cheap.

### Risks

- **Subscription arity.** The library's `async_subscribe` uses a
  variadic template; adding a fifth subscription type in the pack is
  supported. If the wire protocol hits an arity issue, split the
  subscribe into two calls (2 + 3).
- **Buffer saturation.** Depth adds real volume on top of aggTrade+
  bookTicker. With 25 symbols × 10 fps × 1 KB ≈ 250 KB/s, the
  current 16384-entry shared input buffer is still fine. Monitor
  `detail.buf[susp]` for any non-zero value — if seen, raise capacity
  to 65 536.

### Definition of done

- `./binapi2-fapi-async-recorder --with-depth --depth-levels 20`
  creates `detail/<SYM>/depth20/*.jsonl` and those files grow.
- Default run (no `--with-depth`) has no `depth*` directories.
- Existing bookTicker/aggTrade/markPrice/forceOrder files still
  populate normally.

### Effort

**~1–2 hours.** Adds one sink, one subscription, one routing branch.

---

## F4 — Full diff depth + local order book reconstruction

The heavy-storage, research-grade mode. `--full-depth` switches from the
partial stream (F3) to the full diff stream, and the recorder
reconstructs a `local_order_book` in memory so it can verify and
replay the book over any window. Storage is expensive (20–80 GB/day
per major symbol raw, per the plan), so this path is opt-in.

### Scope

When `cfg.with_depth && cfg.depth_mode == depth_mode_t::full`:
- Subscribe each admitted symbol to `<sym>@depth@100ms`.
- Record raw diff frames to `detail/<SYM>/depth_diff/*.jsonl[.zst]`.
- Seed a per-symbol `binapi2::fapi::order_book::local_order_book` from
  the F2 startup depth snapshot, then feed every diff event through it.
- Re-snapshot every `--depth-resnap-seconds` (default 10 min) to
  recover from any missed update and to give replay tooling multiple
  anchor points. Write the new snapshot to
  `detail/<SYM>/depth_snapshot/resnap.<ts>.json`.

### Files touched

- `examples/binapi2/fapi/async-recorder/detail.cpp`
- `examples/binapi2/fapi/async-recorder/config.{hpp,cpp}`
  (add `--depth-resnap-seconds`)

### Work items

1. Config: new `std::uint64_t depth_resnap_seconds{600}` + CLI flag +
   `dump_config` line + usage entry.
2. `per_symbol_sinks` gains `std::unique_ptr<rfs> depth_diff;` and
   `std::unique_ptr<order_book::local_order_book> book;`.
3. Admission branch (when mode==full):
   - Pull REST snapshot (F2).
   - Seed the local book from the snapshot's bids/asks + lastUpdateId.
   - Subscribe `diff_book_depth_subscription{.symbol=sym, .speed="100ms"}`.
   - Track `last_resnap_at = steady_clock::now()`.
4. `drain_loop` depth-diff branch:
   - Always write the raw frame to the depth_diff sink (replay
     fidelity).
   - Additionally parse to the typed `depth_stream_event_t` and pump
     into `book->apply_diff(...)` — the library already handles the
     sequence-number invariant and returns a snapshot callback style
     interface.
5. New housekeeping tick in `manage_subs_loop` (already ticks per
   `--stats-seconds`): for each subscribed symbol, if
   `now - last_resnap_at[s] >= depth_resnap_seconds`, fire a REST
   snapshot, write the new file, and re-seed the local book.
6. Status source: per-symbol book health (last_update_id, levels count)
   can be exposed inline as an aggregate — e.g. `books_ok=N/25`.

### Risks

- **Race between snapshot and diff seeding.** The library's
  `local_order_book` handles the standard "buffer diffs until you have
  the snapshot, then discard diffs older than lastUpdateId" protocol;
  reuse the existing utility rather than hand-rolling.
- **Disk throughput.** Full depth on a 25-symbol set will push the
  `rotating_file_sink` throughput budget. Sanity-check with stats:
  any `push_suspends > 0` on the shared input buffer means the sink is
  the bottleneck — switch to `--depth-levels 20` (partial) if so.
- **Re-snap cost.** 25 × HTTP(S) calls every 10 min is fine. If F2's
  single-client strategy holds, F4 reuses it.
- **Compile cost.** Pulling in `local_order_book.hpp` may drag in more
  library headers; not a problem but watch the build time.

### Definition of done

- `--full-depth --with-depth`:
  - `detail/<SYM>/depth_diff/*.jsonl` files grow.
  - `detail/<SYM>/depth_snapshot/` has `startup.<ts>.json` +
    periodic `resnap.<ts>.json` files.
  - The in-memory `local_order_book` for at least BTCUSDT is
    verifiable against the raw diff stream (spot-check: pick a random
    diff from the file, assert the update_id is contiguous with the
    neighbour).
- `--full-depth` without `--with-depth` is a no-op (consistent with
  partial mode).
- No `push_suspends` > 0 on the detail buffer during a 5-minute run.

### Effort

**~half a day to one day.** The local-book plumbing is the tricky
part; the rest mirrors F3.

---

## F5 — Real aggregates: TF windows for the selector

Orthogonal to F1–F4 (which are all about WHAT to record per symbol).
F5 is about WHICH symbols the selector picks, so they get detailed
recording in the first place. Today the selector scores symbols on a
single proxy: total `!bookTicker` event count since startup. Symbols
with active quotes dominate — regardless of actual volume or volatility
— and the per-symbol ranking is essentially random after the first
minute because counts monotonically grow.

### Scope

Populate the full `symbol_agg` struct and use it in `score_symbol`.
Minimum viable version: **volume and trades on 1m / 5m / 1h windows**,
scored together with the current `events_total` fallback. NATR and
full 4h/1d windows can be layered on later without changing the
interface.

### Files touched

- `examples/binapi2/fapi/async-recorder/aggregates.hpp`
- `examples/binapi2/fapi/async-recorder/screener.cpp` (parse events,
  feed the TF windows)
- `examples/binapi2/fapi/async-recorder/selector.cpp` (scoring)
- `examples/binapi2/fapi/async-recorder/selector_test.cpp` (tests
  for the new scoring path)

### Data sources

- `!ticker@arr` already arrives at 1 Hz with full 24 h stats (close,
  quoteVolume, tradeCount, priceChange). That's the primary feeder.
- Per-symbol `klines_1m` from REST (already fetched every 1 min) can
  provide OHLC for NATR — deferred unless needed.
- `!bookTicker` continues to act as a "is this symbol alive" fallback.

### Work items

1. **Data model**:
   ```cpp
   struct tf_window {
       // Bucketed rolling window. n buckets of `bucket_seconds` each.
       std::vector<double> vol_buckets;
       std::vector<double> trades_buckets;
       std::size_t head{0};        // index of most recent bucket
       std::chrono::steady_clock::time_point head_time{};
       double vol_sum() const;
       double trades_sum() const;
       void advance(time_point now, double d_vol, double d_trades);
   };

   struct symbol_agg {
       std::size_t events_total{0};  // kept for backwards-compat
       tf_window win_1m;             // 60 x 1 s
       tf_window win_5m;             // 60 x 5 s
       tf_window win_1h;             // 60 x 1 min
       // ... 4h, 1d later
   };
   ```
2. **Screener hooks**:
   - Extend `update_aggregates` overloads. For `ticker_array_event_t`
     (one array element per symbol), walk the array and for each
     entry:
     - `v = stod(total_traded_base_asset_volume)` difference since
       last tick (we need to cache last total per symbol to compute the
       delta — which *is* the per-second traded volume).
     - `t = trade_count` delta similarly.
     - Call `win_*.advance(now, dv, dt)` on the three (later five)
       windows.
   - Likewise for `book_ticker_stream_event_t` — keep incrementing
     `events_total` as a cheap liveness proxy.
3. **Selector scoring**:
   - New helpers on `symbol_agg`:
     `vol_1m()`, `trades_1m()`, `vol_5m()`, `trades_5m()`,
     `vol_1h()`, `trades_1h()`.
   - `score_symbol()` becomes a weighted sum of these + the existing
     `w_volume * events_total` fallback, using the weights already in
     `selector_config`.
4. **Tests**:
   - Synthesise a `symbol_agg` with non-zero 1m volume and zero
     everything else, verify the new scoring component is exercised.
   - Time-step a window across several minutes and verify the sum
     reflects the window contents correctly (slide-out).
   - Existing tests (mandatory, max cap, min hold, hysteresis, min
     floor) should all still pass.
5. **Clock discipline**:
   - The screener and selector share an executor; the status reporter
     already uses `steady_clock::now()`. Pass the same `now` into the
     aggregates update path to keep windows deterministic. The
     `selector::tick(aggs, now)` signature already accepts a clock
     value, so testing with a fake clock is already wired.

### Risks

- **Parse cost.** `!ticker@arr` is a biggish JSON with many entries,
  and decoding the numeric strings to doubles per tick adds cost. At
  1 Hz market-wide it's still trivial — benchmark in an F5 smoke run
  and only optimise if `push_suspends > 0`.
- **Volume = delta**. The 24 h total is monotonically increasing; the
  per-tick delta is what we want in the 1 s bucket. Cache last total
  per symbol to compute the delta correctly. Handle roll-over at
  midnight UTC (Binance wraps the 24 h window): if the new total is
  lower than the cached last, reset the cached total to 0 — treat
  that tick's delta as the new total.
- **Ring-buffer maintenance.** `advance` must tick through any
  inactive buckets even when no events arrive (e.g. the status report
  fires a tick independent of event arrival). Simplest: on every
  call, walk forward from `head_time` to `now` in bucket_seconds
  steps, zero-filling.

### Definition of done

- `symbol_agg` has the new TF fields populated from live ticker data.
- `selector::score_symbol` returns weighted scores reflecting real
  volume, not just event counts.
- Unit tests cover at least three new paths (1m delta, window slide,
  daily rollover).
- Live smoke: the selector's top-5 active symbols over a 5-minute run
  are plausible majors (BTC/ETH/SOL/…) rather than the current
  alphabetical-ish soup.

### Effort

**~1–2 days.** Bigger than F1–F4 combined because of the data model,
the delta-from-total accounting, and the test coverage.

---

## Suggested order

1. **F1** (1–2 h) — small, low risk, unlocks Tier-0 completeness.
2. **F2** (2 h) — pairs naturally with F1 (same file) and sets up F4.
3. **F3** (1–2 h) — wires the depth CLI flags end-to-end.
4. **F4** (0.5–1 day) — depends on F2 (snapshot) and F3 (depth
   routing). Largest of the detail-recording steps.
5. **F5** (1–2 days) — orthogonal to F1–F4. Do in parallel with F4 or
   immediately after. Biggest payoff for selection quality.

F1 + F2 together form a single reasonable commit ("tier-0 complete +
depth snapshot on admission"). F3 and F4 are separate commits. F5 is
its own branch because it touches the scoring core and the tests.
