# Plan: Add All Requests and Streams to demo-cli

## Current State

The demo CLI (`examples/binapi2/fapi/async-demo-cli/`) has **43 commands** covering a
subset of the library's API surface. The library exposes **~110 distinct request types**
across 5 REST services, 1 WebSocket API client, and 23 stream subscriptions.

### Files

| File | Role |
|------|------|
| `main.cpp` | Entry point, flag parsing, `commands[]` dispatch table |
| `common.hpp/cpp` | Shared state, `handle_result`, `parse_enum`, `find_flag`, logging |
| `cmd_market_data.hpp/cpp` | 15 market data commands |
| `cmd_account.hpp/cpp` | 4 account commands |
| `cmd_trade.hpp/cpp` | 5 trade commands |
| `cmd_ws_api.hpp/cpp` | 4 WebSocket API commands |
| `cmd_stream.hpp/cpp` | 9 market stream commands |
| `cmd_user_stream.hpp/cpp` | 4 user data stream commands |
| `cmd_order_book.hpp/cpp` | 1 live order book command |
| `cmd_pipeline_order_book.hpp/cpp` | 1 pipelined order book command |

---

## Command Taxonomy

```
command
├── rest_command                          — create_rest_client + async_execute + handle_result
│   ├── market_data_command              — dispatches to (*rest)->market_data
│   │   ├── kline_command                — <symbol|pair> <interval> [limit]  (6 endpoints)
│   │   └── futures_analytics_command    — <symbol> <period> [limit]         (5 endpoints)
│   ├── account_command                  — dispatches to (*rest)->account
│   │   └── download_command             — <startTime> <endTime> or <downloadId>  (6 endpoints)
│   ├── trade_command                    — dispatches to (*rest)->trade
│   ├── convert_command                  — dispatches to (*rest)->convert
│   └── user_data_stream_command         — dispatches to (*rest)->user_data_streams
├── ws_api_command                       — create_ws_api_client + connect + execute + close
│   ├── ws_public_command                — no logon required
│   └── ws_signed_command                — logon + execute
├── market_stream_command                — create_market_stream + subscribe + event loop
├── user_stream_command                  — listen key + subscribe + variant event loop
└── order_book_command                   — specialized (REST snapshot + stream deltas)
```

---

## Boilerplate Helpers (common.hpp)

### 1. REST service executors — one per service group

Each eliminates the 5-line `create_rest_client` → `error check` → `async_execute` →
`handle_result` boilerplate. After refactoring, a simple parameterless command becomes
a one-liner:

```cpp
// Template: create REST client, dispatch to service, handle_result.
template<typename Request>
boost::cobalt::task<int> exec_market_data(binapi2::futures_usdm_api& c, Request req);

template<typename Request>
boost::cobalt::task<int> exec_account(binapi2::futures_usdm_api& c, Request req);

template<typename Request>
boost::cobalt::task<int> exec_trade(binapi2::futures_usdm_api& c, Request req);

template<typename Request>
boost::cobalt::task<int> exec_convert(binapi2::futures_usdm_api& c, Request req);

template<typename Request>
boost::cobalt::task<int> exec_user_data_streams(binapi2::futures_usdm_api& c, Request req);
```

**Example — before:**
```cpp
boost::cobalt::task<int> cmd_ping(binapi2::futures_usdm_api& c, const args_t&)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->market_data.async_execute(types::ping_request_t{});
    if (!r) { print_error(r.err); co_return 1; }
    spdlog::info("pong");
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}
```

**Example — after:**
```cpp
boost::cobalt::task<int> cmd_ping(binapi2::futures_usdm_api& c, const args_t&)
{
    co_return co_await exec_market_data(c, types::ping_request_t{});
}
```

Note: the handful of existing commands that print a custom summary line (ping → "pong",
balances → filter zero, order-book → top 5 levels) keep their hand-written bodies.
All new commands use the one-liner form.

### 2. WS API executors — public vs signed

```cpp
// No logon: connect → execute → print → close.
template<typename Request>
boost::cobalt::task<int> exec_ws_public(binapi2::futures_usdm_api& c, Request req);

// With logon: connect → logon → execute → print → close.
template<typename Request>
boost::cobalt::task<int> exec_ws_signed(binapi2::futures_usdm_api& c, Request req);
```

