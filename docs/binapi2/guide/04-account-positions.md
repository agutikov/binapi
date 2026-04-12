# 4. Account, balances, positions

This document covers everything about **your account state**: assets held,
positions open, margin configuration, and the settings that control how your
account behaves (leverage, margin type, position mode, multi-assets mode).

All of this is available via REST, most is also available via the
WebSocket API, and all changes are pushed live via the user data stream
(next document).

## Account concepts

### Margin, leverage, notional

A futures position is opened by posting **initial margin** (collateral) to
cover the risk. The exchange sets a **leverage multiplier** that determines
how much notional exposure you get per dollar of margin:

```
required_margin = notional / leverage
notional        = quantity * mark_price
```

Example: buying 0.1 BTC at $60,000 with 10× leverage:
- notional = 0.1 × 60 000 = $6 000
- required_margin = 6 000 / 10 = $600

The position P&L flows through **unrealized P&L** until closed. When your
margin ratio approaches 100 % (losses erode your margin), the exchange
triggers **liquidation** — a forced close at the mark price.

**Initial margin** is what you post to open. **Maintenance margin** is the
minimum you must keep to stay open. The difference is your buffer before
liquidation.

### Margin type: cross vs isolated

```cpp
enum class margin_type_t { isolated = 1, crossed = 2 };
```

- **Cross** (`crossed`): All positions share your entire wallet balance as
  collateral. Losses on one position reduce the margin available for others.
  Liquidations are calculated against the combined account equity. Most
  flexible; highest interdependence.
- **Isolated** (`isolated`): Each position has a fixed, pinned margin amount.
  Losses are capped at that amount. If the position liquidates, no other
  positions are affected. Safer compartmentalization; less capital-efficient.

Set per-symbol via `change_margin_type_request_t`. Switching requires
closing the position first.

### Position mode: one-way vs hedge

- **One-way mode** (the default): Net position. Buying 1 BTC then selling
  0.3 BTC leaves you with a single +0.7 BTC long.
- **Hedge mode**: Simultaneous long and short positions on the same symbol.
  Each direction is tracked independently. Specify `positionSide` on each
  order (`long`, `short`, or `both` for one-way).

```cpp
enum class position_side_t { both = 0, long_ = 1, short_ = 2 };
```

In one-way mode, always send `both`. In hedge mode, send `long_` or `short_`
to pick which of the two positions the order affects.

Toggle globally via `change_position_mode_request_t`. Cannot switch while
positions are open.

### Multi-assets mode

```cpp
struct multi_assets_mode_response_t { bool multiAssetsMargin{}; };
```

Normally each USD-M asset has its own margin silo. Multi-assets mode lets
USDT, USDC, BUSD etc. mutually back positions — profits in one can cover
losses in another. Enabled via `change_multi_assets_mode_request_t`.

### BNB burn

```cpp
struct get_bnb_burn_response_t { bool feeBurn{}; };
```

If enabled, Binance deducts trading fees from your BNB balance at a
discounted rate instead of from the quote asset. Purely a fee preference
— set via `toggle_bnb_burn_request_t`.

---

## REST: account information

### Account information (full snapshot)

```cpp
struct account_information_t
{
    decimal_t totalInitialMargin{};
    decimal_t totalMaintMargin{};
    decimal_t totalWalletBalance{};
    decimal_t totalUnrealizedProfit{};
    decimal_t totalMarginBalance{};
    decimal_t totalPositionInitialMargin{};
    decimal_t totalOpenOrderInitialMargin{};
    decimal_t totalCrossWalletBalance{};
    decimal_t totalCrossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    std::vector<account_asset_t> assets{};
    std::vector<account_position_t> positions{};
};
```

This is the most comprehensive account view. Returned by
`account_info_request_t` (REST → `/fapi/v3/account`).

**Aggregate fields** (all in USDT terms):
- `totalWalletBalance` — your base wallet balance (cash).
- `totalUnrealizedProfit` — mark-to-market P&L on open positions.
- `totalMarginBalance` = wallet + unrealized P&L. Your effective equity.
- `totalInitialMargin` — margin locked into open positions and orders.
- `totalMaintMargin` — minimum required to keep positions open.
- `availableBalance` — what you can use to open new positions.
- `maxWithdrawAmount` — what you can withdraw to spot.

**Per-asset breakdown:**

```cpp
struct account_asset_t
{
    std::string asset{};           // "USDT", "USDC", "BNB", ...
    decimal_t walletBalance{};
    decimal_t unrealizedProfit{};
    decimal_t marginBalance{};
    decimal_t maintMargin{};
    decimal_t initialMargin{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    decimal_t crossWalletBalance{};
    decimal_t crossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    bool marginAvailable{};
    timestamp_ms_t updateTime{};
};
```

