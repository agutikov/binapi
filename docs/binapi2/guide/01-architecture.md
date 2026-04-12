# 1. Architecture: the three API surfaces

Binance USD-M Futures exposes the same functional data (prices, orders,
balances) through **three separate network interfaces**, each with different
latency, authentication, and message shape characteristics. Understanding
which surface to use for which purpose is the key to building a responsive,
correct client.

## The three surfaces

| Surface | Protocol | Pattern | Latency | Auth model |
|---------|----------|---------|---------|------------|
| **REST** | HTTPS request/response | Pull, one-shot | Medium (100–500 ms) | API key header + signed query per request |
| **WebSocket API** | WSS request/response | Pull over persistent connection | Low (2–10 ms) | Session logon (Ed25519) or per-request signature |
| **Streams** | WSS push | Push-only, subscribed events | Lowest (tens of ms after exchange event) | None for market data / listen key for user data |

### REST

The classic request/response pattern. Each call opens a fresh TLS connection
(binapi2 does not pool), sends an HTTPS GET/POST/PUT/DELETE, and returns once
the server replies.

**Strengths:**
- Simple, stateless. Good for batch queries, historical data, one-off reads.
- Supports large responses (exchange info, kline history, account snapshots).
- The only surface for some endpoints (convert, portfolio margin, download IDs).

**Weaknesses:**
- TLS handshake overhead on every call. Binance publishes rate limits that
  penalize rapid repeated requests.
- Polling for live data is inefficient — use streams instead.

**In binapi2:** `create_rest_client() → rest::client` exposes five services
(`market_data`, `account`, `trade`, `convert`, `user_data_streams`). Each
service has `async_execute<Request>(req)` constrained by a concept so only
its own request types compile.

### WebSocket API (WS API)

A JSON-RPC style persistent WebSocket. Connect once, log on once (Ed25519
signature), then send any number of requests. Each request carries an `id`,
the server replies with the same `id`. Responses are typed identically to
REST responses for the same operation.

**Strengths:**
- No per-request TLS handshake — sub-10 ms round trips on well-located clients.
- Session logon means subsequent trade operations don't need to re-sign every
  query; the session is authenticated.
- Identical response types to REST for trading — drop-in low-latency replacement.

**Weaknesses:**
- Not every REST endpoint has a WS API method (e.g. no kline history, no
  exchange info, no convert).
- Connection lifecycle to manage: connect, logon, ping/keepalive, close.
- Ed25519 key is mandatory for session logon (HMAC is rejected).