**Example — before (8 lines per command):**
```cpp
boost::cobalt::task<int> cmd_ws_account_status(binapi2::futures_usdm_api& c, const args_t&)
{
    auto ws = co_await c.create_ws_api_client();
    if (!ws) { spdlog::error("connect: {}", ws.err.message); co_return 1; }
    if (auto conn = co_await (*ws)->async_connect(); !conn) { print_error(conn.err); co_return 1; }
    auto logon = co_await (*ws)->async_session_logon();
    if (!logon) { print_error(logon.err); co_await (*ws)->async_close(); co_return 1; }
    auto r = co_await (*ws)->async_execute(types::ws_account_status_request_t{});
    if (!r) { print_error(r.err); co_await (*ws)->async_close(); co_return 1; }
    if (r->result) spdlog::info("feeTier={} canTrade={}", ...);
    if (verbosity >= 1 && r->result) print_json(*r->result);
    co_await (*ws)->async_close();
    co_return 0;
}
```

**Example — after:**
```cpp
boost::cobalt::task<int> cmd_ws_account_status(binapi2::futures_usdm_api& c, const args_t&)
{
    co_return co_await exec_ws_signed(c, types::ws_account_status_request_t{});
}
```

### 3. Market stream executor

```cpp
// Create market stream, attach recorder, subscribe, print events in a loop.
template<typename Subscription>
boost::cobalt::task<int> exec_stream(binapi2::futures_usdm_api& c, Subscription sub);
```

**Example — before (12 lines per stream):**
```cpp
boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: ..."); co_return 1; }
    auto streams = c.create_market_stream();
    if (record_buffer) streams->connection().attach_buffer(*record_buffer);
    types::book_ticker_subscription sub;
    sub.symbol = args[0];
    auto gen = streams->subscribe(sub);
    while (gen) {
        auto event = co_await gen;
        if (!event) { print_error(event.err); co_return 1; }
        if (verbosity >= 1) { print_json(*event); }
        else { out("{}  bid: {} x {}  ask: {} x {}", ...); }
    }
    co_return 0;
}
```

**Example — after:**
```cpp
boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-book-ticker <symbol>"); co_return 1; }
    types::book_ticker_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}
```

### 4. Kline argument parser (market data subclass)

6 kline endpoints share the `<symbol|pair> <interval> [limit]` shape. Two flavors:
symbol-based and pair-based.

```cpp
// Parse symbol + interval + optional limit into a kline-like request, then execute.
template<typename Request>
boost::cobalt::task<int> cmd_kline(binapi2::futures_usdm_api& c, const args_t& args,
                                   std::string_view usage);

// Parse pair + interval + optional limit (for continuous-kline, index-price-kline).
template<typename Request>
boost::cobalt::task<int> cmd_pair_kline(binapi2::futures_usdm_api& c, const args_t& args,
                                        std::string_view usage);
```

**Endpoints using `cmd_kline` (symbol-based):**
- `klines` → `klines_request_t`
- `mark-price-klines` → `mark_price_klines_request_t`
- `premium-index-klines` → `premium_index_klines_request_t`

**Endpoints using `cmd_pair_kline` (pair-based):**
- `continuous-kline` → `continuous_kline_request_t`
- `index-price-kline` → `index_price_kline_request_t`

(Existing `klines` command can be refactored to use `cmd_kline` too.)

### 5. Futures analytics argument parser (market data subclass)

5 analytics endpoints all have the `<symbol> <period> [limit]` shape:

```cpp
// Parse symbol + period + optional limit, then execute via market_data service.
template<typename Request>
boost::cobalt::task<int> cmd_futures_analytics(binapi2::futures_usdm_api& c, const args_t& args,
                                               std::string_view usage);
```

**Endpoints:**
- `open-interest-stats` → `open_interest_statistics_request_t`
- `top-ls-account-ratio` → `top_long_short_account_ratio_request_t`
- `top-ls-trader-ratio` → `top_trader_long_short_ratio_request_t`
- `long-short-ratio` → `long_short_ratio_request_t`
- `taker-volume` → `taker_buy_sell_volume_request_t`

### 6. Download argument parsers (account subclass)

6 download endpoints, two shapes:

```cpp
// Parse <startTime> <endTime> as epoch-ms, execute via account service.
template<typename Request>
boost::cobalt::task<int> cmd_download_id(binapi2::futures_usdm_api& c, const args_t& args,
                                         std::string_view usage);

// Parse <downloadId> string, execute via account service.
template<typename Request>
boost::cobalt::task<int> cmd_download_link(binapi2::futures_usdm_api& c, const args_t& args,
                                           std::string_view usage);
```

**Endpoints:**
- `download-id-transaction` / `download-link-transaction`
- `download-id-order` / `download-link-order`
- `download-id-trade` / `download-link-trade`

---

## Missing Endpoints — Full Inventory

### REST: Market Data (24 missing)

