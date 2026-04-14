# async-recorder

Institutional-grade recorder example built on `binapi2::fapi`. Runs four
coroutine stages on a single `cobalt::main` executor:

1. **Screener** — subscribes to the three all-market WS streams
   (`!bookTicker`, `!markPrice@arr@1s`, `!ticker@arr`) and archives raw
   frames as rotating, fsync'd, zstd-compressed JSONL. Maintains a
   per-symbol aggregates map fed from `!bookTicker` events.
2. **Selector** — on a configurable tick, scores every symbol from the
   aggregates map, applies hysteresis / min-hold / cooloff / bounds /
   mandatory rules, emits add/remove signals to `selector/signals.jsonl`.
3. **Detail monitor** — tracks the selector's active set; for every
   admitted symbol, opens `SUBSCRIBE` for its Tier-0 per-symbol streams
   (`@aggTrade`, `@bookTicker` in this version), routes raw frames into
   per-symbol rotating sinks, and `UNSUBSCRIBE`s + closes sinks when the
   selector evicts the symbol.
4. **REST sync** — periodically fetches funding rate, 1-minute klines,
   open interest history, and long/short ratio via REST and appends
   each response to a per-endpoint JSONL file.

A fifth stage, the **status reporter**, ticks on the same interval
(`--stats-seconds`, default 10 s) and emits one spdlog line containing
the full state of every other stage — buffer push/drain/occupancy
counters, selector active set, detail subscriptions, REST fetch
tallies.

See [`docs/binapi2/plans/async_recorder.md`](../../../docs/binapi2/plans/async_recorder.md)
for the full design and the locked-in decisions.

## Building & running

The example is built by the project's top-level CMake. A debug build
with the default testnet target:

```
./build.sh
./_build/examples/binapi2/fapi/async-recorder/binapi2-fapi-async-recorder \
    --root ./data --stats-seconds 5 \
    --selector examples/binapi2/fapi/async-recorder/selector.yaml
```

Ctrl-C / SIGTERM triggers graceful shutdown: every stage sees its next
timer wake return with a cancel, drains its buffers, closes its files,
and fsync+compresses the final rotating segment.

## CLI flags

| Flag                          | Default        | Purpose                                                           |
| ----------------------------- | -------------- | ----------------------------------------------------------------- |
| `--root <dir>`                | `./data`       | Output root directory.                                            |
| `--selector <file.yaml>`      | (built-in)     | Selector config YAML (weights, thresholds, hold, bounds, etc.).   |
| `--rotate-size <bytes>`       | 100 MiB        | Rotation size for the screener/detail JSONL sinks.                |
| `--rotate-seconds <s>`        | 900            | Rotation time for the same sinks.                                 |
| `--depth-levels {5,10,20}`    | 20             | Partial-book depth levels (detail monitor, future step).          |
| `--full-depth`                | off            | Use full diff depth instead of partial (future step).             |
| `--with-depth`                | off            | Record depth at all (future step).                                |
| `--no-klines`                 | on             | Disable per-symbol 1m kline recording (detail monitor).           |
| `--stats-seconds <s>`         | 10             | Status-reporter / selector / detail / rest_sync tick interval.    |
| `--loglevel <lvl>`            | `trace`        | spdlog level: trace/debug/info/warn/error/critical/off.           |
| `--logfile <path>`            | —              | Also write logs to this file (truncated on start).                |
| `--debug-stream <name>`       | (off)          | Run single-stream debug screener against `bookTicker` / `markPriceArr` / `tickerArr`. |
| `--live`                      | off (testnet)  | Use production endpoints.                                         |
| `--testnet`                   | on             | Use testnet endpoints (default).                                  |
| `--print-config`              | —              | Parse flags + YAML, dump resolved config, exit.                   |
| `-h`, `--help`                | —              | Print help and exit.                                              |

## Output directory layout

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
    ETHUSDT/
      aggTrade/…
      bookTicker/…
    …one directory per currently-active symbol…
  rest/
    fundingRate/       fundingRate.jsonl            (one line per fetch)
    klines_1m/         <SYMBOL>.jsonl               (one line per fetch)
    openInterestHist/  <SYMBOL>.jsonl               (one line per fetch)
    longShortRatio/    <SYMBOL>.jsonl               (one line per fetch)
```

Rotating sinks write segments until they hit `--rotate-size` bytes or
`--rotate-seconds` age, then `fsync(2)` the fd, `close(2)` it, and
`posix_spawn` `zstd --rm` out-of-process so the `.jsonl` is replaced
atomically by `.jsonl.zst`. The current (unrotated) segment is always
uncompressed `.jsonl`. REST fetches are one line per tick wrapped as
`{"t":"<ISO8601>","d":<raw response>}`.

## Status line format

On each tick the status reporter emits one `spdlog::info` line
containing a snapshot from every registered source, e.g.:

```
status: screener{bt[push=1524 drn=1524 occ=0/16384 hw=32 susp=0]
                 mp[push=27 drn=27 occ=0/16384 hw=1 susp=0]
                 tk[push=13 drn=13 occ=0/16384 hw=1 susp=0]}
      | selector{active=25/25 signals=23}
      | rest{fund=1/0 kln=5/0 oi=0/5 lsr=0/5 active=25}
      | detail{subs=25/25 buf=[push=343 drn=343 occ=0 hw=23]}
```

Each buffer entry shows `push`, `drain`, current `occupancy/capacity`,
`high_water` mark, and `push_suspends` (times the producer had to wait
for backpressure). Non-zero `susp` means the sink is behind.

## Selector YAML

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

Unit tests for the selector (`async_recorder_selector_test`) cover
mandatory pre-admission, bounds, hysteresis dead-band, min-hold, and
the `min_active` floor.

## Known scope cut-outs

- Aggregates are only populated from `!bookTicker` (volume proxy via
  event count). Full TF windows (1m/5m/1h/4h/1d volume, trades, range,
  NATR) come in a follow-up pass.
- Detail monitor only subscribes to `@aggTrade` + `@bookTicker`. The
  other Tier-0 streams (`@markPrice@1s`, `@forceOrder`, `@depth20@100ms`,
  `@kline_1m`) are the next expansion.
- REST analytics endpoints (`openInterestHist`, `longShortRatio`) are
  not exposed on testnet — they return errors there. Run against
  `--live` to see them populate.
- Depth snapshot on-demand (REST pull when a symbol is added) is
  planned but not wired yet.
- No integration test against `compose/postman-mock/`; smoke tested
  end-to-end on testnet instead.
