# 2. Market data

Market data types in binapi2 fall into seven categories: **order book**,
**trades**, **klines**, **tickers**, **mark price and funding**, **futures
analytics**, and **exchange metadata**. Each is available via one or more of
the three API surfaces (REST, WebSocket API, streams). This document covers
every market data type and explains when each surface is the right choice.

## Order book

The order book is the central limit order book for a symbol: all resting
limit orders, grouped into **bids** (buy orders) and **asks** (sell orders).
The highest bid and lowest ask form the **spread**. Orders are aggregated by
price level — you see the total quantity at each price, not individual
orders.

### REST: order book snapshot

```cpp
struct order_book_request_t
{
    symbol_t symbol{};             // required
    std::optional<int> limit{};    // 5, 10, 20, 50, 100, 500, 1000
};

struct order_book_response_t
{
    std::uint64_t lastUpdateId{};
    timestamp_ms_t E{};            // message output time
    timestamp_ms_t T{};            // transaction time
    std::vector<price_level_t> bids{};
    std::vector<price_level_t> asks{};
};

struct price_level_t
{
    decimal_t price{};
    decimal_t quantity{};
};
```

Returns one snapshot of the book. `lastUpdateId` is the monotonic sequence
number of the last update included — critical for synchronizing with the
diff-depth stream (below). `bids` are descending by price, `asks` ascending.

**Use for:** one-shot depth queries, initial snapshot for local order book
synchronization, pre-trade quote lookups.

### Stream: diff-depth updates

```cpp
struct diff_book_depth_subscription
{
    symbol_t symbol{};
    std::string speed{"100ms"};    // or "250ms" or "500ms"
};

struct depth_stream_event_t
{
    timestamp_ms_t event_time{};            // E
    timestamp_ms_t transaction_time{};      // T
    symbol_t symbol{};                      // s
    std::uint64_t first_update_id{};        // U
    std::uint64_t final_update_id{};        // u
    std::uint64_t prev_final_update_id{};   // pu — previous event's u
    std::vector<price_level_t> bids{};      // b
    std::vector<price_level_t> asks{};      // a
};
```

The stream pushes **deltas** — levels that changed since the previous event.
A level with `quantity == 0` means "remove this price". You maintain the full
book locally by applying each delta.

`first_update_id`, `final_update_id`, and `prev_final_update_id` let you
detect gaps. If `event.prev_final_update_id != last_applied_u`, you missed
an update — the book is out of sync and you must re-fetch a snapshot.

The `speed` parameter selects the push frequency: 100 ms (fastest), 250 ms,
or 500 ms.

### Stream: partial depth (fixed levels)

```cpp
struct partial_book_depth_subscription
{
    symbol_t symbol{};
    int levels{5};                 // 5, 10, or 20
    std::string speed{"100ms"};
};
```

Same event type (`depth_stream_event_t`) but each frame contains the **top N
levels** of the full book (not deltas). No reconstruction needed — just
replace your book snapshot on each event. Simpler but less efficient for deep
books.

### Synchronized local order book

Combining the REST snapshot and the diff-depth stream correctly requires a
careful sync algorithm:

1. Subscribe to the diff stream, buffer events.
2. Fetch a REST snapshot.
3. Discard buffered events where `final_update_id < snapshot.lastUpdateId`.
4. Verify the next buffered event brackets the snapshot (`first_update_id <=
   snapshot.lastUpdateId + 1 <= final_update_id`). If not, resync.
5. Apply remaining buffered events and continue live.
6. On any gap, restart from step 1.

binapi2 implements this as `order_book::local_order_book`:

```cpp
auto streams = api.create_market_stream();
auto rest = co_await api.create_rest_client();
order_book::local_order_book book(*streams, (*rest)->market_data);
book.set_snapshot_callback([](const auto& snap) { /* ... */ });
co_await book.async_run(symbol_t{"BTCUSDT"}, 1000);
```

A **pipelined** variant `order_book::pipeline_order_book` runs network,
parser, and logic in three dedicated threads connected via SPSC ring buffers
— use this when you need to keep the network thread unblocked by book logic.

### WS API: book ticker (best bid/ask only)

