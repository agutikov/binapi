# 5. User data stream events

The user data stream is a dedicated WebSocket connection that pushes
real-time events about **your account**: order fills, balance changes,
position updates, margin calls, algo order status, strategy execution. It
is the only way to track account state changes with sub-second latency.

This document explains the listen key lifecycle, the ten event types, and
the variant-based dispatch model binapi2 uses.

## Listen key lifecycle

A **listen key** is a short-lived token that authorizes one WebSocket
connection to receive your account's events. Obtain it via REST or WS API,
then connect the stream to `/ws/<listenKey>`.

```cpp
// REST
struct start_listen_key_request_t { };
struct listen_key_response_t { std::string listenKey{}; };

// Keep-alive — must be called within 60 minutes or the key expires
struct keepalive_listen_key_request_t { };  // returns current listen key

// Close — explicit teardown
struct close_listen_key_request_t { };
```

The same three operations are available over WebSocket API
(`ws_user_data_stream_start_request_t`, `..._ping_`, `..._stop_`).

**Operational rules:**
- A listen key is valid for 60 minutes from creation or last keepalive.
- Binance recommends sending a keepalive **every 30 minutes** for safety.
- Only **one listen key per account**. Starting a new one extends the existing
  one — you don't need to keep track of old keys.
- When the key expires, the stream disconnects and emits a
  `listen_key_expired_event_t` before closing. Your code must:
  1. Request a new listen key.
  2. Reconnect the user stream with the new key.
  3. Reconcile state via REST (you may have missed events).

### Subscribing the stream

```cpp
auto rest = co_await api.create_rest_client();
auto key = co_await (*rest)->user_data_streams.async_execute(
    start_listen_key_request_t{});
if (!key) co_return 1;

auto user_stream = api.create_user_stream();
auto gen = user_stream->subscribe(key->listenKey);

while (gen) {
    auto ev = co_await gen;
    if (!ev) { /* handle error, maybe restart */ break; }
    handle(*ev);
}
```

`handle(*ev)` receives a `user_stream_event_t`, which is a `std::variant`
over the ten event types below.

## The ten event types

```cpp
using user_stream_event_t = std::variant<
    account_update_event_t,
    order_trade_update_event_t,
    margin_call_event_t,
    listen_key_expired_event_t,
    account_config_update_event_t,
    trade_lite_event_t,
    algo_order_update_event_t,
    conditional_order_trigger_reject_event_t,
    grid_update_event_t,
    strategy_update_event_t
>;
```

Each event contains exactly one state change. Handle them with
`std::visit` + an `overloaded` dispatcher:

```cpp
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::visit(overloaded{
    [&](const account_update_event_t& e)                   { on_balance(e); },
    [&](const order_trade_update_event_t& e)               { on_order(e); },
    [&](const margin_call_event_t& e)                      { on_margin_call(e); },
    [&](const listen_key_expired_event_t&)                 { restart_stream(); },
    [&](const account_config_update_event_t& e)            { on_config(e); },
    [&](const trade_lite_event_t& e)                       { on_trade_lite(e); },
    [&](const algo_order_update_event_t& e)                { on_algo(e); },
    [&](const conditional_order_trigger_reject_event_t& e) { on_cond_reject(e); },
    [&](const grid_update_event_t& e)                      { on_grid(e); },
    [&](const strategy_update_event_t& e)                  { on_strategy(e); },
}, *ev);
```

Each case below documents what triggers the event, what it contains, and
how to act on it.

---

### 1. `account_update_event_t` — balance and position changes

**Triggered by:** any change to wallet balance or position state. Covers
order fills, funding payments, transfers, deposits/withdrawals, auto-margin
adjustments, liquidations.