**Per-position breakdown:**

```cpp
struct account_position_t
{
    symbol_t symbol{};
    decimal_t initialMargin{};
    decimal_t maintMargin{};
    decimal_t unrealizedProfit{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    int leverage{};
    bool isolated{};
    decimal_t entryPrice{};
    decimal_t maxNotional{};
    decimal_t bidNotional{};
    decimal_t askNotional{};
    position_side_t positionSide{};
    decimal_t positionAmt{};       // signed: positive = long
    decimal_t notional{};
    decimal_t isolatedWallet{};
    timestamp_ms_t updateTime{};
};
```

`positionAmt` is the signed quantity — positive for long, negative for
short. `notional = positionAmt * markPrice`. `entryPrice` is the VWAP of
position entries.

### Balances only

```cpp
struct balances_request_t { };      // → /fapi/v3/balance

struct futures_account_balance_t
{
    std::string accountAlias{};
    std::string asset{};
    decimal_t balance{};
    decimal_t crossWalletBalance{};
    decimal_t crossUnPnl{};
    decimal_t availableBalance{};
    decimal_t maxWithdrawAmount{};
    bool marginAvailable{};
    timestamp_ms_t updateTime{};
};
```

Returns `std::vector<futures_account_balance_t>`. A lighter, asset-only view
compared to the full account information. Use this for balance displays
where you don't need position data.

### Position risk

```cpp
struct position_risk_request_t { std::optional<symbol_t> symbol{}; };

struct position_risk_t
{
    std::string symbol{};
    decimal_t positionAmt{};
    decimal_t entryPrice{};
    decimal_t breakEvenPrice{};
    decimal_t markPrice{};
    decimal_t unRealizedProfit{};
    decimal_t liquidationPrice{};
    int leverage{};
    decimal_t maxNotionalValue{};
    margin_type_t marginType{};
    decimal_t isolatedMargin{};
    bool isAutoAddMargin{};
    position_side_t positionSide{};
    decimal_t notional{};
    decimal_t isolatedWallet{};
    timestamp_ms_t updateTime{};
};
```

Returned by `position_risk_request_t`. A **per-position risk snapshot** with
the fields you need for a risk dashboard:

- `markPrice` and `unRealizedProfit` — current valuation.
- `liquidationPrice` — exchange-calculated liquidation threshold. Your stop
  loss should sit well above this for longs (below for shorts).
- `breakEvenPrice` — entry price plus fees. Below this, closing realizes a
  loss even if `markPrice > entryPrice`.
- `maxNotionalValue` — the maximum notional you can hold at the current
  leverage. Lower leverage → higher max.

### Position info v3 (newer variant)

```cpp
struct position_info_v3_request_t { std::optional<symbol_t> symbol{}; };

struct position_info_v3_t
{
    symbol_t symbol{};
    position_side_t positionSide{};
    decimal_t positionAmt{};
    decimal_t entryPrice{};
    decimal_t breakEvenPrice{};
    decimal_t markPrice{};
    decimal_t unRealizedProfit{};
    decimal_t liquidationPrice{};
    decimal_t isolatedMargin{};
    decimal_t notional{};
    margin_type_t marginType{};
    decimal_t isolatedWallet{};
    decimal_t initialMargin{};
    decimal_t maintMargin{};
    decimal_t positionInitialMargin{};
    decimal_t openOrderInitialMargin{};
    decimal_t adl{};
    decimal_t bidNotional{};
    decimal_t askNotional{};
    timestamp_ms_t updateTime{};
};
```

A refined position view with fields dropped in v2 (`adl`, `bidNotional`,
`askNotional`). Prefer `position_info_v3_t` for new code.

### Income history

```cpp
struct income_history_request_t
{
    std::optional<symbol_t> symbol{};
    std::optional<income_type_t> incomeType{};
    std::optional<timestamp_ms_t> startTime{};
    std::optional<timestamp_ms_t> endTime{};
    std::optional<int> limit{};
};

enum class income_type_t {
    transfer, welcome_bonus, realized_pnl, funding_fee, commission,
    insurance_clear, referral_kickback, commission_rebate, api_rebate,
    contest_reward, cross_collateral_transfer, options_premium_fee,
    options_settle_profit, internal_transfer, auto_exchange,
    delivered_settelment, coin_swap_deposit, coin_swap_withdraw,
    position_limit_increase_fee,
};
```