If you only need the **top of book** (best bid and ask), the WS API
book-ticker request is simpler than maintaining a full book:

```cpp
// Single symbol → single ticker
struct websocket_api_book_ticker_request_t { symbol_t symbol{}; };

// No symbol → array of tickers (all symbols)
struct websocket_api_book_tickers_request_t { };

struct book_ticker_t
{
    symbol_t symbol{};
    decimal_t bidPrice{};
    decimal_t bidQty{};
    decimal_t askPrice{};
    decimal_t askQty{};
    timestamp_ms_t time{};
    std::optional<std::uint64_t> lastUpdateId{};
};
```

Same `book_ticker_t` type is also available via REST (`book_ticker_request_t`)
and as a streaming event (`book_ticker_stream_event_t`). Three ways to get
the same data — choose based on latency needs.

---

## Trades

A **trade** is a completed match between a buyer and a seller. Each trade has
a price, quantity, timestamp, and direction. On Binance futures, individual
trades are available both as recent/historical snapshots (REST) and as a live
feed (stream).

### REST: recent and aggregate trades

```cpp
struct recent_trades_request_t
{
    symbol_t symbol{};
    std::optional<int> limit{};    // default 500, max 1000
};

struct recent_trade_t
{
    std::uint64_t id{};
    decimal_t price{};
    decimal_t qty{};
    decimal_t quoteQty{};          // price * qty
    timestamp_ms_t time{};
    bool isBuyerMaker{};
};
```

`isBuyerMaker` is Binance's signed-trade indicator: `true` means the buyer
was the resting order (market sell). Used to classify trades as "buy" or
"sell" aggression.

**Aggregate trades** merge consecutive trades at the same price from the same
taker side into one record — a cleaner tape for charting and analytics:

```cpp
struct aggregate_trade_t
{
    std::uint64_t a{};              // aggregate trade ID
    decimal_t p{};                  // price
    decimal_t q{};                  // quantity
    std::uint64_t f{};              // first trade ID
    std::uint64_t l{};              // last trade ID
    timestamp_ms_t T{};             // trade time
    bool m{};                       // was buyer the maker
};
```

Historical trades (`historical_trades_request_t`) is the same as recent
trades but accepts a `fromId` cursor to paginate backward.

### Stream: aggregate trade feed

```cpp
struct aggregate_trade_subscription { symbol_t symbol{}; };

struct aggregate_trade_stream_event_t
{
    timestamp_ms_t event_time{};            // E
    symbol_t symbol{};                      // s
    std::uint64_t aggregate_trade_id{};     // a
    decimal_t price{};                      // p
    decimal_t quantity{};                   // q
    std::uint64_t first_trade_id{};         // f
    std::uint64_t last_trade_id{};          // l
    timestamp_ms_t trade_time{};            // T
    bool is_buyer_maker{};                  // m
};
```

One event per aggregate trade — the live trade tape. For charting, trading
signals, and any "what's happening right now" logic.

**Duplication note:** REST returns a *historical list*, the stream pushes
*live events*. There is no individual-trade stream — aggregation is the only
live feed format.

---

## Klines (candlesticks)

A **kline** (Japanese: 蝋足, candlestick) summarizes price action over a time
interval: open, high, low, close, volume. Intervals range from 1 minute to
1 month.

### Kline intervals (`kline_interval_t`)

```
_1m, _3m, _5m, _15m, _30m, _1h, _2h, _4h, _6h, _8h, _12h, _1d, _3d, _1w, _1M
```

The underscore prefix avoids conflict with C++ integer literals. Wire format
is the Binance convention (`"1m"`, `"1h"`, etc.).

### REST: historical klines

```cpp
struct klines_request_t
{
    symbol_t symbol{};
    kline_interval_t interval{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};    // default 500, max 1500
};

struct kline_t
{
    timestamp_ms_t open_time{};
    decimal_t open{};
    decimal_t high{};
    decimal_t low{};
    decimal_t close{};
    decimal_t volume{};
    timestamp_ms_t close_time{};
    decimal_t quote_volume{};       // volume in quote asset
    std::uint64_t trades{};
    decimal_t taker_buy_volume{};
    decimal_t taker_buy_quote_volume{};
};
```

