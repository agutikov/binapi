# 3. Trading and orders

Trading on Binance USD-M Futures means submitting orders, letting the
matching engine execute them against the order book, and tracking their
lifecycle. This document explains the order types, their state machine, and
how to place/modify/cancel/query them via REST or WebSocket API — both
surfaces share the same response types.

## Crypto futures order concepts

### What an order represents

An **order** is your instruction to buy or sell a quantity of a symbol,
subject to constraints (price, type, time-in-force). Once placed, the order
lives in one of several states until it is fully filled, fully cancelled, or
expired.

Binance USD-M orders are **contract orders**: each one takes or closes a
position of `quantity` contracts at the specified or matched price. One
contract on most symbols equals one unit of the base asset (e.g. 1 BTC for
`BTCUSDT`); delivery contracts and some coin-margined symbols have fixed
USD notional per contract.

### Order side

```cpp
enum class order_side_t { buy = 0, sell = 1 };
```

- **`buy`** — open or increase a long position (or close/decrease a short).
- **`sell`** — open or increase a short position (or close/decrease a long).

The same side field is used whether you're opening or closing. In **one-way
position mode**, the effect depends on your current net position. In **hedge
mode**, you additionally specify `positionSide` (`long`, `short`, `both`).

### Order type

```cpp
enum class order_type_t {
    limit, market, stop, stop_market, take_profit, take_profit_market,
    trailing_stop_market,
};
```

| Type | How it matches | Required fields |
|------|----------------|-----------------|
| `market` | Immediately at best available price until `quantity` filled | `quantity` |
| `limit` | Rests on the book at `price`; only fills at `price` or better | `quantity`, `price`, `timeInForce` |
| `stop` | Triggered when mark/last crosses `stopPrice`, then becomes a limit order at `price` | `quantity`, `price`, `stopPrice`, `timeInForce`, `workingType` |
| `stop_market` | Triggered when price crosses `stopPrice`, then becomes a market order | `quantity`, `stopPrice`, `workingType` |
| `take_profit` | Triggered when price crosses `stopPrice`, becomes a limit order | same as `stop` |
| `take_profit_market` | Triggered, becomes market | same as `stop_market` |
| `trailing_stop_market` | Follows the price by `callbackRate` %, triggers when the price retraces | `quantity`, `callbackRate`, optional `activationPrice` |

**Stop vs take-profit semantics** are about **direction of trigger relative
to current price**:
- `stop` and `stop_market` are triggered when price moves **against** the
  stated direction (long stop: price drops below `stopPrice`; short stop:
  price rises above).
- `take_profit` and `take_profit_market` are triggered when price moves
  **in** the stated direction.

Binance's matching engine doesn't enforce this — your client must choose the
right type based on intent.

### Time in force

```cpp
enum class time_in_force_t {
    gtc,    // Good Till Cancel — rests until filled or cancelled
    ioc,    // Immediate Or Cancel — fill what you can now, cancel the rest
    fok,    // Fill Or Kill — fill completely now or cancel entirely
    gtx,    // Good Till Crossing — post-only, rejected if it would cross the book
    gtd,    // Good Till Date — expires at `goodTillDate` timestamp
};
```

`gtx` (also known as "post-only" on other exchanges) guarantees you pay
the maker fee, not the taker fee. If your `price` would cross the book at
submit time, the order is rejected. Used heavily by market makers.

### Working type (trigger price source)

```cpp
enum class working_type_t { mark_price, contract_price };
```

For stop and take-profit orders, specifies which price feed triggers them:
- `mark_price` — triggered on the exchange-computed mark price (smoother,
  less vulnerable to wicks).
- `contract_price` — triggered on the last trade price.

**Use `mark_price` for stop losses** to avoid liquidation-style wicks
triggering your stop on a thin book.

### Reduce-only and close-position

- `reduceOnly: true` — the order can only reduce an existing position, never
  increase it or open a new one. Safer for stop losses and take profits.
