# binapi2::fapi — type-level allocation audit

**Scope**: every response / event / payload struct in `include/binapi2/fapi/types/` that the parsers materialize on the data path. Drives the follow-up micro-tasks queued under `plans/`.

**Date of audit**: 2026-04-22.

**Prerequisite**: plan [`plans/001-depth-vectors-to-small-vector.md`](../../plans/001-depth-vectors-to-small-vector.md) step 1 has landed — `depth_stream_event_t.bids/asks` are now `boost::container::small_vector<price_level_t, 20>` (`depth_levels_t`) and parse without heap when ≤ 20 levels.

## Methodology

For every response / event struct, count the members that actually cause a heap allocation under libstdc++ on the current toolchain. Treated as **free** (not listed in tables):

- `decimal_t` — `__int128` (no heap).
- `timestamp_ms_t` — `uint64_t` wrapper.
- `symbol_t`, `pair_t` — short fixed-string wrappers (always SSO).
- `enum_set_t<E>` — fixed bitset.
- `optional<scalar>`, `optional<decimal_t>` — trivial.
- bare scalars (`int`, `uint64_t`, `bool`, `double`).
- `depth_levels_t` (`small_vector<price_level_t, 20>`) — already optimized.

What actually costs:

- `std::vector<T>` (any `T`) — one heap alloc per non-empty parse.
- `std::optional<std::vector<T>>` — alloc only when present.
- `std::map`, `std::unordered_map`, `std::set` — node-per-entry allocs.
- `std::string` whose typical content is > 15 chars (libstdc++ SSO threshold). Field-level judgement:
  - SSO-safe (drop the entry, but call them out if relevant): 3-letter `asset` codes (`USDT`, `BNB`), enum-ish `status`/`side`/`type` strings (`"FILLED"`, `"BUY"`, `"GTC"`), `timezone` (`"UTC"`).
  - Past-SSO (heap-alloc on parse): `clientOrderId` (Binance default ≥ 22 chars), `apiKey`, `signature`, `listenKey` (~32 chars), download URLs, error `msg`, `reject_reason`, JSON-encoded `batchOrders`.
- `glz::raw_json_view` — non-owning `string_view` (free, but the underlying storage must outlive the value).

Cadence is graded **hot** (per-frame / multiple per second), **warm** (per-active-symbol per minute or per user trade event), **cold** (startup / hourly / operator-driven). Cold types are not worth optimizing.

## Per-file inventory

### `types/market_stream_events.hpp` — WS market data per-frame events

Cadence: **hot** for every event. The variant lives across 13 alternatives; each frame parses exactly one.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap |
|---|---|---|---|---|
| `book_ticker_stream_event_t` | — | — | — | **0** |
| `aggregate_trade_stream_event_t` | — | — | — | **0** |
| `mark_price_stream_event_t` | — | — | — | **0** |
| `depth_stream_event_t` | bids, asks (small_vector ≤ 20) | — | — | **0** in steady state, heap only on > 20-level bursts |
| `mini_ticker_stream_event_t` | — | — | — | **0** |
| `ticker_stream_event_t` | — | — | — | **0** |
| `liquidation_order_stream_event_t` | — | — | — | **0** |
| `kline_stream_event_t` | — | — | — | **0** |
| `continuous_contract_kline_stream_event_t` | — | — | — | **0** |
| `composite_index_stream_event_t` | composition | — | optional<string> base_asset_type | 1 vector / event (warm; per composite-index symbol per minute) |
| `contract_info_bracket_t` (nested) | — | — | — | **0** |
| `contract_info_stream_event_t` | — | — | optional<vector<bracket>> brackets | 1 vector / event when present (warm; per contract-info push) |
| `asset_index_stream_event_t` | — | — | — | **0** |
| `trading_session_stream_event_t` | — | — | — | **0** |

