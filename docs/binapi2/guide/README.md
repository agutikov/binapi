# binapi2 user guide

A developer guide to the binapi2 Binance USD-M Futures client library, combining
Binance API semantics, crypto derivatives fundamentals, and binapi2 type mappings.

## Audience

This guide assumes you know C++ and want to trade or analyze Binance USD-M
Futures (perpetual or delivery contracts). No prior Binance API knowledge is
required — each concept is introduced in terms of what the data represents and
why you would use it.

## How to read this guide

Start with the architecture overview, then read whichever topic matches your
goal. Each topic document covers **all three API surfaces** (REST, WebSocket
API, Streams) for its subject area and explains when to use which.

| # | Document | Covers |
|---|----------|--------|
| 1 | [Architecture: three API surfaces](01-architecture.md) | How REST, WebSocket API, and Streams relate. When to use each. Latency, authentication, and duplication. |
| 2 | [Market data](02-market-data.md) | Order book, klines, trades, tickers, mark/index price, funding rate, analytics. REST snapshots vs stream deltas. |
| 3 | [Trading and orders](03-trading.md) | Order types, order lifecycle, placement, modification, cancellation, queries. REST vs WS API. Algo orders. |
| 4 | [Account, balances, positions](04-account-positions.md) | Account information, balances, position risk, leverage, margin type, position mode, multi-assets mode. |
| 5 | [User data stream events](05-user-stream-events.md) | Real-time account updates, order/trade events, margin calls, algo and strategy updates (10 event types). |
| 6 | [Core types and primitives](06-core-types.md) | `decimal_t`, `symbol_t`, `pair_t`, `timestamp_ms_t`, enums, `enum_set`, `result<T>`. |

## Quick map: what you want to do → which document

| Goal | Document(s) |
|------|-------------|
| Place a market or limit order | [03](03-trading.md), [06](06-core-types.md) |
| Display a live order book | [02](02-market-data.md) § local order book |
| React to my own order fills in real time | [05](05-user-stream-events.md) § order-trade updates |
| Query historical candles | [02](02-market-data.md) § klines |
| Monitor my account balance | [04](04-account-positions.md), [05](05-user-stream-events.md) |
| Understand `decimal_t` / `symbol_t` / enums | [06](06-core-types.md) |
| Choose between REST and WebSocket API for orders | [01](01-architecture.md), [03](03-trading.md) § latency trade-offs |
| Decode a mark price or funding rate | [02](02-market-data.md) § mark price & funding |

## Background concepts (brief)

If you're new to crypto derivatives, these terms recur throughout the guide:

- **Perpetual contract** — a futures contract with no expiry. Anchored to the
  spot index price via a periodic **funding rate** payment between longs and
  shorts. Most Binance USD-M symbols (e.g. `BTCUSDT`) are perpetuals.
- **Delivery contract** — a futures contract with a fixed expiry date (e.g.
  `BTCUSDT_250627`). No funding; settles at expiry to the mark price.
- **Pair vs symbol** — the *pair* is the base/quote asset combination
  (`BTCUSDT`). For perpetuals, pair and symbol are identical. For delivery,
  the symbol adds an expiry suffix: pair `BTCUSDT` → symbol `BTCUSDT_250627`.
- **Mark price** — an exchange-computed reference price used for margin and
  P&L calculations. Smoother than the last trade price to prevent manipulative
  liquidations.
- **Index price** — a volume-weighted average of the underlying spot price
  across major exchanges. Feeds into the mark price.
- **Funding rate** — periodic cash flow between perpetual longs and shorts
  (typically every 8 hours) that nudges the perpetual price toward the index.
  Positive → longs pay shorts; negative → shorts pay longs.
- **Maintenance margin** — the minimum collateral required to keep a position
  open. Falling below triggers **liquidation**.
- **Notional value** — position size in quote currency:
  `notional = quantity × mark_price`.
- **Leverage** — the multiplier between the notional and the margin required.
  Binance USD-M supports up to 125× on some symbols.
- **Cross vs isolated margin** — cross margin shares the entire wallet as
  collateral across positions; isolated pins a specific margin amount to one
  position.
- **Hedge vs one-way mode** — hedge mode allows simultaneous long and short
  positions on the same symbol (each direction tracked separately); one-way
  mode nets them.

These are all exposed as binapi2 types (`margin_type_t`, `position_side_t`,
funding fields in `mark_price_t`, etc.) — each topic document introduces the
relevant Binance mechanics before listing the C++ types.

## How the library is organized

The public API lives under `binapi2::fapi::`:

- `types/` — plain request and response structs (this guide's main subject).
- `rest/` — `pipeline` and service classes for REST endpoints.
- `websocket_api/` — authenticated WebSocket RPC client.
- `streams/` — `market_stream`, `user_stream`, `local_order_book`, etc.
- `transport/` — `http_client` and `websocket_client` built on Boost.Beast.

The top-level entry point is `binapi2::futures_usdm_api` — a factory for
connected REST, WS API, and stream clients. See
[`README.md`](../../README.md#minimal-example-async-coroutine) for usage.

## Related documents

- [../DESIGN.md](../DESIGN.md) — architecture, async model, dispatch.
- [../streams.md](../streams.md) — stream component reference.
- [../json_parsing.md](../json_parsing.md) — deserialization behaviour.
- [../data_types.md](../data_types.md) — full type catalog with Binance doc links.
- [../demo_cli_all_apis.md](../demo_cli_all_apis.md) — 135-command demo CLI reference.