- `closePosition: true` — only valid for `stop_market` and
  `take_profit_market`. Closes the **entire** position when triggered,
  ignoring the `quantity` field. Often combined with `stopPrice` to act as
  a classic "emergency stop".

### Self-trade prevention (STP)

```cpp
enum class stp_mode_t {
    none,           // No STP applied
    expire_taker,   // Cancel taker side on self-match
    expire_maker,   // Cancel maker side on self-match
    expire_both,    // Cancel both sides
};
```

Prevents your own orders from matching against each other (which would
produce a wash trade). Optional per order; defaults to `none`.

### Price match

```cpp
enum class price_match_t {
    none,
    opponent, opponent_5, opponent_10, opponent_20,
    queue, queue_5, queue_10, queue_20,
};
```

Binance's automatic price-following. `opponent` means "match the best
opposite-side price"; `opponent_5` means "five ticks beyond the best opposite
price"; `queue` means "match the front of the same-side queue". Useful for
algos that want to stay at a specific position relative to the spread
without manually repricing.

## Order state machine

```cpp
enum class order_status_t {
    new_order,              // accepted, resting on book
    partially_filled,       // some fills but not complete
    filled,                 // fully executed
    canceled,               // user-cancelled
    rejected,               // rejected at submit (validation failure)
    expired,                // time-in-force or trigger expired
    new_adl,                // ADL is about to close this
    new_insurance,          // insurance fund is about to close this
    expired_in_match,       // cancelled due to self-trade prevention
};
```

Every order transitions through these states. REST/WS API queries return the
current state. The **user data stream** (`order_trade_update_event_t`) pushes
an event for *every* transition and every partial fill — the canonical way
to track order lifecycle in real time.

## REST API: order requests and responses

### Place a new order

```cpp
struct new_order_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    order_type_t type{};
    std::optional<time_in_force_t> timeInForce{};
    std::optional<decimal_t> quantity{};
    std::optional<bool> reduceOnly{};              // "true"/"false" string on wire
    std::optional<decimal_t> price{};
    std::optional<std::string> newClientOrderId{};
    std::optional<decimal_t> stopPrice{};
    std::optional<bool> closePosition{};           // "true"/"false" string on wire
    std::optional<decimal_t> activationPrice{};
    std::optional<decimal_t> callbackRate{};
    std::optional<working_type_t> workingType{};
    std::optional<bool> priceProtect{};
    std::optional<response_type_t> newOrderRespType{};
    std::optional<price_match_t> priceMatch{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
    std::optional<std::string> recvWindow{};
};
```

Every field is optional except `symbol`, `side`, and `type`. The specific
combination required depends on `type` (see the order-type table above).
`newClientOrderId` is a client-supplied unique ID — useful for idempotency
and for correlating your own request with the response. If omitted, Binance
generates one.

### The unified order response

```cpp
struct order_response_t
{
    std::string clientOrderId{};
    std::optional<decimal_t> cumQty{};         // cumulative filled quantity
    decimal_t cumQuote{};                      // cumulative quote-asset filled
    decimal_t executedQty{};                   // same as cumQty (legacy alias)
    std::optional<decimal_t> cumBase{};
    std::uint64_t orderId{};
    decimal_t avgPrice{};
    decimal_t origQty{};                       // original order quantity
    std::optional<pair_t> pair{};
    decimal_t price{};
    bool reduceOnly{};
    order_side_t side{};
    position_side_t positionSide{};
    order_status_t status{};
    decimal_t stopPrice{};
    bool closePosition{};
    symbol_t symbol{};
    time_in_force_t timeInForce{};
    order_type_t type{};
    std::optional<working_type_t> workingType{};
    std::optional<bool> priceProtect{};
    order_type_t origType{};
    std::optional<price_match_t> priceMatch{};
    std::optional<stp_mode_t> selfTradePreventionMode{};
    std::optional<timestamp_ms_t> goodTillDate{};
    std::optional<decimal_t> activatePrice{};   // trailing stop
    std::optional<decimal_t> priceRate{};        // trailing callback
    std::optional<timestamp_ms_t> time{};
    std::optional<timestamp_ms_t> updateTime{};
};
```