| # | Command name | Request type | Required args | Optional args |
|---|-------------|-------------|--------------|--------------|
| 1 | `aggregate-trades` | `aggregate_trades_request_t` | `<symbol>` | `[limit]` |
| 2 | `historical-trades` | `historical_trades_request_t` | `<symbol>` | `[limit]` |
| 3 | `continuous-kline` | `continuous_kline_request_t` | `<pair> <interval>` | `[limit]` |
| 4 | `index-price-kline` | `index_price_kline_request_t` | `<pair> <interval>` | `[limit]` |
| 5 | `mark-price-klines` | `mark_price_klines_request_t` | `<symbol> <interval>` | `[limit]` |
| 6 | `premium-index-klines` | `premium_index_klines_request_t` | `<symbol> <interval>` | `[limit]` |
| 7 | `price-ticker-v2` | `price_ticker_v2_request_t` | `<symbol>` | — |
| 8 | `price-tickers-v2` | `price_tickers_v2_request_t` | — | — |
| 9 | `ticker-24hrs` | `ticker_24hrs_request_t` | — | — |
| 10 | `funding-rate-info` | `funding_rate_info_request_t` | — | — |
| 11 | `open-interest-stats` | `open_interest_statistics_request_t` | `<symbol> <period>` | `[limit]` |
| 12 | `top-ls-account-ratio` | `top_long_short_account_ratio_request_t` | `<symbol> <period>` | `[limit]` |
| 13 | `top-ls-trader-ratio` | `top_trader_long_short_ratio_request_t` | `<symbol> <period>` | `[limit]` |
| 14 | `long-short-ratio` | `long_short_ratio_request_t` | `<symbol> <period>` | `[limit]` |
| 15 | `taker-volume` | `taker_buy_sell_volume_request_t` | `<symbol> <period>` | `[limit]` |
| 16 | `basis` | `basis_request_t` | `<pair> <period>` | `[limit]` |
| 17 | `delivery-price` | `delivery_price_request_t` | `<pair>` | — |
| 18 | `composite-index-info` | `composite_index_info_request_t` | — | `[symbol]` |
| 19 | `index-constituents` | `index_constituents_request_t` | `<symbol>` | — |
| 20 | `asset-index` | `asset_index_request_t` | — | `[symbol]` |
| 21 | `insurance-fund` | `insurance_fund_request_t` | — | `[symbol]` |
| 22 | `adl-risk` | `adl_risk_request_t` | — | `[symbol]` |
| 23 | `rpi-depth` | `rpi_depth_request_t` | `<symbol>` | `[limit]` |
| 24 | `trading-schedule` | `trading_schedule_request_t` | — | — |

### REST: Account (17 missing)

| # | Command name | Request type | Required args | Optional args |
|---|-------------|-------------|--------------|--------------|
| 1 | `account-config` | `account_config_request_t` | — | — |
| 2 | `symbol-config` | `symbol_config_request_t` | — | `[symbol]` |
| 3 | `multi-assets-mode` | `get_multi_assets_mode_request_t` | — | — |
| 4 | `position-mode` | `get_position_mode_request_t` | — | — |
| 5 | `rate-limit-order` | `rate_limit_order_request_t` | — | — |
| 6 | `leverage-bracket` | `leverage_bracket_request_t` | — | `[symbol]` |
| 7 | `commission-rate` | `commission_rate_request_t` | `<symbol>` | — |
| 8 | `bnb-burn` | `get_bnb_burn_request_t` | — | — |
| 9 | `toggle-bnb-burn` | `toggle_bnb_burn_request_t` | `<true\|false>` | — |
| 10 | `quantitative-rules` | `quantitative_rules_request_t` | — | `[symbol]` |
| 11 | `pm-account-info` | `pm_account_info_request_t` | `<asset>` | — |
| 12 | `download-id-transaction` | `download_id_transaction_request_t` | `<startTime> <endTime>` | — |
| 13 | `download-link-transaction` | `download_link_transaction_request_t` | `<downloadId>` | — |
| 14 | `download-id-order` | `download_id_order_request_t` | `<startTime> <endTime>` | — |
| 15 | `download-link-order` | `download_link_order_request_t` | `<downloadId>` | — |
| 16 | `download-id-trade` | `download_id_trade_request_t` | `<startTime> <endTime>` | — |
| 17 | `download-link-trade` | `download_link_trade_request_t` | `<downloadId>` | — |

### REST: Trade (24 missing)