**In binapi2:** `create_ws_api_client() → websocket_api::client`. Use
`async_connect()` then `async_session_logon()`, then `async_execute<Request>`.
Four auth modes exist for different request categories
(see [DESIGN.md](../DESIGN.md#websocket-api)).

### Streams

Push-only subscriptions. Connect to a URL like `/ws/btcusdt@bookTicker` and
the server pushes JSON frames as events occur. No request/response — just a
one-way event feed.

**Market streams** are unauthenticated and cover all public market data.
**User data streams** are authenticated via a **listen key** (a short-lived
token obtained from REST or WS API) and deliver account-specific events:
order fills, balance changes, margin calls, algo order updates.

**Strengths:**
- Lowest latency — events arrive within tens of milliseconds of happening on
  the exchange matching engine.
- No polling overhead. Your code reacts to events as they happen.
- User streams are the only way to get real-time account updates — REST would
  require brutal polling.

**Weaknesses:**
- Push model complicates error recovery. Gap detection (for order books) and
  reconnection logic must be handled.
- User streams require a listen key kept alive with REST keepalives every
  30 minutes.
- Not every data type has a stream variant.

**In binapi2:** `create_market_stream() → streams::market_stream` (plus
`combined_market_stream`, `dynamic_market_stream`, `user_stream`,
`local_order_book`). The typed `subscribe(sub)` returns a
`cobalt::generator<result<Event>>` that yields one event per network frame.

## What binds the three surfaces: identical types

Binance designed the WS API responses to mirror REST responses. binapi2 goes
further — wherever possible, both surfaces deserialize into the **exact same
C++ struct**. This means you can write code against `order_response_t` once
and switch between REST and WS API without changing your response handling.

The duplication is intentional and covered per-topic in the remaining
documents. Example:

```cpp
// REST order placement
auto r = co_await (*rest)->trade.async_execute(new_order_request_t{
    .symbol = "BTCUSDT", .side = order_side_t::buy, .type = order_type_t::limit,
    .quantity = decimal_t{"0.01"}, .price = decimal_t{"60000"},
    .timeInForce = time_in_force_t::gtc,
});
// r is result<order_response_t>

// WS API order placement (same response type)
auto w = co_await (*ws)->async_execute(websocket_api_order_place_request_t{
    .symbol = "BTCUSDT", .side = order_side_t::buy, .type = order_type_t::limit,
    .quantity = decimal_t{"0.01"}, .price = decimal_t{"60000"},
    .timeInForce = time_in_force_t::gtc,
});
// w is result<websocket_api_response_t<order_response_t>>
//                                      ^^^^^^^^^^^^^^^^ same type
```

## When to use which surface

### Use REST when
- You need historical data (klines, trade history, funding rate history,
  income history).
- The data is a snapshot, not a live feed (exchange info, leverage brackets,
  commission rates).
- The endpoint only exists on REST (convert, download IDs for CSV exports,
  position margin adjustments, margin type changes, BNB burn toggle).
- You're implementing a bot that only needs polling every few seconds.

### Use the WebSocket API when
- You are placing, modifying, or cancelling orders and latency matters.
- You are querying orders or positions repeatedly (e.g. from a risk engine).
- You want to avoid per-request TLS handshake cost.
- You already have a persistent WebSocket connection for streams and can
  multiplex trading on it.

### Use streams when
- You need real-time market data (order book, trades, klines, tickers).
- You need real-time account events (fills, balance changes, liquidations).
- You're building anything that reacts to market or account events.
- You maintain a local order book synchronized from the exchange.

### Combining them (typical pattern)

A realistic trading bot uses **all three** simultaneously:

```
Streams              — live order book + real-time fills
    |
    v
Local state (book, position, balance)
    |
    v
Strategy decision
    |
    v
WebSocket API        — place/cancel orders with low latency
    |
REST                 — initial snapshots, historical backfill,
                       periodic account reconciliation, exchange info
```

## Duplication matrix

This table shows where the same semantic data is reachable via multiple
surfaces. Subsequent documents explain the shape and usage of each.

| Data | REST | WS API | Stream | Notes |
|------|:---:|:---:|:---:|-------|
| Order book snapshot | ✓ | | | REST one-shot |
| Order book updates | | | ✓ | Diff depth stream (deltas) |
| Book ticker (best bid/ask) | ✓ | ✓ | ✓ | Three ways to read it |
| Price ticker (last price) | ✓ | ✓ | ✓ | |
| 24hr ticker statistics | ✓ | | ✓ | |
| Klines (current) | ✓ | | ✓ | Stream pushes updates to *current* kline; REST returns history |
| Klines (historical) | ✓ | | | Only REST |
| Mark price | ✓ | | ✓ | Mark price + funding combined in both |
| Funding rate history | ✓ | | | Only REST |
| Recent trades | ✓ | | ✓ | Aggregate trade stream is the live feed |
| Exchange info | ✓ | | | Only REST |
| Account info / balances | ✓ | ✓ | ✓ | Stream: push on change via user stream |
| Positions | ✓ | ✓ | ✓ | Stream: push via account update event |
| Place order | ✓ | ✓ | | WS API ≈ 50 % lower RTT |
| Query order | ✓ | ✓ | | |
| Cancel order | ✓ | ✓ | | |
| Modify order | ✓ | ✓ | | |
| Order status changes | ✓ | ✓ | ✓ | Stream: push on every transition |
| Trade/fill events | ✓ | | ✓ | Stream: push on every fill |
| Leverage brackets | ✓ | | | |
| Change leverage | ✓ | | | |
| Change margin type | ✓ | | | |
| Position mode | ✓ | | | |
| Listen key lifecycle | ✓ | ✓ | | |
| Algo orders | ✓ | ✓ | ✓ | Stream: live algo updates |
| Liquidations | ✓ | | ✓ | ADL risk REST + liquidation stream |
| Convert | ✓ | | | REST only |

## Authentication summary

| Surface | Credential | Added where |
|---------|-----------|-------------|
| REST public (market data) | none | — |
| REST signed (account / trade) | `api_key` + signed query | `X-MBX-APIKEY` header + `signature=` query parameter |
| WS API market data | none | — |
| WS API signed base (account status/balance, listen key lifecycle) | `api_key` + signed query | Inside the JSON-RPC `params` object |
| WS API user-injected (order place/query/cancel) | `api_key` + signed query | Merged with request params, full object signed |
| WS API api-key only (user data stream mgmt over WS) | `api_key` | Inside `params.apiKey` |
| WS API session logon | Ed25519 signature | Once per connection; subsequent calls don't re-sign |
| Market streams | none | — |
| User data stream | listen key in URL | `/ws/<listenKey>` |

The signing method (Ed25519 vs HMAC-SHA256) is a single config choice:
`config::sign_method` defaults to Ed25519. WS API session logon **requires**
Ed25519 — HMAC is only accepted for REST signing and remains available for
legacy credentials.

See [`secret_provider.md`](../secret_provider.md) for credential storage
options (libsecret, systemd-creds, env, test provider).