This is the **canonical order representation** in binapi2. It is returned by
every order-related operation:

| Operation | Returns |
|-----------|---------|
| New order (`new_order_request_t`) | `order_response_t` |
| Test order (`test_order_request_t`) | `order_response_t` — validation only, not placed |
| Query order (`query_order_request_t`) | `order_response_t` |
| Query open order (`query_open_order_request_t`) | `order_response_t` |
| Modify order (`modify_order_request_t`) | `order_response_t` |
| Cancel order (`cancel_order_request_t`) | `order_response_t` |
| Open orders (`open_orders_request_t`) | `std::vector<order_response_t>` |
| All orders (`all_orders_request_t`) | `std::vector<order_response_t>` |
| Force orders (`force_orders_request_t`) | `std::vector<order_response_t>` — liquidation history |

**Field notes:**
- `cumQty` and `executedQty` both represent the cumulative filled quantity.
  Binance inconsistently returns one or the other; `cumQty` is optional
  because Binance's force-orders endpoint omits it entirely. Always prefer
  `executedQty` in application code.
- `cumQuote` is the total quote-asset amount filled so far. For a LIMIT BUY
  of 0.1 BTC at $60,000 that fills fully: `cumQty=0.1`, `cumQuote=6000`.
- `avgPrice` is the volume-weighted average fill price.
- `origType` is populated after a stop/take-profit order triggers — it
  holds the original conditional type (e.g. `stop_market`), while `type`
  transitions to the executed type (e.g. `market`).
- `priceProtect` enables price protection on stop/take-profit triggers,
  rejecting a trigger if the mark price moves too far in one tick.
- `time` is the order creation timestamp, `updateTime` is the last state
  change timestamp.

### Modify an existing order

```cpp
struct modify_order_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
    decimal_t quantity{};
    decimal_t price{};
    std::optional<price_match_t> priceMatch{};
    std::optional<std::string> recvWindow{};
};
```

Only **LIMIT orders** can be modified. You can change `quantity` and/or
`price`. Internally Binance cancels and re-creates the order, but it
preserves the order ID. Useful for market makers that re-quote frequently.

Identify the order by either `orderId` or `origClientOrderId`.

### Cancel orders

```cpp
struct cancel_order_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct cancel_multiple_orders_request_t
{
    symbol_t symbol{};
    std::optional<std::string> orderIdList{};                 // comma-separated
    std::optional<std::string> origClientOrderIdList{};
};

struct cancel_all_open_orders_request_t { symbol_t symbol{}; };
```

`cancel_all_open_orders_request_t` is the "panic button" — cancels every
open order for one symbol.

### Auto-cancel (deadman's switch)

```cpp
struct auto_cancel_request_t
{
    symbol_t symbol{};
    std::uint64_t countdownTime{};
};
```

Sets a server-side timer: if you don't send another `auto_cancel_request_t`
within `countdownTime` milliseconds, the exchange cancels all your open
orders for that symbol. Essential for trading bots that must not leave
orders stranded if the bot crashes or loses connectivity.

Set `countdownTime = 0` to cancel the timer.

### Query orders

```cpp
struct query_order_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct open_orders_request_t { std::optional<symbol_t> symbol{}; };
struct all_orders_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};
```

- `query_order` — one specific order by ID.
- `open_orders` — all currently-open orders. Omit symbol for every symbol.
- `all_orders` — full order history (including filled and cancelled).
  Paginated via `startTime`/`endTime`/`orderId` cursor.

### Account trades