Returns an array of closed candles (plus the currently-forming one if the
time range includes "now"). The glaze deserializer reads klines from
Binance's **array format** (not object format):

```json
[[1566727587000,"0.29","0.29","0.28","0.28","2.54",1566728591999,"0.74",3,"2.54","0.74","0"]]
```

This is a binapi2 design choice — positional parsing avoids named field
overhead for this high-volume type.

**Variants:**

| Request | Data source | Notes |
|---------|-------------|-------|
| `klines_request_t` | The symbol's trade tape | Standard candles |
| `continuous_kline_request_t` | Pair + contract type (perpetual/current quarter/next quarter) | For delivery contracts — aggregates without the expiry rollover |
| `index_price_klines_request_t` | Underlying spot index | Tracks the index, not the futures contract |
| `mark_price_klines_request_t` | Mark price series | For margin and P&L analysis |
| `premium_index_klines_request_t` | Premium index (mark minus index) | The funding premium signal |

All five variants share the same response shape (`std::vector<kline_t>`).

### Stream: kline updates

```cpp
struct kline_subscription
{
    symbol_t symbol{};
    kline_interval_t interval{};
};

struct kline_stream_event_t
{
    timestamp_ms_t event_time{};    // E
    symbol_t symbol{};              // s
    struct kline_data_t {
        timestamp_ms_t start_time{};
        timestamp_ms_t close_time{};
        symbol_t symbol{};
        kline_interval_t interval{};
        std::uint64_t first_trade_id{};
        std::uint64_t last_trade_id{};
        decimal_t open{};
        decimal_t close{};
        decimal_t high{};
        decimal_t low{};
        decimal_t volume{};
        std::uint64_t trades{};
        bool is_closed{};
        decimal_t quote_volume{};
        decimal_t taker_buy_volume{};
        decimal_t taker_buy_quote_volume{};
    } kline{};
};
```

The stream pushes the **currently-forming kline** after every trade in that
interval, until the interval closes. When `kline.is_closed == true`, that
kline is final and a new one starts on the next event.

Typical usage: maintain a rolling deque of closed klines from REST history,
then replace the last element with every stream event (or append when
`is_closed` flips).

A **continuous contract kline stream** variant exists for delivery contracts,
keyed on `(pair, contract_type, interval)`.

---

## Tickers

A **ticker** is a compact summary of a symbol's recent market state.
Binance offers several ticker types with different field sets and update
strategies.

### Book ticker — best bid and ask

`book_ticker_t` (see order book section) is the top-of-book snapshot. Same
type on all three surfaces. The stream variant pushes on every book change
(no frequency cap):

```cpp
struct book_ticker_subscription { symbol_t symbol{}; };

struct book_ticker_stream_event_t
{
    std::uint64_t update_id{};              // u
    symbol_t symbol{};                      // s
    decimal_t best_bid_price{};             // b
    decimal_t best_bid_quantity{};          // B
    decimal_t best_ask_price{};             // a
    decimal_t best_ask_quantity{};          // A
    timestamp_ms_t transaction_time{};      // T
    timestamp_ms_t event_time{};            // E
};
```

### Price ticker — last trade price

```cpp
struct price_ticker_t
{
    symbol_t symbol{};
    decimal_t price{};
    timestamp_ms_t time{};
};
```

Just the last traded price. REST v1 (`price_ticker_request_t`) and v2
(`price_ticker_v2_request_t`) differ only in a minor endpoint path — use v2
for new code. Also available via WS API and as an all-symbols array.

### 24-hour ticker — rolling stats

```cpp
struct ticker_24hr_t
{
    symbol_t symbol{};
    decimal_t priceChange{};
    decimal_t priceChangePercent{};
    decimal_t weightedAvgPrice{};
    decimal_t lastPrice{};
    decimal_t lastQty{};
    decimal_t openPrice{};
    decimal_t highPrice{};
    decimal_t lowPrice{};
    decimal_t volume{};
    decimal_t quoteVolume{};
    timestamp_ms_t openTime{};
    timestamp_ms_t closeTime{};
    std::uint64_t firstId{};
    std::uint64_t lastId{};
    std::uint64_t count{};
};
```