Every cash movement affecting your futures wallet: fills (`realized_pnl`,
`commission`), funding payments (`funding_fee`), liquidations
(`insurance_clear`), transfers to/from spot, referrals, rebates. Used for
accurate accounting, tax reports, and PnL reconciliation.

---

## Config queries and changes

### Account configuration snapshot

```cpp
struct account_config_response_t
{
    int feeTier{};
    bool canTrade{};
    bool canDeposit{};
    bool canWithdraw{};
    bool dualSidePosition{};       // hedge mode
    timestamp_ms_t updateTime{};
    bool multiAssetsMargin{};
    int tradeGroupId{};
};
```

Returned by `account_config_request_t`. High-level account state — your fee
tier (based on 30-day volume and BNB holdings), capability flags, and the
two position-mode toggles.

### Symbol configuration

```cpp
struct symbol_config_request_t { std::optional<symbol_t> symbol{}; };

struct symbol_config_entry_t
{
    symbol_t symbol{};
    margin_type_t marginType{};
    bool isAutoAddMargin{};
    int leverage{};
    decimal_t maxNotionalValue{};
};
```

Returned by `symbol_config_request_t`. Per-symbol settings: current leverage,
margin type (cross or isolated), whether auto-add-margin is enabled, and the
max allowed notional at the current leverage. Used to check your symbol
setup before placing large orders.

### Leverage bracket

```cpp
struct leverage_bracket_request_t { std::optional<symbol_t> symbol{}; };

struct symbol_leverage_brackets_t
{
    symbol_t symbol{};
    std::optional<decimal_t> notionalCoef{};
    std::vector<leverage_bracket_entry_t> brackets{};
};

struct leverage_bracket_entry_t
{
    int bracket{};
    int initialLeverage{};
    std::uint64_t notionalCap{};
    std::uint64_t notionalFloor{};
    decimal_t maintMarginRatio{};
    decimal_t cum{};
};
```

The **leverage tier table** — as your notional grows, you're pushed into
higher brackets with lower maximum leverage and higher maintenance margin
ratio. Critical for understanding your effective leverage cap.

Example for BTCUSDT: bracket 1 might be notional 0–50 000 USDT with 125×
max; bracket 2 might be 50 000–250 000 USDT with 100× max, and so on up to
several million USDT with 1× max.

### Commission rate

```cpp
struct commission_rate_request_t { symbol_t symbol{}; };

struct commission_rate_response_t
{
    symbol_t symbol{};
    decimal_t makerCommissionRate{};
    decimal_t takerCommissionRate{};
};
```

Your actual maker/taker fee rates for a symbol (account-specific — VIP tiers
and referral discounts apply). Use to estimate P&L before placing trades.

### Rate limit

```cpp
struct rate_limit_order_request_t { };
```

Returns your current rate limit usage across multiple dimensions (orders per
10s, 60s, 24h). Monitor to avoid hitting limits.

### Other config operations

| Request | Effect |
|---------|--------|
| `change_position_mode_request_t` | Toggle hedge mode |
| `change_multi_assets_mode_request_t` | Toggle multi-assets mode |
| `change_leverage_request_t` | Set leverage for a symbol |
| `change_margin_type_request_t` | Set cross or isolated for a symbol |
| `modify_isolated_margin_request_t` | Add/remove margin from an isolated position |
| `position_margin_history_request_t` | History of isolated-margin adjustments |
| `quantitative_rules_request_t` | Trading rules indicators (for quant compliance) |
| `pm_account_info_request_t` | Portfolio margin account info |
| `toggle_bnb_burn_request_t` | Enable/disable BNB fee deduction |

### Downloadable histories

For large histories, Binance offers an async download workflow:

```cpp
struct download_id_transaction_request_t
{
    timestamp_ms_t startTime{};
    timestamp_ms_t endTime{};
};
struct download_id_response_t { std::string downloadId{}; /* ... */ };

struct download_link_transaction_request_t { std::string downloadId{}; };
struct download_link_response_t { std::string downloadUrl{}; /* ... */ };
```

Step 1: request a `downloadId` for a time range. Step 2: poll the download
link endpoint with that ID until the file is ready. Available for:
- `transaction` — income history
- `order` — order history
- `trade` — trade history

## WebSocket API: account and positions

The WS API exposes three account queries and one position query:

```cpp
// Full account information — same type as REST
struct ws_account_status_request_t { };
struct ws_account_status_v2_request_t { };
// response: account_information_t (same as REST)

// Balances only
struct ws_account_balance_request_t { };
// response: std::vector<futures_account_balance_t>

// Positions
struct websocket_api_position_request_t : websocket_api_signed_request_t
{
    std::optional<symbol_t> symbol{};
};
// response: std::vector<position_risk_t>
```

