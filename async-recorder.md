



examples/binapi2/fapi/async-recorder





practical institutional-grader screener and history recorder

Goals:
✅ deterministic replay
✅ realistic fills
✅ factor research
✅ execution research
✅ signal extraction
✅ storage discipline


Main idea is to record details only for active instruments.

So split work into 3 levels:
1. Screener
    - see stream-all-* data
    - record raw jsonl stream data
        - with rotation and compression
            - write raw jsonl file until limit
            - then start writing next one (rotation)
            - compress finished one
    - calculate aggregates required for selector
2. Instrument selector
    - selects active/interesting symbols
    - filters: min volume, etc...
    - rules: when add and remove symbols
        - prevent constant add/remove
            - two limits for add and remove
            - histeresis loop
        - min duration
        - min/max number of symbols (in addition to mandotory)
        - mandatory required symbols (BTC, ETH)
    - scores:
        - on main TFs: 1m, 5m, 1h, 4h, 1d
            - volume
            - trades
            - normalized range (gainers, losers)
            - NATR
        - assign score points
        - select N with top score
    - record signals with reasons
3. Detailed monitoring
    - aggregated trades vs. trades (which to select?)
    - L2 incremental order book with snapshots
    - open interest
    - funding
    - klines



---


Tier 0:
- bookTicker
- aggTrade
- markPrice
- liquidation

Tier 1:
- diff depth (L2 incremental order book)
- periodic REST order-book snapshots

Tier 2:
- raw trades
- klines 1m
- funding
- open interest

Tier 3:
- long-short ratio
- taker buy-sell volume
- index price


Live WS resord:
```
aggTrade
bookTicker
markPrice
forceOrder
depth@100ms (optional top symbols only)
```

REST periodical sync:
```
depth snapshot
funding
open interest
long short ratio
1m klines
```






# Binance USDⓈ-M Futures: Full Institutional-Grade Recording Set

Practical ranking for **institutional-grade replay / research / execution simulation**, optimized by:

- **Usefulness**
- **Storage cost**
- **Alpha density**

Goal:

- deterministic replay  
- realistic fills  
- signal extraction  
- execution research  
- long-term extensibility  

---

# Tier 0 — Absolutely Mandatory

These provide the **highest value per stored byte**.

---

# 1) aggTrade ⭐ Highest value / byte

## Stream

`<symbol>@aggTrade`

## Contains

- executed price  
- executed quantity  
- taker side  
- trade timestamp  
- aggregated micro fills  

## Why mandatory

Primary source for:

- tape replay  
- delta  
- CDV  
- volume impulses  
- burst detection  
- local volatility  
- micro momentum  

## Alpha value

Extremely high.

Many intraday crypto systems can be built almost entirely on aggTrades.

## Storage

Moderate.

Much smaller than raw trades.

## Institutional note

Usually preferred over raw trade because:

raw trades often add little extra alpha relative to storage cost.

---

# 2) bookTicker ⭐ Mandatory liquidity layer

## Stream

`<symbol>@bookTicker`

## Contains

- best bid  
- best ask  
- best bid quantity  
- best ask quantity  

## Why mandatory

Adds:

- spread  
- micro pressure  
- queue pressure proxy  
- bid/ask asymmetry  
- realistic entry modeling  

## Alpha value

Very high.

Spread behavior often predicts short micro moves.

## Storage

Very cheap.

Excellent value per byte.

---

# 3) markPrice ⭐ Mandatory futures-specific layer

## Stream

`<symbol>@markPrice`

## Contains

- mark price  
- index price  
- funding rate  
- next funding timestamp  

## Why mandatory

Needed for:

- liquidation modeling  
- pnl realism  
- funding regime  
- premium regime  

## Alpha value

Very high in futures.

Especially around:

- funding windows  
- volatility spikes  
- squeeze conditions  

## Storage

Tiny.

Almost free.

---

# 4) liquidation stream ⭐ Hidden alpha source

## Stream

`<symbol>@forceOrder`

## Contains

- liquidation side  
- liquidation price  
- liquidation quantity  

## Why mandatory

Unique signal for:

- squeeze detection  
- liquidation cascades  
- exhaustion detection  

## Alpha value

Very high, especially in altcoins.

## Storage

Tiny.

Near zero cost.

---

# Tier 1 — Strongly Recommended

---

# 5) diff depth (incremental L2 book)

## Stream

`<symbol>@depth@100ms`

## Why record

Needed for:

- order book reconstruction  
- imbalance  
- spoofing research  
- slippage model  
- queue simulation  

## Alpha value

Very high for execution systems.

## Storage

Very expensive.

BTCUSDT may reach:

20–80 GB/day

## Institutional advice

Usually:

store only reconstructed top levels instead of full raw forever.

---

# 6) Periodic REST order book snapshots

## REST endpoint

`/fapi/v1/depth`

## Why mandatory with diff depth

Diff stream alone is incomplete.

Snapshots are required for exact reconstruction.

## Storage

Low.

---

# Tier 2 — Valuable Historical Enrichment

---

# 7) Raw trades (optional over aggTrade)

## REST / WS

- `/fapi/v1/trades`
- `trade`

## Why optional

Adds:

- exact fills  
- zero aggregation loss  

## But usually

Very small alpha gain compared to aggTrade.

## Storage

High.

## Institutional reality

Often skipped.

---

# 8) Klines (1m baseline mandatory)

## REST / WS

1m candles

## Why still record

Needed for:

- fast factor building  
- coarse scans  
- sanity checks  
- multi-timeframe systems  

## Recommendation

Record only:

- 1m

Higher TF derive locally.

## Storage

Cheap.

## Alpha

Medium.

Useful but insufficient alone.

---

# 9) Funding history

## REST

`/fapi/v1/fundingRate`

## Why

Needed for:

- funding regime research  
- carry models  
- squeeze context  

## Storage

Tiny.

## Alpha

High for longer regime analysis.

---

# 10) Open interest history

## REST endpoints

Open interest history endpoints

## Why

Adds:

- participation regime  
- leverage buildup  
- breakout confirmation  

## Alpha value

Very high.

## Storage

Tiny.

---

# Tier 3 — Advanced Institutional Enrichment

---

# 11) Long / short ratios

## REST

- global long short ratio  
- top trader long short ratio  

## Why

Useful for:

- crowd positioning  
- contrarian filters  

## Alpha

Moderate but useful in regime analysis.

---

# 12) Taker buy / sell volume history

## REST

Historical taker volume endpoints

## Why

Efficient aggression factor.

## Storage

Tiny.

---

# 13) Index price klines

## Why

Needed for:

- premium distortion  
- basis analysis  

---

# 14) Premium index history / stream

## Why

Strong futures-specific factor.

---

# Tier 4 — Optional Research Only

---

# 15) miniTicker / ticker

Usually redundant if better primitives already recorded.

---

# 16) Contract info stream

Useful only for:

- symbol lifecycle  
- contract changes  

---

# 17) Insurance fund / ADL stats

Mostly research-only.

Rarely directly useful.

---

# Institutional Recommended Final Stack

---

# Live WebSocket

Record:

```text
aggTrade
bookTicker
markPrice
forceOrder
depth@100ms (top symbols only)
```

---

# REST Periodic Sync

Record:

```text
depth snapshot
funding
open interest
long short ratio
1m klines
```

---

# Best Storage Architecture

Separate event classes:

```text
TRADE
QUOTE
MARK
LIQ
DEPTH
STATE
```

---

Store append-only binary log:

```text
timestamp | event_type | payload
```

---

# Institutional Rule

Never store indicators.
Store primitives only.
Indicators are disposable.
Raw events are permanent.


