Notes:
- `composite_index_constituent_t.base_asset` / `quote_asset` are short asset codes, SSO-safe.
- The `all_market_*` aliases (`std::vector<X>`) materialize as top-level vectors at parse — treated under "REST list responses" semantics below; cadence is per-second per-market-tick, so warm-to-hot.

### `types/user_stream_events.hpp` — user data stream events

Cadence: **warm** in active sessions (driven by user activity); the parser materializes one variant alternative per event.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap |
|---|---|---|---|---|
| `account_update_balance_t` (nested) | — | — | — | **0** (`asset` SSO) |
| `account_update_position_t` (nested) | — | — | — | **0** |
| `account_update_data_t` (nested) | balances, positions | — | — | 2 vectors / event |
| `account_update_event_t` | — | — | — | inherits 2 from nested data |
| `order_trade_update_order_t` (nested) | — | client_order_id (~22), expiry_reason (opt) | — | 1 string / order (commission_asset SSO) |
| `order_trade_update_event_t` | — | — | — | inherits 1 from nested order |
| `margin_call_position_t` (nested) | — | — | — | **0** |
| `margin_call_event_t` | positions | — | — | 1 vector / event |
| `listen_key_expired_event_t` | — | listen_key (~32) | — | 1 string / event (rare) |
| `account_config_leverage_t` (nested) | — | — | — | **0** |
| `account_config_multi_assets_t` (nested) | — | — | — | **0** |
| `account_config_update_event_t` | — | — | 2× optional<nested> | **0** |
| `trade_lite_event_t` | — | client_order_id (~22) | — | 1 string / event |
| `algo_order_update_data_t` (nested) | — | client_algo_id (~22), matched_order_id (opt), reject_reason (opt) | — | 1–3 strings / event |
| `algo_order_update_event_t` | — | — | — | inherits 1–3 from nested |
| `conditional_order_reject_data_t` (nested) | — | reject_reason | — | 1 string / event |
| `conditional_order_trigger_reject_event_t` | — | — | — | inherits 1 |
| `grid_update_event_t` | — | — | — | **0** |
| `strategy_update_event_t` | — | — | — | **0** |

### `types/market_data.hpp` — REST market data responses

Cadence varies wildly per endpoint — see the cadence column.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap | Cadence |
|---|---|---|---|---|---|
| `order_book_response_t` | bids, asks (`std::vector<price_level_t>`) | — | — | 2 vectors | warm — periodic resync (every few minutes per active symbol) |
| `recent_trade_t` | — | — | — | **0** | per-call (whole list is `vector<recent_trade_t>` — see REST list responses below) |
| `aggregate_trade_t` | — | — | — | **0** | per-call (list response) |
| `kline_t` | — | — | — | **0** | per-minute per active symbol (list response) |
| `book_ticker_t` | — | — | — | **0** | warm |
| `price_ticker_t` | — | — | — | **0** | warm |
| `ticker_24hr_t` | — | — | — | **0** | per-minute |
| `mark_price_t` | — | — | — | **0** | per-minute |
| `funding_rate_history_entry_t` | — | — | — | **0** | cold (historical query) |
| `funding_rate_info_t` | — | — | — | **0** | cold |
| `open_interest_t` | — | — | — | **0** | per-minute |
| `open_interest_statistics_entry_t` | — | — | — | **0** | per-minute (list response) |
| `long_short_ratio_entry_t` | — | — | — | **0** | per-minute (list response) |
| `taker_buy_sell_volume_entry_t` | — | — | — | **0** | per-minute (list response) |
| `basis_entry_t` | — | — | — | **0** | per-minute (list response) |
| `delivery_price_entry_t` | — | — | — | **0** | cold (operator-driven) |
| `composite_index_base_asset_t` (nested) | — | — | — | **0** (`baseAsset`/`quoteAsset` SSO) | nested in composite_index_info |
| `composite_index_info_t` | baseAssetList | component | — | 1 vector + 1 string / parse | cold |
| `index_constituent_t` (nested) | — | exchange (typically ≥ 16, e.g. `"binance-futures"`) | — | possibly 1 string | nested in index_constituents |
| `index_constituents_response_t` | constituents | — | — | 1 vector + N strings | cold |
| `asset_index_t` | — | — | — | **0** | per-minute (list response) |
| `insurance_fund_asset_t` (nested) | — | — | — | **0** (`asset` SSO) | nested |
| `insurance_fund_response_t` | symbols (`vector<string>`), assets | — | — | 2 vectors + N short strings | cold (operator-driven) |
| `adl_risk_entry_t` | — | — | — | **0** | warm |
| `trading_session_entry_t` (nested) | — | — | — | **0** (`type` SSO) | nested |
| `market_schedule_t` (nested) | sessions | — | — | 1 vector / market | nested |
| `trading_schedule_response_t` | — | — | `std::map<std::string, market_schedule_t>` marketSchedules | N tree-nodes + nested vectors | cold (per-day) |