The typical "market overview" data: open, high, low, close, volume, and
percentage change for the last 24 hours. Exchanges usually display this as
the ticker strip on a trading UI. Update frequency: ~1s via the stream.

### Mini ticker — compact variant

```cpp
struct mini_ticker_stream_event_t
{
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t close{};
    decimal_t open{};
    decimal_t high{};
    decimal_t low{};
    decimal_t base_volume{};
    decimal_t quote_volume{};
};
```

Same idea as the 24-hour ticker but without the derived fields (percent
change, weighted average). Smaller payload — useful when subscribing to
`stream-all-mini-tickers` for every symbol.

### All-market ticker streams

For any per-symbol stream (book ticker, mini ticker, 24hr ticker), there is
an **all-market** variant that pushes a single event containing an array for
every active symbol:

- `all_market_book_ticker_subscription` → `/ws/!bookTicker`
- `all_market_ticker_subscription` → `/ws/!ticker@arr`
- `all_market_mini_ticker_subscription` → `/ws/!miniTicker@arr`

The event type is a `std::vector<per_symbol_event_t>` wrapped in a top-level
container. Use this when you need market-wide data and don't want hundreds
of individual subscriptions.

---

## Mark price and funding rate

The **mark price** is the exchange's reference price for margin calculations.
Binance computes it as a weighted combination of the spot index price and
the moving average of the futures price, damped to prevent manipulative
liquidations.

The **funding rate** is the periodic payment between perpetual longs and
shorts. It's calculated from the premium of the futures price over the index
price (the "premium index") plus a fixed interest component. Paid every 8
hours by default. Positive rate → longs pay shorts.

### REST: mark price and funding

```cpp
struct mark_price_t
{
    symbol_t symbol{};
    decimal_t markPrice{};
    decimal_t indexPrice{};
    decimal_t estimatedSettlePrice{};
    decimal_t lastFundingRate{};        // most recent funding applied
    decimal_t interestRate{};
    timestamp_ms_t nextFundingTime{};
    timestamp_ms_t time{};
};
```

`mark_price_request_t` fetches one symbol, `mark_prices_request_t` fetches
all symbols at once.

`funding_rate_request_t` returns historical funding rates for one symbol
with optional time range. `funding_rate_info_request_t` returns the
**interest rate** and **funding rate cap/floor** for every symbol — what
you need to estimate future funding.

### Stream: mark price updates

```cpp
struct mark_price_subscription
{
    symbol_t symbol{};
    bool every_1s{false};       // false → every 3s, true → every 1s
};

struct mark_price_stream_event_t
{
    timestamp_ms_t event_time{};
    symbol_t symbol{};
    decimal_t mark_price{};
    decimal_t index_price{};
    decimal_t estimated_settle_price{};
    decimal_t funding_rate{};
    timestamp_ms_t next_funding_time{};
};
```

The all-market variant `all_market_mark_price_subscription` pushes a single
event with an array of `mark_price_stream_event_t`, one per active symbol.
Same `every_1s` option.

**Design note:** mark price is the only stream where the per-symbol and
all-market variants have the same event shape — useful for unified handlers.

---

## Futures analytics

Binance exposes a set of aggregated statistics useful for market analysis.
Only available on REST (no stream variant, no testnet).

| Request | Returns | What it measures |
|---------|---------|------------------|
| `open_interest_request_t` | `open_interest_t` | Total open positions (long + short) in one symbol |
| `open_interest_statistics_request_t` | historical OI array | Open interest time series (30min / 1h / 2h / 4h / 6h / 12h / 1d periods) |
| `top_long_short_account_ratio_request_t` | ratio time series | Long vs short **accounts** among top traders |
| `top_trader_long_short_ratio_request_t` | ratio time series | Long vs short **positions** among top traders |
| `long_short_ratio_request_t` | ratio time series | Long/short ratio across all accounts |
| `taker_buy_sell_volume_request_t` | volume time series | Taker buy vs sell volume (aggression) |
| `basis_request_t` | basis time series | Futures premium / discount to spot |

These are important sentiment indicators for traders but are pull-only —
typically fetched every minute or every period. None are available on
testnet (the endpoints return plain `ok` text instead of JSON).