```cpp
struct account_trade_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<std::uint64_t> fromId{};
    std::optional<int> limit{};
};

struct account_trade_t
{
    symbol_t symbol{};
    std::uint64_t id{};
    std::uint64_t orderId{};
    decimal_t price{};
    decimal_t qty{};
    decimal_t quoteQty{};
    decimal_t commission{};
    std::string commissionAsset{};
    timestamp_ms_t time{};
    order_side_t side{};
    position_side_t positionSide{};
    bool buyer{};                  // you were the buyer
    bool maker{};                  // you were the maker (vs taker)
    decimal_t realizedPnl{};
};
```

Individual fills, not orders. One order can produce many `account_trade_t`
records (one per partial fill). Fields track commission paid and realized
P&L at fill time. Use for accurate accounting and tax/reporting.

## WebSocket API: the low-latency path

The WS API exposes the same trade operations with identical response types:

```cpp
// include/binapi2/fapi/types/websocket_api.hpp (abbreviated)
struct websocket_api_order_place_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    order_type_t type{};
    std::optional<time_in_force_t> timeInForce{};
    std::optional<decimal_t> quantity{};
    // ... identical field set to new_order_request_t
};

struct websocket_api_order_query_request_t : websocket_api_signed_request_t
{
    symbol_t symbol{};
    std::optional<std::uint64_t> orderId{};
    std::optional<std::string> origClientOrderId{};
};

struct websocket_api_order_cancel_request_t : websocket_api_signed_request_t
{ /* same as REST */ };

struct websocket_api_order_modify_request_t : websocket_api_signed_request_t
{ /* same as REST */ };
```

Every WS API trade request inherits from `websocket_api_signed_request_t`
which adds the per-request auth fields:

```cpp
struct websocket_api_signed_request_t
{
    std::string apiKey{};
    timestamp_ms_t timestamp{};
    std::uint64_t recvWindow{};
    std::string signature{};
};
```

But **you don't populate these yourself**. After `async_session_logon()`,
the WS API client's `async_execute()` injects the auth fields on your behalf
(the `inject_auth` code path in `websocket_api::client`).

### Response wrapping

WS API responses are wrapped in an RPC envelope:

```cpp
template<typename T>
struct websocket_api_response_t
{
    std::string id{};
    int status{};                                          // HTTP status
    std::optional<T> result{};                             // the payload
    std::optional<std::vector<rate_limit_t>> rateLimits{};
    std::optional<websocket_api_error_t> error{};
};
```

For order operations, `T = order_response_t` — **identical to REST**. The
`status` field is the HTTP-status-code equivalent (200 on success), `error`
is populated on failure with Binance's error code and message.

### REST vs WS API: when each wins

| Factor | REST | WS API |
|--------|------|--------|
| Latency | 100–500 ms per call | 2–10 ms after logon |
| Setup cost | None | TCP + TLS + WS handshake + logon (once) |
| Response type | `order_response_t` | `websocket_api_response_t<order_response_t>` |
| Error handling | HTTP status + body | Envelope `status` + `error` |
| Rate limits | Weight per request | Weight per request (shared pool) |
| Connection loss | Next request retries | Must reconnect and logon again |
| Works without keys | Market data only | Market data only |

**Use REST** for initial setup (exchange info, existing orders snapshot),
occasional queries, and operations that have no WS API equivalent
(`modify_isolated_margin`, `change_margin_type`, `auto_cancel`).

**Use WS API** for the hot path of your trading loop — new orders, cancels,
modifies, order status polling from a risk engine, position queries.

A common pattern is to **maintain the WS API connection alongside a stream
subscription** on the same executor, using both for different purposes.

## Algo orders

Binance offers two algorithmic execution algorithms:

```cpp
enum class algo_type_t { vp, twap };
```

- **VP** (Volume Participation) — slices a parent order into child orders
  that participate as a percentage of the traded volume.