| # | Command name | Request type | Required args | Optional args |
|---|-------------|-------------|--------------|--------------|
| 1 | `modify-order` | `modify_order_request_t` | `<symbol> <side> <orderId> -q Q -p P` | — |
| 2 | `cancel-multiple-orders` | `cancel_multiple_orders_request_t` | `<symbol> <id1,id2,...>` | — |
| 3 | `cancel-all-orders` | `cancel_all_open_orders_request_t` | `<symbol>` | — |
| 4 | `auto-cancel` | `auto_cancel_request_t` | `<symbol> <countdownMs>` | — |
| 5 | `query-open-order` | `query_open_order_request_t` | `<symbol> <orderId>` | — |
| 6 | `all-orders` | `all_orders_request_t` | `<symbol>` | `[limit]` |
| 7 | `position-info-v3` | `position_info_v3_request_t` | — | `[symbol]` |
| 8 | `adl-quantile` | `adl_quantile_request_t` | — | `[symbol]` |
| 9 | `force-orders` | `force_orders_request_t` | — | `[symbol] [limit]` |
| 10 | `account-trades` | `account_trade_request_t` | `<symbol>` | `[limit]` |
| 11 | `change-position-mode` | `change_position_mode_request_t` | `<true\|false>` | — |
| 12 | `change-multi-assets-mode` | `change_multi_assets_mode_request_t` | `<true\|false>` | — |
| 13 | `change-leverage` | `change_leverage_request_t` | `<symbol> <leverage>` | — |
| 14 | `change-margin-type` | `change_margin_type_request_t` | `<symbol> <ISOLATED\|CROSSED>` | — |
| 15 | `modify-isolated-margin` | `modify_isolated_margin_request_t` | `<symbol> <amount> <1\|2>` | — |
| 16 | `position-margin-history` | `position_margin_history_request_t` | `<symbol>` | `[limit]` |
| 17 | `order-modify-history` | `order_modify_history_request_t` | `<symbol>` | `[orderId]` |
| 18 | `new-algo-order` | `new_algo_order_request_t` | `<symbol> <side> <type> <algoType> -q Q` | `[-p P]` |
| 19 | `cancel-algo-order` | `cancel_algo_order_request_t` | `<algoId>` | — |
| 20 | `query-algo-order` | `query_algo_order_request_t` | `<algoId>` | — |
| 21 | `all-algo-orders` | `all_algo_orders_request_t` | `<symbol>` | `[limit]` |
| 22 | `open-algo-orders` | `open_algo_orders_request_t` | — | — |
| 23 | `cancel-all-algo-orders` | `cancel_all_algo_orders_request_t` | — | — |
| 24 | `tradfi-perps` | `tradfi_perps_request_t` | — | — |

### REST: Convert (3 missing — new file)

| # | Command name | Request type | Required args |
|---|-------------|-------------|--------------|
| 1 | `convert-quote` | `convert_quote_request_t` | `<fromAsset> <toAsset> <fromAmount>` |
| 2 | `convert-accept` | `convert_accept_request_t` | `<quoteId>` |
| 3 | `convert-order-status` | `convert_order_status_request_t` | `<orderId>` |

### WebSocket API (12 missing)

| # | Command name | Request type | Auth | Required args |
|---|-------------|-------------|------|--------------|
| 1 | `ws-book-ticker` | `websocket_api_book_ticker_request_t` | no | `[symbol]` |
| 2 | `ws-price-ticker` | `websocket_api_price_ticker_request_t` | no | `[symbol]` |
| 3 | `ws-order-query` | `websocket_api_order_query_request_t` | yes | `<symbol> <orderId>` |
| 4 | `ws-order-modify` | `websocket_api_order_modify_request_t` | yes | `<sym> <side> <orderId> -q Q -p P` |
| 5 | `ws-position` | `websocket_api_position_request_t` | yes | `[symbol]` |
| 6 | `ws-account-status-v2` | `ws_account_status_v2_request_t` | yes | — |
| 7 | `ws-account-balance` | `ws_account_balance_request_t` | yes | — |
| 8 | `ws-algo-order-place` | `websocket_api_algo_order_place_request_t` | yes | `<sym> <side> <type> <algo> -q Q` |
| 9 | `ws-algo-order-cancel` | `websocket_api_algo_order_cancel_request_t` | yes | `<algoId>` |
| 10 | `ws-user-stream-start` | `ws_user_data_stream_start_request_t` | api-key | — |
| 11 | `ws-user-stream-ping` | `ws_user_data_stream_ping_request_t` | api-key | — |
| 12 | `ws-user-stream-stop` | `ws_user_data_stream_stop_request_t` | api-key | — |

### Market Streams (12 missing)