---

## Exchange metadata

### Exchange info

```cpp
struct exchange_info_request_t { };    // no parameters

struct exchange_info_response_t
{
    std::string timezone{};
    timestamp_ms_t serverTime{};
    std::vector<rate_limit_t> rateLimits{};
    std::vector<asset_info_t> assets{};
    std::vector<symbol_info_t> symbols{};
};

struct symbol_info_t
{
    symbol_t symbol{};
    pair_t pair{};
    contract_type_t contractType{};
    timestamp_ms_t deliveryDate{};     // 0 for perpetuals
    timestamp_ms_t onboardDate{};
    contract_status_t status{};
    decimal_t maintMarginPercent{};
    decimal_t requiredMarginPercent{};
    std::string baseAsset{};
    std::string quoteAsset{};
    std::string marginAsset{};
    int pricePrecision{};
    int quantityPrecision{};
    int baseAssetPrecision{};
    int quotePrecision{};
    std::vector<symbol_filter_t> filters{};
    // ... plus order types, time-in-force, trigger protect, liquidation fee
};
```

`exchange_info_response_t` is the one-stop reference: all tradeable symbols,
their precision, their filters (price tick size, quantity step size, min
notional, max leverage), rate limits, and the server clock.

**You should call this at startup** and cache the result. It gives you the
rules you must follow when building order requests (min/max price, quantity
step, min notional value). Sending an order that violates a filter is
rejected with a `-4xxx` Binance error code.

`symbol_filter_t` is a `std::variant` over all Binance filter types
(`PRICE_FILTER`, `LOT_SIZE`, `MARKET_LOT_SIZE`, `MAX_NUM_ORDERS`,
`MAX_NUM_ALGO_ORDERS`, `MIN_NOTIONAL`, `PERCENT_PRICE`) — unwrap with
`std::visit` or `std::get_if`.

### Server time

```cpp
struct server_time_request_t { };
struct server_time_response_t { timestamp_ms_t serverTime{}; };
```

Use at startup to estimate clock skew for signing (Binance rejects requests
whose `timestamp + recvWindow < serverTime`).

### ADL risk

Auto-deleveraging (ADL) is the mechanism by which winning counterparty
positions are force-closed to cover losses when the insurance fund is
depleted. `adl_risk_request_t` returns the ADL queue indicator (0–4) for a
symbol — useful for assessing your own ADL exposure.

### Other metadata endpoints

| Request | Purpose |
|---------|---------|
| `composite_index_info_request_t` | Index constituents and weights |
| `index_constituents_request_t` | Which symbols make up an index |
| `asset_index_request_t` | Multi-asset margin asset index values |
| `insurance_fund_request_t` / `insurance_funds_request_t` | Current insurance fund balance (single symbol or all) |
| `delivery_price_request_t` | Settlement price history for expired delivery contracts |
| `trading_schedule_request_t` | Trading sessions / holidays |
| `rpi_depth_request_t` | Retail Price Improvement order book variant |

---

## What you can do with market data

This section is a quick catalog of what each data type enables. Pick the
type(s) you need for your use case.

### Live price tracking
- **Streams:** book ticker, price ticker, or 24hr ticker per symbol.
- **Derived:** maintain a symbol→last-price map updated from the stream,
  use for position valuation and risk calculations.

### Local order book
- **REST + Stream:** `order_book_request_t` snapshot + diff depth stream.
- **binapi2:** `local_order_book::async_run()` or `pipeline_order_book`.

### Trade signals from volume
- **Streams:** aggregate trade stream → detect bursts, compute VWAP, classify
  aggressor direction via `is_buyer_maker`.
- **Historical:** `recent_trades_request_t` or `aggregate_trades_request_t`
  for backfill.

### Charting and technical analysis
- **REST:** `klines_request_t` for historical backfill (up to 1500 candles
  per call; paginate via `startTime`/`endTime` for longer series).
- **Stream:** kline stream to update the current forming candle.

### Funding awareness and basis trading
- **REST:** `mark_price_request_t` for current mark/funding, funding rate
  history via `funding_rate_request_t`.
- **Stream:** mark price stream to track premium in real time.