- **TWAP** (Time Weighted Average Price) — slices over a fixed duration,
  targeting the TWAP benchmark.

### Place an algo order

```cpp
struct new_algo_order_request_t
{
    symbol_t symbol{};
    order_side_t side{};
    std::optional<position_side_t> positionSide{};
    decimal_t quantity{};
    std::optional<time_in_force_t> timeInForce{};
    std::optional<bool> reduceOnly{};
    algo_type_t algoType{};
    // ... plus algo-specific parameters (participationRate, duration, etc.)
};
```

Returned as `algo_order_response_t` — a parent order ID plus the status of
the algo execution. Cancel via `cancel_algo_order_request_t`, query via
`query_algo_order_request_t`, list via `all_algo_orders_request_t` or
`open_algo_orders_request_t`.

Also available through the WS API with identical semantics
(`websocket_api_algo_order_place_request_t`,
`websocket_api_algo_order_cancel_request_t`).

Live algo status is pushed via the user data stream as
`algo_order_update_event_t` — see [05-user-stream-events.md](05-user-stream-events.md).

## Placing an order correctly

A canonical new-order flow:

```cpp
boost::cobalt::task<int> place(futures_usdm_api& api)
{
    auto rest = co_await api.create_rest_client();
    if (!rest) co_return 1;

    // 1. Validate the symbol exists and get its filters.
    auto info = co_await (*rest)->market_data.async_execute(exchange_info_request_t{});
    if (!info) co_return 1;

    const auto* sym = find_symbol(*info, "BTCUSDT");
    if (!sym) co_return 1;

    // 2. Build the order, honoring the symbol's price/quantity precision.
    new_order_request_t req{};
    req.symbol = sym->symbol;
    req.side = order_side_t::buy;
    req.type = order_type_t::limit;
    req.timeInForce = time_in_force_t::gtc;
    req.quantity = round_to_step(decimal_t{"0.002"}, sym->quantity_step);
    req.price    = round_to_tick(decimal_t{"60000"}, sym->price_tick);

    // 3. Check minimum notional.
    if (req.quantity * req.price < sym->min_notional) {
        spdlog::error("notional below minimum");
        co_return 1;
    }

    // 4. Submit.
    auto r = co_await (*rest)->trade.async_execute(req);
    if (!r) {
        spdlog::error("{}", r.err.message);
        co_return 1;
    }

    spdlog::info("order {} status={}", r->orderId, types::to_string(r->status));
    co_return 0;
}
```

**Common rejection codes:**

| Binance code | Meaning |
|:---:|---|
| −1013 | Filter rejection (price/quantity tick, min notional) |
| −1102 | Mandatory parameter was missing |
| −1116 | Invalid orderType |
| −1121 | Invalid symbol |
| −2010 | New order rejected — insufficient balance or risk |
| −2011 | Cancel rejected — unknown order |
| −2013 | Order does not exist |
| −4003 | Quantity less than minimum |
| −4164 | Notional less than 100 (USD-M minimum) |
| −4165 | Notional over max |

See [Binance error code docs](https://binance-docs.github.io/apidocs/futures/en/#error-codes)
for the full list.

## Summary

| You want | Use |
|----------|-----|
| Place an order with lowest latency | WS API `async_execute(websocket_api_order_place_request_t)` |
| Place an order simply | REST `trade.async_execute(new_order_request_t)` |
| Validate an order without placing | REST `test_order_request_t` |
| Modify a resting limit | REST or WS API `modify_order_request_t` |
| Cancel one order | REST or WS API `cancel_order_request_t` |
| Cancel every open order for a symbol | `cancel_all_open_orders_request_t` |
| Emergency deadman switch | `auto_cancel_request_t` |
| See live status changes | User data stream `order_trade_update_event_t` |
| Fetch individual fills for accounting | REST `account_trade_request_t` |
| Schedule an execution algorithm | `new_algo_order_request_t` (VP or TWAP) |