### `types/account.hpp` — REST account responses

Cadence: **warm** at most (per-login or per-poll); never per-frame.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap | Cadence |
|---|---|---|---|---|---|
| `account_asset_t` (nested) | — | — | — | **0** (`asset` SSO) | nested |
| `account_position_t` (nested) | — | — | — | **0** | nested |
| `account_information_t` | assets, positions | — | — | 2 vectors / fetch | warm (per-login or per-poll) |
| `futures_account_balance_t` | — | — | — | **0** (`accountAlias`/`asset` SSO) | warm (list response) |
| `position_risk_t` | — | — | — | **0** | warm (list response) |
| `account_config_response_t` | — | — | — | **0** | warm (per-login) |
| `symbol_config_entry_t` | — | — | — | **0** | warm (list response) |
| `multi_assets_mode_response_t` | — | — | — | **0** | cold |
| `position_mode_response_t` | — | — | — | **0** | cold |
| `income_history_entry_t` | — | — | — | **0** (`asset`/`info`/`tradeId` SSO in normal cases) | cold (historical query) |
| `leverage_bracket_entry_t` (nested) | — | — | — | **0** | nested |
| `symbol_leverage_brackets_t` | brackets | — | — | 1 vector / symbol | warm (per-login, occasionally re-queried) |
| `commission_rate_response_t` | — | — | — | **0** | cold |
| `download_id_response_t` | — | downloadId (UUID-ish, ~36) | — | 1 string | cold |
| `download_link_response_t` | — | downloadId, status, url | — | up to 3 strings | cold |
| `bnb_burn_status_response_t` | — | — | — | **0** | cold |
| `trading_status_indicator_t` (nested) | — | indicator (e.g. `"UFR"` SSO) | — | **0** | nested |
| `quantitative_rules_response_t` | — | — | `std::map<std::string, std::vector<trading_status_indicator_t>>` indicators | N tree-nodes + nested vectors | warm (per-login + per-poll) |
| `pm_account_info_response_t` | — | — | — | **0** (`asset` SSO) | cold |

### `types/trade.hpp` — REST trade responses (request-only structs omitted)

Cadence: **warm** during active trading; nothing per-frame.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap | Cadence |
|---|---|---|---|---|---|
| `order_response_t` | — | clientOrderId (~22) | — | 1 string / order | warm (per place / query / modify; bursty during admission) |
| `code_msg_response_t` | — | msg | — | 1 string / response | per error / per cancel-all |
| `auto_cancel_response_t` | — | — | — | **0** | cold |
| `position_risk_v3_t` | — | marginAsset, isolatedWallet (both look short but isolatedWallet is a decimal-as-string, often > 15 chars) | — | up to 2 strings | warm (list response) |
| `adl_quantile_values_t` (nested) | — | — | — | **0** | nested |
| `adl_quantile_entry_t` | — | — | — | **0** | warm (list response) |
| `account_trade_entry_t` | — | — | — | **0** (`commissionAsset` SSO) | warm (list response, per-trade history fetch) |
| `change_leverage_response_t` | — | — | — | **0** | cold |
| `modify_isolated_margin_response_t` | — | msg | — | 1 string | cold |
| `position_margin_history_entry_t` | — | — | — | **0** (`asset` SSO) | cold (list response) |
| `algo_order_response_t` | — | clientAlgoId (opt, ~22) | — | up to 1 string / order | warm |