### Market sentiment / positioning
- **REST only:** long/short ratios, top trader ratios, taker volume, basis,
  open interest statistics. Poll on a minute timer.

### Symbol discovery and validation
- **REST:** `exchange_info_request_t` at startup. Validate every order
  against its symbol's filters before sending.

---

## Summary table: market data by surface

| Data type | REST request | WS API request | Stream subscription | Response / event type |
|-----------|--------------|----------------|---------------------|------------------------|
| Order book snapshot | `order_book_request_t` | — | — | `order_book_response_t` |
| Order book deltas | — | — | `diff_book_depth_subscription` | `depth_stream_event_t` |
| Partial depth | — | — | `partial_book_depth_subscription` | `depth_stream_event_t` |
| Book ticker | `book_ticker_request_t` / `book_tickers_request_t` | `websocket_api_book_ticker_request_t` / `_tickers_` | `book_ticker_subscription` / `all_market_book_ticker_subscription` | `book_ticker_t` / `book_ticker_stream_event_t` |
| Price ticker | `price_ticker_request_t` / `price_ticker_v2_request_t` | `websocket_api_price_ticker_request_t` / `_tickers_` | — | `price_ticker_t` |
| 24hr ticker | `ticker_24hr_request_t` / `ticker_24hrs_request_t` | — | `ticker_subscription` / `all_market_ticker_subscription` | `ticker_24hr_t` / `ticker_stream_event_t` |
| Mini ticker | — | — | `mini_ticker_subscription` / `all_market_mini_ticker_subscription` | `mini_ticker_stream_event_t` |
| Recent trades | `recent_trades_request_t` | — | — | `std::vector<recent_trade_t>` |
| Historical trades | `historical_trades_request_t` | — | — | `std::vector<recent_trade_t>` |
| Aggregate trades | `aggregate_trades_request_t` | — | `aggregate_trade_subscription` | `std::vector<aggregate_trade_t>` / `aggregate_trade_stream_event_t` |
| Klines | `klines_request_t` + 4 variants | — | `kline_subscription` / `continuous_contract_kline_subscription` | `std::vector<kline_t>` / `kline_stream_event_t` |
| Mark price | `mark_price_request_t` / `mark_prices_request_t` | — | `mark_price_subscription` / `all_market_mark_price_subscription` | `mark_price_t` / `mark_price_stream_event_t` |
| Funding rate history | `funding_rate_request_t` | — | — | `std::vector<funding_rate_t>` |
| Funding rate info | `funding_rate_info_request_t` | — | — | `std::vector<funding_rate_info_t>` |
| Open interest | `open_interest_request_t` | — | — | `open_interest_t` |
| OI statistics | `open_interest_statistics_request_t` | — | — | `std::vector<open_interest_statistics_entry_t>` |
| Long/short ratios | `long_short_ratio_request_t` and siblings | — | — | arrays of ratio entries |
| Taker buy/sell volume | `taker_buy_sell_volume_request_t` | — | — | array |
| Basis | `basis_request_t` | — | — | array |
| Exchange info | `exchange_info_request_t` | — | — | `exchange_info_response_t` |
| Server time | `server_time_request_t` | — | — | `server_time_response_t` |
| Liquidation orders | — | — | `liquidation_order_subscription` / `all_market_liquidation_order_subscription` | `liquidation_order_stream_event_t` |
| Contract info events | — | — | `contract_info_subscription` | `contract_info_stream_event_t` |
| Composite index | — | — | `composite_index_subscription` | `composite_index_stream_event_t` |
| Asset index | `asset_index_request_t` | — | `asset_index_subscription` / `all_asset_index_subscription` | `asset_index_t` / `asset_index_stream_event_t` |
| Trading session events | — | — | `trading_session_subscription` | `trading_session_stream_event_t` |
| ADL risk | `adl_risk_request_t` | — | — | array |
| Insurance fund | `insurance_fund_request_t` / `insurance_funds_request_t` | — | — | `insurance_fund_response_t` |
| Delivery price | `delivery_price_request_t` | — | — | array |
| RPI depth | `rpi_depth_request_t` | — | — | order-book-like |
| Trading schedule | `trading_schedule_request_t` | — | — | array |