| # | Command name | Subscription type | Required args | Optional args |
|---|-------------|------------------|--------------|--------------|
| 1 | `stream-aggregate-trade` | `aggregate_trade_subscription` | `<symbol>` | — |
| 2 | `stream-diff-depth` | `diff_book_depth_subscription` | `<symbol>` | `[speed]` |
| 3 | `stream-mini-ticker` | `mini_ticker_subscription` | `<symbol>` | — |
| 4 | `stream-all-liquidations` | `all_market_liquidation_order_subscription` | — | — |
| 5 | `stream-all-mark-prices` | `all_market_mark_price_subscription` | — | — |
| 6 | `stream-continuous-kline` | `continuous_contract_kline_subscription` | `<pair> <interval>` | — |
| 7 | `stream-composite-index` | `composite_index_subscription` | `<symbol>` | — |
| 8 | `stream-contract-info` | `contract_info_subscription` | — | — |
| 9 | `stream-asset-index` | `asset_index_subscription` | `<symbol>` | — |
| 10 | `stream-all-asset-index` | `all_asset_index_subscription` | — | — |
| 11 | `stream-trading-session` | `trading_session_subscription` | — | — |
| 12 | `stream-rpi-diff-depth` | `rpi_diff_book_depth_subscription` | `<symbol>` | — |

---

## Totals

| Category | Existing | Missing | After |
|----------|---------|---------|-------|
| Market data REST | 15 | 24 | 39 |
| Account REST | 4 | 17 | 21 |
| Trade REST | 5 | 24 | 29 |
| Convert REST | 0 | 3 | 3 |
| WebSocket API | 4 | 12 | 16 |
| Market streams | 9 | 12 | 21 |
| User data streams | 4 | 0 | 4 |
| Order book | 2 | 0 | 2 |
| **Total** | **43** | **92** | **135** |

---

## Implementation Plan

### Phase 0 — Refactoring (before adding new commands)

Add boilerplate helpers to `common.hpp` (template implementations inline in the
header since they're coroutine templates):

1. **REST executors** (§1 above): `exec_market_data`, `exec_account`, `exec_trade`,
   `exec_convert`, `exec_user_data_streams` — 5 template functions.

2. **WS API executors** (§2): `exec_ws_public`, `exec_ws_signed` — 2 template functions.

3. **Market stream executor** (§3): `exec_stream` — 1 template function.

4. **Argument-shape helpers** (§4–6): `cmd_kline`, `cmd_pair_kline`,
   `cmd_futures_analytics`, `cmd_download_id`, `cmd_download_link`.

5. **Refactor existing 43 commands** to use the new helpers where applicable.
   Commands with custom summary printing (ping "pong", balances filter-zero,
   order-book top-5, book-ticker bid/ask line, etc.) keep their bodies but
   can still use the REST executor internally via a two-step pattern:
   ```cpp
   auto r = co_await rest_call(c, types::ping_request_t{});
   // custom summary ...
   ```
   Or just leave them as-is — the priority is making the 92 new commands trivial.

### Phase 1 — New files

- Create `cmd_convert.hpp` / `cmd_convert.cpp` (3 commands)
- Add `cmd_convert.cpp` to `CMakeLists.txt`

### Phase 2 — Add commands to existing files

Edit in parallel:
- `cmd_market_data.hpp/cpp` — add 24 commands
- `cmd_account.hpp/cpp` — add 17 commands
- `cmd_trade.hpp/cpp` — add 24 commands
- `cmd_ws_api.hpp/cpp` — add 12 commands
- `cmd_stream.hpp/cpp` — add 12 commands

### Phase 3 — Register commands

- `main.cpp` — add `#include "cmd_convert.hpp"`, add all 92 new entries to
  the `commands[]` table.

### Phase 4 — Build and verify

- `./build.sh`
- Fix any compilation errors
- Spot-check a few commands against testnet

---

## Notes

- All new commands use `co_return co_await exec_*(c, req)` — no custom summary
  printing. Users pass `-v` for JSON output.
- `pair_t` fields (continuous kline, basis, delivery, index price kline) accept
  the pair string directly (e.g. "BTCUSDT").
- `kline_interval_t` is reused for the `period` field in futures data analytics
  requests (the distinct request types use `kline_interval_t` not
  `futures_data_period_t`).
- Download endpoints take epoch-ms timestamps as string args (`std::stoull`).
- `toggle_bnb_burn_request_t.feeBurn` is a string field ("true"/"false").
- `change_position_mode_request_t.dualSidePosition` is a string field ("true"/"false").
- `change_multi_assets_mode_request_t.multiAssetsMargin` is a string field ("true"/"false").
- `change_margin_type_request_t.marginType` is a string field ("ISOLATED"/"CROSSED").
- `modify_isolated_margin_request_t.type` is `delta_type_t` (1=add, 2=reduce).