### `types/websocket_api.hpp` — WS-API responses (request types skipped)

Cadence: per-RPC-call. Trading-RPC traffic is warm during admission, cold otherwise.

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap |
|---|---|---|---|---|
| `websocket_api_error_t` | — | msg | — | 1 string / error |
| `websocket_api_status_t` | — | id (caller-chosen, often UUID/long) | — | 1 string |
| `session_logon_result_t` | — | apiKey (opt, ~64) | — | up to 1 string |
| `websocket_api_listen_key_result_t` | — | listenKey (~32) | — | 1 string |
| `websocket_api_response_t<T>` | — | id | optional<vector<rate_limit_t>> rateLimits, optional<websocket_api_error_t> error | 1 string + 1 vector when rateLimits present |

### `types/common.hpp` — shared primitives

| Type | `std::vector<T>` | `std::string` (past-SSO) | Other dynamic | Per-parse heap | Cadence |
|---|---|---|---|---|---|
| `rate_limit_t` (nested) | — | — | — | **0** | nested in WS-API responses + exchange_info |
| `server_time_response_t` | — | — | — | **0** | cold |
| `binance_error_document_t` | — | msg | — | 1 string | per-error |
| `price_level_t` (nested) | — | — | — | **0** | nested |
| `exchange_info_asset_t` (nested) | — | — | — | **0** (`asset` SSO; `autoAssetExchange` opt SSO) | nested in exchange_info |
| `symbol_filter_t` (nested) | — | filterType (e.g. `"MAX_NUM_ALGO_ORDERS"` ~17) | — | up to 1 string | nested |
| `symbol_info_t` (nested) | underlyingSubType (`vector<string>`), filters | underlyingType | — | 2 vectors + N strings | nested in exchange_info |
| `exchange_info_response_t` | exchangeFilters (`vector<glz::generic>`), rateLimits, assets, symbols | timezone (SSO) | — | 4 top-level vectors + dozens of nested | **cold** — startup + ~hourly refresh |
| `listen_key_response_t` | — | listenKey (~32) | — | 1 string | cold (per-listen-key roll) |
| `combined_stream_frame_t` | — | stream | `glz::raw_json_view` data (non-owning) | 1 string per frame | **hot** — every WebSocket frame |

### `types/convert.hpp` — convert API responses

All cold (per-quote, per-accept, per-status).

| Type | Heap members | Per-parse |
|---|---|---|
| `convert_quote_response_t` | quoteId | 1 string |
| `convert_accept_response_t` | orderId, orderStatus | up to 2 strings |
| `convert_order_status_response_t` | orderId, orderStatus, fromAsset, toAsset | up to 4 strings (assets SSO) |

### REST list responses (declared in `rest/endpoint_traits.hpp`)

These are not types in `types/*.hpp` — they are top-level `std::vector<entry_t>` typedefs used as `endpoint_traits<R>::response_type_t`. Each call materializes one outer vector + N inner allocations.