These return **identical response types to the REST equivalents**. The only
difference is the envelope (`websocket_api_response_t<T>`) and the lower
latency.

Use the WS API for account queries when your bot is running a live trading
loop and needs to re-check account state frequently without paying per-query
TLS handshake cost.

## Streams: live updates

The **user data stream** pushes real-time updates whenever any of the above
state changes:

- **`account_update_event_t`** — balance or position changed (fills, funding
  payments, transfers, auto-add-margin).
- **`order_trade_update_event_t`** — any order state change or fill.
- **`margin_call_event_t`** — approaching liquidation on one or more
  positions.
- **`account_config_update_event_t`** — leverage change, multi-assets toggle.
- **`algo_order_update_event_t`** — algo order lifecycle.

These events carry the new state directly — you don't need to re-query REST
to see the updated balance after a fill, just listen to the stream.

The full event type list and field-level details are in
[05-user-stream-events.md](05-user-stream-events.md).

---

## Typical usage patterns

### Initial account state at startup

```cpp
auto rest = co_await api.create_rest_client();
auto info = co_await (*rest)->account.async_execute(account_info_request_t{});
// info->assets, info->positions, info->totalWalletBalance ...
```

Query once at startup to populate your in-memory model.

### Keep state in sync via user stream

```cpp
// Start listen key
auto lk = co_await (*rest)->user_data_streams.async_execute(start_listen_key_request_t{});

// Keep alive every 30 min
// ... separate timer task that periodically sends keepalive_listen_key_request_t

// Subscribe to user stream
auto stream = api.create_user_stream();
auto gen = stream->subscribe(lk->listenKey);
while (gen) {
    auto ev = co_await gen;
    if (!ev) break;
    std::visit(overloaded{
        [&](const account_update_event_t& e)   { apply(e, state); },
        [&](const order_trade_update_event_t& e) { apply(e, state); },
        [&](const margin_call_event_t& e)      { alert(e); },
        [](const auto&)                        { /* ignore */ },
    }, *ev);
}
```

This is the **only** reliable way to track account changes in real time.

### Periodic reconciliation

Even with the stream, drift can occur (missed events during reconnection,
subtle inconsistencies). Reconcile against REST every few minutes:

```cpp
auto info = co_await (*rest)->account.async_execute(account_info_request_t{});
validate_against(my_state, *info);
```

### Risk dashboard

For a risk monitoring UI, combine:
- `position_risk_t` from REST for `liquidationPrice`, `breakEvenPrice`.
- Mark price stream for real-time valuation.
- User stream for position updates on fills.

`liquidationPrice` is only recalculated server-side when margin or leverage
changes — use `markPrice` distance to `liquidationPrice` as your "how close
to liquidation" signal.

---

## Summary table: account data by surface

| Data | REST request | WS API request | User stream event |
|------|--------------|----------------|-------------------|
| Full account info | `account_info_request_t` | `ws_account_status_request_t` / `_v2` | — |
| Balances only | `balances_request_t` | `ws_account_balance_request_t` | partial (`account_update_event_t`) |
| Positions | `position_risk_request_t` / `position_info_v3_request_t` | `websocket_api_position_request_t` | `account_update_event_t` |
| Leverage brackets | `leverage_bracket_request_t` | — | — |
| Symbol config | `symbol_config_request_t` | — | — |
| Account config | `account_config_request_t` | — | `account_config_update_event_t` |
| Multi-assets mode | `get_multi_assets_mode_request_t` / `change_` | — | `account_config_update_event_t` |
| Position mode | `get_position_mode_request_t` / `change_` | — | — |
| Change leverage | `change_leverage_request_t` | — | `account_config_update_event_t` |
| Change margin type | `change_margin_type_request_t` | — | — |
| Modify isolated margin | `modify_isolated_margin_request_t` | — | — |
| Position margin history | `position_margin_history_request_t` | — | — |
| Commission rate | `commission_rate_request_t` | — | — |
| Rate limit usage | `rate_limit_order_request_t` | — | — |
| Income history | `income_history_request_t` | — | — |
| BNB burn | `get_bnb_burn_request_t` / `toggle_bnb_burn_request_t` | — | — |
| Quantitative rules | `quantitative_rules_request_t` | — | — |
| PM account info | `pm_account_info_request_t` | — | — |
| Download history | `download_id_*` / `download_link_*` | — | — |
| Liquidation warning | — | — | `margin_call_event_t` |
