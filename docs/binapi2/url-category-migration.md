# WebSocket URL Category Migration (2026)

In 2026 Binance split the USD-M Futures WebSocket URL hierarchy into three
category-routed paths:

| Category   | Base URL                                | Streams                                                                 |
|------------|-----------------------------------------|-------------------------------------------------------------------------|
| `/public`  | `wss://fstream.binance.com/public`      | `bookTicker`, `!bookTicker`, partial/diff depth, RPI diff depth         |
| `/market`  | `wss://fstream.binance.com/market`      | `aggTrade`, `markPrice`, `kline`, `ticker`, `miniTicker`, `forceOrder`, `compositeIndex`, `assetIndex`, `contractInfo`, `tradingSession`, continuous kline, all `!*@arr` aggregate variants of the above |
| `/private` | `wss://fstream.binance.com/private`     | User data (listenKey-based)                                             |

**A combined-stream connection can only carry streams from one category.**
Mixing produces a silent failure mode — the WebSocket handshake succeeds,
SUBSCRIBE returns OK, but Binance never delivers frames for streams in
the "wrong" category. There is no error, no close, just zero frames.

Reference: Binance's
`docs/derivatives/usds-margined-futures/websocket-market-streams/Important-WebSocket-Change-Notice.md`.

## binapi2 API

### `stream_category_e`

Defined in `binapi2/fapi/stream_category.hpp`:

```cpp
enum class stream_category_e {
    public_,   // bookTicker, depth
    market,    // aggTrade, markPrice, kline, ticker, ...
    private_,  // user data (listenKey-based)
};

constexpr std::string_view category_single_target(stream_category_e);   // "/public/ws"
constexpr std::string_view category_combined_target(stream_category_e); // "/public/stream"
```

### Per-stream-type categorization

Every `stream_traits<T>` carries a `static constexpr stream_category_e category`:

```cpp
template<> struct stream_traits<types::aggregate_trade_subscription> {
    static constexpr stream_category_e category = stream_category_e::market;
    // ...
};
```

`stream_traits<T>::target(cfg, sub)` derives the URL from the category, not
from `cfg.stream_base_target`. Single-stream `market_stream::subscribe(sub)`
auto-routes to the right URL with no caller intervention.

### Combined-stream connections

`basic_dynamic_market_stream` and `basic_combined_market_stream` accept a
`stream_category_e` at construction:

```cpp
streams::dynamic_market_stream pub(cfg, stream_category_e::public_);
streams::dynamic_market_stream mkt(cfg, stream_category_e::market);

co_await pub.async_connect();   // → wss://.../public/stream
co_await mkt.async_connect();   // → wss://.../market/stream

co_await pub.async_subscribe(book_ticker_subscription{...},
                             partial_book_depth_subscription{...});
co_await mkt.async_subscribe(aggregate_trade_subscription{...},
                             mark_price_subscription{...},
                             kline_subscription{...});

// Compile error — same_category concept rejects mixing:
co_await pub.async_subscribe(book_ticker_subscription{...},
                             aggregate_trade_subscription{...});
//                           ^ mark_price_subscription is /market, expected /public
```

The `same_category` concept is enforced at compile time on
`async_subscribe` / `async_unsubscribe`. A runtime check additionally
catches mismatch between subscription category and the stream's
construction-time category.

## Recipe: split a single-connection multi-category subscriber

If your code currently does this (legacy single-arg ctor — no longer compiles):

```cpp
// Pre-2026 — no category, single bare /stream URL. REMOVED.
streams::dynamic_market_stream dyn(cfg);
co_await dyn.async_connect();
for (auto const& sym : symbols) {
    co_await dyn.async_subscribe(
        types::book_ticker_subscription{.symbol = sym},      // /public
        types::aggregate_trade_subscription{.symbol = sym},  // /market
        types::mark_price_subscription{.symbol = sym, .every_1s = true},
        types::kline_subscription{.symbol = sym, .interval = ...});
}
```

Replace with two parallel connections, one per category:

```cpp
streams::dynamic_market_stream pub(cfg, stream_category_e::public_);
streams::dynamic_market_stream mkt(cfg, stream_category_e::market);
co_await pub.async_connect();
co_await mkt.async_connect();
for (auto const& sym : symbols) {
    co_await pub.async_subscribe(
        types::book_ticker_subscription{.symbol = sym});
    co_await mkt.async_subscribe(
        types::aggregate_trade_subscription{.symbol = sym},
        types::mark_price_subscription{.symbol = sym, .every_1s = true},
        types::kline_subscription{.symbol = sym, .interval = ...});
}
```

Each connection has its own reconnect lifecycle, IP-steering, and watchdog.

## Legacy fields

`config::stream_base_target` and `config::combined_stream_target` are no
longer consumed by the streaming primitives. They remain in the struct
for ABI compatibility with pre-2026 consumers. They will be marked
`[[deprecated]]` once internal examples have been migrated to the
category-aware constructors.

## Testnet

`fstream.binancefuture.com` (testnet) accepts both the legacy `/ws` and
the new `/public`/`/market` paths during the transition. The category-
aware code paths described above work against testnet without changes.