```cpp
struct account_update_event_t
{
    timestamp_ms_t event_time;                      // E
    timestamp_ms_t transaction_time;                // T
    account_update_data_t update_data;              // a
};

struct account_update_data_t
{
    reason_type_t reason_type;                      // m — why this event fired
    std::vector<account_update_balance_t> balances; // B — changed balances
    std::vector<account_update_position_t> positions;// P — changed positions
};

struct account_update_balance_t
{
    std::string asset;                              // a
    decimal_t wallet_balance;                       // wb
    decimal_t cross_wallet_balance;                 // cw
    decimal_t balance_change;                       // bc — delta
};

struct account_update_position_t
{
    symbol_t symbol;                                // s
    decimal_t position_amount;                      // pa
    decimal_t entry_price;                          // ep
    decimal_t accum_realized;                       // cr — cumulative realized
    decimal_t unrealized_pnl;                       // up
    margin_type_t margin;                           // mt
    decimal_t isolated_wallet;                      // iw
    position_side_t pos_side;                       // ps
    decimal_t breakeven_price;                      // bep
};
```

**Reason types** (`reason_type_t`): `deposit`, `withdraw`, `order`, `funding_fee`,
`margin_transfer`, `margin_type_change`, `asset_transfer`, `option_put_exercise`,
`insurance_clear`, `admin_deposit`, `admin_withdraw`, `auto_exchange`,
`coin_swap_deposit`, `coin_swap_withdraw`, `transfer`, `liquidation`, etc.

**Semantics:**
- Only **changed** balances and positions are listed — not your full state.
  Use these as deltas on top of your in-memory model.
- `balance_change` is the incremental change for an order/funding fee. For a
  fee deduction it's negative.
- `position_amount` is signed: positive = long, negative = short. Zero
  means the position was closed.
- `accum_realized` is cumulative — you can compute per-event realized P&L
  as `new.accum_realized - old.accum_realized`.

**How to act:**
- Update your wallet balance model.
- Update your position book.
- If `position_amount == 0`, mark the position as closed and record the
  final realized P&L.

---

### 2. `order_trade_update_event_t` — order and trade lifecycle

**Triggered by:** every order state transition and every trade (fill). This
is the most frequent and most important event type for any trading bot.

```cpp
struct order_trade_update_event_t
{
    timestamp_ms_t event_time;                  // E
    timestamp_ms_t transaction_time;            // T
    order_trade_update_order_t order;           // o
};

struct order_trade_update_order_t
{
    symbol_t symbol;                            // s
    std::string client_order_id;                // c
    order_side_t side;                          // S
    order_type_t type;                          // o — current type
    time_in_force_t tif;                        // f
    decimal_t original_quantity;                // q
    decimal_t original_price;                   // p
    decimal_t average_price;                    // ap
    decimal_t stop_price;                       // sp
    execution_type_t exec_type;                 // x
    order_status_t status;                      // X
    std::uint64_t order_id;                     // i
    decimal_t last_filled_qty;                  // l
    decimal_t filled_accum_qty;                 // z
    decimal_t last_filled_price;                // L
    std::string commission_asset;               // N
    decimal_t commission;                       // n
    timestamp_ms_t trade_time;                  // T
    std::uint64_t trade_id;                     // t
    decimal_t bids_notional;                    // b
    decimal_t ask_notional;                     // a
    bool is_maker;                              // m
    bool is_reduce_only;                        // R
    working_type_t work_type;                   // wt
    order_type_t orig_order_type;               // ot
    position_side_t pos_side;                   // ps
    bool is_close_all;                          // cp
    std::optional<decimal_t> activation_price;  // AP
    std::optional<decimal_t> callback_rate;     // cr
    bool price_protection;                      // pP
    int ignore_si;                              // si (reserved)
    int ignore_ss;                              // ss (reserved)
    decimal_t realized_profit;                  // rp
    stp_mode_t stp;                             // V
    price_match_t price_match_mode;             // pm
    std::optional<timestamp_ms_t> gtd_auto_cancel;// gtd
    std::optional<std::string> expiry_reason;   // er
};
```

**Execution type** (`execution_type_t`) identifies what specifically
triggered the event:

| Value | Meaning |
|-------|---------|
| `new_order` | Order accepted and placed on the book |
| `canceled` | Order cancelled (by user or deadman's switch) |
| `calculated` | Liquidation or ADL execution |
| `expired` | TIF expiration (e.g. IOC partial, GTD time) |
| `trade` | A fill happened |
| `amendment` | Order was modified |

**How to distinguish the important fields:**
- `last_filled_qty` + `last_filled_price` — this *specific* fill (0 if
  event isn't a trade).
- `filled_accum_qty` — total filled so far on this order.
- `original_quantity` − `filled_accum_qty` = remaining quantity.
- `realized_profit` — P&L from the last fill (for closing fills).
- `commission` + `commission_asset` — fee paid for this fill.
- `is_maker` — were you the maker (rebate/lower fee) or taker.

**Status vs exec type:**
- `exec_type == trade` with `status == partially_filled` — a partial fill,
  more expected.
- `exec_type == trade` with `status == filled` — the fill that completed
  the order.
- `exec_type == canceled` — no fill, order removed.
- `exec_type == expired` — IOC/FOK/GTD didn't complete.

**How to act:**
- Record the fill in your trade log (store `trade_id` for deduplication).
- Update the in-memory order state keyed by `order_id` or `client_order_id`.
- On `status == filled`, remove the order from the open set.
- Flow the P&L and commission into your account state.

---

### 3. `margin_call_event_t` — liquidation warning

**Triggered by:** one or more positions approaching the maintenance margin
threshold. This is a warning, **not** a liquidation itself. If margin
continues to erode, a liquidation follows.

```cpp
struct margin_call_event_t
{
    timestamp_ms_t event_time;                      // E
    decimal_t cross_wallet_balance;                 // cw
    std::vector<margin_call_position_t> positions;  // p
};

struct margin_call_position_t
{
    symbol_t symbol;
    position_side_t pos_side;                       // ps
    decimal_t position_amount;                      // pa
    margin_type_t margin;                           // mt
    decimal_t isolated_wallet;                      // iw
    decimal_t mark_price_t;                         // mp
    decimal_t unrealized_pnl;                       // up
    decimal_t maint_margin;                         // mm
};
```

**How to act:**
- Log the event with full position details.
- Optionally send a notification (email, SMS, Slack).
- Optionally auto-add margin or close positions to avoid liquidation.

**Do not ignore this event.** By the time the exchange sends it, you're
close to losing the position.

---

### 4. `listen_key_expired_event_t` — stream about to close

**Triggered by:** the 60-minute listen key expiry. The stream will
disconnect shortly after this event.

```cpp
struct listen_key_expired_event_t
{
    timestamp_ms_t event_time;
    timestamp_ms_t transaction_time;
    std::string listen_key;
};
```

**How to act:** restart the listen key workflow:
1. Call `start_listen_key_request_t` to obtain a fresh key.
2. Reconnect the user stream with the new key.
3. Reconcile account state via REST (fetch positions, balances, open orders)
   since events during the brief gap may have been missed.

**Typically avoidable**: send a keepalive every 30 minutes and you'll never
see this event in practice.

---

### 5. `account_config_update_event_t` — config change

**Triggered by:** leverage change on a symbol, or multi-assets mode toggle.

```cpp
struct account_config_update_event_t
{
    timestamp_ms_t event_time;
    timestamp_ms_t transaction_time;
    std::optional<account_config_leverage_t> leverage_config;
    std::optional<account_config_multi_assets_t> multi_assets_config;
};

struct account_config_leverage_t
{
    symbol_t symbol;
    int leverage;
};

struct account_config_multi_assets_t
{
    bool multi_assets_mode;
};
```

Exactly one of the two `optional` fields is populated per event.

**How to act:**
- Update the in-memory config cache for that symbol.
- Recompute liquidation thresholds if you track them locally — your
  leverage changed.

---

### 6. `trade_lite_event_t` — lightweight fill

**Triggered by:** a fill, on accounts that have opted in to the "lite"
variant. A compact version of `order_trade_update_event_t` with just the
fields needed for fast order-flow tracking.

```cpp
struct trade_lite_event_t
{
    timestamp_ms_t event_time;
    timestamp_ms_t transaction_time;
    symbol_t symbol;                     // s
    decimal_t original_quantity;         // q
    decimal_t original_price;            // p
    bool is_maker;                       // m
    std::string client_order_id;         // c
    order_side_t side;                   // S
    decimal_t last_filled_price;         // L
    decimal_t last_filled_qty;           // l
    std::uint64_t trade_id;              // t
    std::uint64_t order_id;              // i
};
```

**Note:** Binance sends *both* `order_trade_update_event_t` and
`trade_lite_event_t` for every fill by default. The lite version can
sometimes arrive first — use it for the lowest-latency fill detection and
wait for the full update for detailed fields like realized P&L.

---

### 7. `algo_order_update_event_t` — algo order lifecycle

**Triggered by:** state changes for an active VP or TWAP algo order — child
order fills, trigger execution, completion, cancellation, rejection.

```cpp
struct algo_order_update_event_t
{
    timestamp_ms_t transaction_time;
    timestamp_ms_t event_time;
    algo_order_update_data_t order;
};

struct algo_order_update_data_t
{
    std::string client_algo_id;
    std::uint64_t algo_id;
    algo_type_t alg_type;                       // VP or TWAP
    order_type_t type;
    symbol_t symbol;
    order_side_t side;
    position_side_t pos_side;
    time_in_force_t tif;
    decimal_t quantity;
    algo_status_t alg_status;
    std::optional<std::string> matched_order_id;
    std::optional<decimal_t> avg_fill_price;
    std::optional<decimal_t> executed_quantity;
    std::optional<order_type_t> actual_order_type;
    std::optional<decimal_t> trigger_price;
    std::optional<decimal_t> order_price;
    std::optional<stp_mode_t> stp;
    std::optional<working_type_t> work_type;
    std::optional<price_match_t> price_match_mode;
    std::optional<bool> is_close_all;
    std::optional<bool> price_protection;
    std::optional<bool> is_reduce_only;
    std::optional<timestamp_ms_t> trigger_time;
    std::optional<timestamp_ms_t> gtd_cancel_time;
    std::optional<std::string> reject_reason;
};
```

**How to act:** update your algo-order tracking. `alg_status` transitions
tell you when an algo is running, completed, cancelled, or rejected.
`reject_reason` populates if the algo fails to start or execute.

---

### 8. `conditional_order_trigger_reject_event_t` — stop/TP trigger failed

**Triggered by:** a stop or take-profit order that failed to execute when
its trigger condition was met (e.g. insufficient margin to actually open
the resulting position, or the order would violate risk limits).

```cpp
struct conditional_order_trigger_reject_event_t
{
    timestamp_ms_t event_time;
    timestamp_ms_t message_send_time;
    conditional_order_reject_data_t order_reject;
};

struct conditional_order_reject_data_t
{
    symbol_t symbol;
    std::uint64_t order_id;
    std::string reject_reason;
};
```

**How to act:** critical for any stop-loss strategy. A rejected stop means
your stop didn't trigger and the position is still exposed. Alert
immediately and consider alternative mitigation (cancel and re-place a
market order, reduce position manually, etc.).

---

### 9. `grid_update_event_t` — grid strategy update

**Triggered by:** state change in a running grid trading strategy
(a grid of automatically-placed buy/sell orders at configured levels).

```cpp
struct grid_update_event_t
{
    timestamp_ms_t transaction_time;
    timestamp_ms_t event_time;
    grid_update_data_t grid_update;
};

struct grid_update_data_t
{
    std::uint64_t strategy_id;
    strategy_type_t strategy_type;
    strategy_status_t strategy_status;
    symbol_t symbol;
    decimal_t realized_pnl;
    decimal_t unmatched_avg_price;
    decimal_t unmatched_qty;
    decimal_t unmatched_fee;
    decimal_t matched_pnl;
    timestamp_ms_t update_time;
};
```

Grid bots are configured through the Binance UI; this event reports their
live performance. Relevant only if you run grids.

---

### 10. `strategy_update_event_t` — strategy state change

**Triggered by:** non-grid strategy status changes (DCA, stop-loss
automation, other Binance-managed strategies).

```cpp
struct strategy_update_event_t
{
    timestamp_ms_t transaction_time;
    timestamp_ms_t event_time;
    strategy_update_data_t strategy_update;
};

struct strategy_update_data_t
{
    std::uint64_t strategy_id;
    strategy_type_t strategy_type;
    strategy_status_t strategy_status;
    symbol_t symbol;
    timestamp_ms_t update_time;
    int op_code;
};
```

A minimal strategy status ping. Use with the strategy APIs to fetch full
state.

---

## Field naming: short Binance keys vs expanded C++ names

binapi2 exposes the fields with readable C++ names, but the wire JSON uses
Binance's short abbreviations (`"s"` for symbol, `"E"` for event time,
`"wb"` for wallet balance, etc.). The glaze metadata in
`include/binapi2/fapi/types/user_stream_events.hpp` maps each C++ field to
its short key.

If you need to cross-reference with the Binance docs or another library
consuming the same JSON, check the `glz::meta` specialization for the
struct.

## End-to-end handler example

```cpp
struct account_state {
    std::map<std::string, decimal_t> balances;
    std::map<std::pair<symbol_t, position_side_t>, position_info> positions;
    std::map<std::uint64_t, order_info> orders;
};

void on_event(account_state& st, const user_stream_event_t& ev)
{
    std::visit(overloaded{
        [&](const account_update_event_t& e) {
            for (const auto& b : e.update_data.balances) {
                st.balances[b.asset] = b.wallet_balance;
            }
            for (const auto& p : e.update_data.positions) {
                auto key = std::pair{ p.symbol, p.pos_side };
                if (p.position_amount.is_zero()) {
                    st.positions.erase(key);
                } else {
                    st.positions[key] = { p.position_amount, p.entry_price, p.unrealized_pnl };
                }
            }
        },
        [&](const order_trade_update_event_t& e) {
            const auto& o = e.order;
            auto& rec = st.orders[o.order_id];
            rec.status = o.status;
            rec.filled = o.filled_accum_qty;
            rec.avg_price = o.average_price;
            if (o.status == order_status_t::filled ||
                o.status == order_status_t::canceled ||
                o.status == order_status_t::expired) {
                st.orders.erase(o.order_id);
            }
        },
        [&](const margin_call_event_t& e) {
            spdlog::warn("MARGIN CALL — cross wallet: {}", e.cross_wallet_balance);
            for (const auto& p : e.positions) {
                spdlog::warn("  {} maint margin {} unrealized {}",
                             p.symbol, p.maint_margin, p.unrealized_pnl);
            }
        },
        [&](const listen_key_expired_event_t&) {
            // Schedule stream restart on a separate task.
        },
        [&](const auto&) { /* other events — handle as needed */ },
    }, ev);
}
```

## Duplication and reconciliation

Many user stream events mirror REST/WS API data:

| Event | Equivalent pull API |
|-------|---------------------|
| `account_update_event_t` | `account_info_request_t`, `balances_request_t`, `position_risk_request_t` |
| `order_trade_update_event_t` | `query_order_request_t`, `account_trade_request_t` |
| `account_config_update_event_t` | `account_config_request_t`, `symbol_config_request_t` |
| `algo_order_update_event_t` | `query_algo_order_request_t` |
| `margin_call_event_t` | (no pull equivalent — warning only) |

The stream is **always more up-to-date** than REST, but REST is the
authoritative source when the stream disconnects or misses events.

**Recommended pattern:** initialize state from REST, keep it current from
the stream, and reconcile against REST every few minutes as a drift check.

## Relationship to other documents

- [03-trading.md](03-trading.md) — order types, REST/WS API order operations.
- [04-account-positions.md](04-account-positions.md) — REST/WS API queries
  that correspond to each event type.
- [../streams.md](../streams.md) — generic stream component reference,
  reconnection model, buffers.