| Endpoint | Outer container | Cadence | Notes |
|---|---|---|---|
| `recent_trades` / `historical_trades` | `vector<recent_trade_t>` | warm | Per-symbol periodic |
| `aggregate_trades` | `vector<aggregate_trade_t>` | warm | Per-symbol periodic |
| `klines` / `mark_price_klines` / `premium_index_klines` / `continuous_klines` | `vector<kline_t>` | per-minute per active symbol | Outer vector ~500 entries by default — single big alloc + reserve avoids reallocation |
| `funding_rate_history` | `vector<funding_rate_history_entry_t>` | cold | |
| `open_interest_statistics` / `long_short_ratio` / `taker_buy_sell_volume` / `basis` | `vector<*_entry_t>` | per-minute | |
| `delivery_price` | `vector<delivery_price_entry_t>` | cold | |
| `composite_index` (no symbol) | `vector<composite_index_info_t>` | cold | |
| `asset_indices` (no symbol) | `vector<asset_index_t>` | per-minute | |
| `insurance_funds` (no symbol) | `vector<insurance_fund_response_t>` | cold | |
| `adl_risks` | `vector<adl_risk_entry_t>` | warm | |
| `position_risk_v2/v3` | `vector<position_risk_t/_v3_t>` | warm | |
| `symbol_config` | `vector<symbol_config_entry_t>` | warm | |
| `income_history` | `vector<income_history_entry_t>` | cold | |
| `leverage_brackets` | `vector<symbol_leverage_brackets_t>` | warm | Each entry has its own nested `brackets` vector → N+1 allocs per fetch |
| `account_trades` | `vector<account_trade_entry_t>` | warm | |
| `position_margin_history` / `force_orders` / `all_orders` / `all_open_orders` | `vector<*>` | warm | |
| `all_algo_orders` / `open_algo_orders` | `vector<algo_order_response_t>` | warm | |

### WebSocket frame transport

Every inbound frame from `transport::websocket_client::async_read_text` is materialized as a `std::string` of the raw JSON before parse. Frame bodies on the futures venue run **500–4 000 bytes**, always past SSO. At 25 active symbols with a depth + book-ticker + agg-trade subscription each, this is **~250–500 frames/sec ⇒ ~250–500 std::string heap allocs/sec just to receive the wire bytes**, before any of the typed parses above run.

This is the single biggest allocation source on the WS hot path (matches the sqf recorder audit's findings — see [`plans/topics/recorder-hot-path-allocations.md`](../../../../plans/topics/recorder-hot-path-allocations.md)).

## Findings

1. **Plan 001 step 1 collapsed the per-frame depth allocations to zero in steady state.** The remaining allocation pressure on the WS market-data path comes from:
   - `composite_index_stream_event_t.composition` and `contract_info_stream_event_t.brackets` — both warm, not hot. Optimizing them is low-value.
   - The raw frame `std::string` in the transport layer — see finding 5.

2. **REST market-data list responses dominate non-WS allocation.** `klines` materializes a 500-entry `vector<kline_t>` each call; `position_risk` / `account_trades` similar. Every entry is allocation-free internally (`decimal_t` everywhere), so the outer vector is the entire cost — a single allocation per fetch. Not worth optimizing in isolation.

3. **The `_clientOrderId` string family is a hidden steady cost during active trading.** `order_response_t.clientOrderId`, `trade_lite_event_t.client_order_id`, `order_trade_update_order_t.client_order_id`, `algo_order_update_data_t.client_algo_id`, the WS-API `id` echo. Binance defaults are 22-char alphanumeric, which spills past SSO — every order parse pays one heap alloc for it. At HFT-style order rates (10s of orders/sec) this becomes meaningful; at typical trading rates (orders/minute) it's noise.

4. **`account_information_t` and `account_update_data_t` each produce two vectors per parse** (`assets` + `positions`). `assets` is bounded by the number of held assets (≤ ~20 in practice), `positions` by the number of trading symbols (≤ ~50 typical). Could become `small_vector` with N=32, but cadence is warm, not hot — payoff is small.

5. **The `combined_stream_frame_t` raw frame string is the dominant binapi2-side allocation on the WS path.** Same conclusion as the sqf recorder audit. Fixing it requires a deeper change to `transport::websocket_client` + `streams::detail::stream_consumer`'s buffer type — it is not localized to the type definitions in this audit's scope.

6. **`exchange_info_response_t` is genuinely big** (4 top-level vectors, dozens of nested) but cold (startup + hourly). Don't touch.

## Recommendations

Priority order; each is a separate follow-up plan to be filed under `plans/`, not bundled with the audit.

1. **Frame-string pooling in the WS transport** (cross-cuts beyond `types/`). Replace `std::string` with `boost::container::small_vector<char, 2048>` or a thread-local arena reused across frames. **High value (~250–500 allocs/sec), high effort** — touches `transport::websocket_client::async_read_text`, `streams::detail::stream_consumer`, and the `combined_stream_frame_t.stream` field. File as `plans/002-ws-frame-transport-pooling.md` when scheduled.

2. **`account_update_data_t` `balances`/`positions` → `small_vector<T, 32>`.** Same pattern as plan 001. Bounds are venue-defined: `assets` ≤ ~20 supported margin assets; `positions` ≤ active symbol count. Inline 32 covers both common cases without bloating `account_update_event_t` past a cache line catastrophically. Cadence is warm (per user-trade burst), so payoff is per-active-trading-session, not per-frame. **Medium-low value, small effort**. File as `plans/003-account-update-vectors-to-small-vector.md`.

3. **`margin_call_event_t.positions` → `small_vector<margin_call_position_t, 8>`.** Margin-call events are rare but spike during liquidation cascades; positions per event is low single digits. **Low value, trivial effort** — bundle with (2) if ever filed.

4. **Reserve outer vectors at `klines` / `account_trades` / `position_risk` parse sites.** Glaze's array parser already reserves based on JSON-array size discovery; verify in `stream_parse_benchmark` it actually does for the kline 500-entry case. If not, a per-endpoint reserve hint is a one-line change. **Tiny value, trivial effort** — only worth doing when touching that code for other reasons.

5. **`order_response_t.clientOrderId`, `trade_lite_event_t.client_order_id`, etc. → `boost::container::small_string<24>` or a `std::array<char, 36>` wrapper.** Binance default IDs are 22 chars; with a 24-char inline buffer, every order parse becomes alloc-free. Requires a strong-type wrapper in `types/detail/` similar to `symbol_t`. **Medium value at HFT rates, low value at trading rates, medium effort**. File as `plans/004-client-order-id-fixed-buffer.md` only if order throughput becomes a measured concern.

## Don't touch

These are intentionally not on the recommendation list — optimizing them costs reader/maintenance time without runtime payoff:

- **`exchange_info_response_t`** and everything reachable through it (`symbol_info_t`, `symbol_filter_t`, etc.) — cold (startup / hourly).
- **All convert API responses** — cold (per-RPC operator action).
- **`download_*_response_t`, `funding_rate_history_*`, `delivery_price_*`, `income_history_*`, `force_orders`** — cold historical / operator queries.
- **`listen_key_response_t`, `listen_key_expired_event_t.listen_key`** — once-per-listen-key-roll (every 60 minutes). Rare.
- **`code_msg_response_t.msg`, `binance_error_document_t.msg`, `*reject_reason*`, `websocket_api_error_t.msg`** — error path; if you're hitting these often enough for the alloc to matter, the alloc is the wrong problem.
- **`quantitative_rules_response_t.indicators` map** — operator-driven query, infrequent.
- **`trading_schedule_response_t.marketSchedules` map** — per-day fetch.
- **`exchangeFilters: vector<glz::generic>`** — opaque dynamic JSON; changing this would mean parsing it into a typed shape, which is a feature, not an allocation fix.

## Takeaway

After plan 001 step 1, the type-level allocation pressure in `binapi2::fapi` is concentrated in three places, in priority order:

1. The raw WS frame `std::string` in `transport::websocket_client` (≥ 250 allocs/sec at 25 active symbols).
2. The two vectors inside `account_update_data_t` and the (rare) margin-call vector — only meaningful at user-trade burst rates.
3. The `clientOrderId` family — only meaningful at HFT order rates.

Everything else is either already alloc-free, or cold enough that optimizing it is wasted effort. The follow-up plans should be filed individually, not bundled, so each can be measured against `stream_parse_benchmark` / `rest_benchmark` independently.
