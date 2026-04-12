# Demo CLI: API Coverage

Complete reference for the `binapi2-fapi-async-demo-cli` command set and its
coverage of the binapi2 USD-M Futures library surface.

**Total commands: 135** (39 market data + 21 account + 29 trade + 3 convert +
16 WebSocket API + 22 market streams + 4 user data streams + 2 order book)

---

## Authentication

Endpoints declare a `security_type_t` that determines what credentials are
required. The signing method (Ed25519 or HMAC-SHA256) is a config-level choice
(`config::sign_method`), defaulting to Ed25519.

| Auth | `security_type_t` | Credentials | Description |
|------|-------------------|-------------|-------------|
| none | `none`, `market_data` | -- | Public data, no credentials required |
| api-key | `user_stream` | API key header | Listen key lifecycle; no signature |
| signed | `user_data`, `trade` | API key + signature | Timestamp, recvWindow, and Ed25519 (default) or HMAC-SHA256 signature appended to query |

---

## Usage

```
binapi2-fapi-async-demo-cli [flags] <command> [args...]
```

| Flag | Description |
|------|-------------|
| `-v` | Print full JSON responses |
| `-vv` | JSON + transport log (method, target, status, body) |
| `-vvv` | JSON + full HTTP with headers |
| `-l`, `--live` | Use production endpoints (default: testnet) |
| `--testnet` | Use testnet endpoints (default) |
| `-S`, `--save-request <file>` | Save request to file |
| `-R`, `--save-response <file>` | Save response body to file |
| `-r`, `--record <file>` | Record raw stream frames to JSONL file |
| `-K`, `--secrets <source>` | Secret source: `libsecret:<profile>`, `env`, `systemd-creds:<dir>` |
| `-L`, `--log-file <file>` | Log to file |
| `-F`, `--file-loglevel <lvl>` | File log level (trace/debug/info/warn/error/off) |
| `-O`, `--stdout-loglevel <lvl>` | Stdout log level |
| `-h`, `--help` | Print help |

### Testnet scripts

| Script | Coverage |
|--------|----------|
| `scripts/testnet/market_data.sh` | All 39 market data commands |
| `scripts/testnet/account.sh` | 14 of 21 account commands (excludes download + toggle-bnb-burn) |
| `scripts/testnet/trade.sh` | 12 of 29 trade commands (read-only + test-order) |
| `scripts/testnet/convert.sh` | 2 of 3 convert commands (excludes convert-accept) |
| `scripts/testnet/ws_api.sh` | 11 of 16 WebSocket API commands |
| `scripts/testnet/streams.sh` | All 22 market stream commands |
| `scripts/testnet/user_streams.sh` | All 4 user data stream commands |
| `scripts/testnet/order_book.sh` | Both order book commands |

---

## REST: Market Data (39 commands)

| # | Command | Request type | Args | Auth |
|---|---------|-------------|------|------|
| 1 | `ping` | `ping_request_t` | -- | none |
| 2 | `time` | `time_request_t` | -- | none |
| 3 | `exchange-info` | `exchange_info_request_t` | `[symbol]` | none |
| 4 | `order-book` | `order_book_request_t` | `<symbol> [limit]` | none |
| 5 | `recent-trades` | `recent_trades_request_t` | `<symbol> [limit]` | none |
| 6 | `aggregate-trades` | `aggregate_trades_request_t` | `<symbol> [limit]` | none |
| 7 | `historical-trades` | `historical_trades_request_t` | `<symbol> [limit]` | none |
| 8 | `book-ticker` | `book_ticker_request_t` | `<symbol>` | none |
| 9 | `book-tickers` | `book_tickers_request_t` | -- | none |
| 10 | `price-ticker` | `price_ticker_request_t` | `<symbol>` | none |
| 11 | `price-tickers` | `price_tickers_request_t` | -- | none |
| 12 | `price-ticker-v2` | `price_ticker_v2_request_t` | `<symbol>` | none |
| 13 | `price-tickers-v2` | `price_tickers_v2_request_t` | -- | none |
| 14 | `ticker-24hr` | `ticker_24hr_request_t` | `<symbol>` | none |
| 15 | `ticker-24hrs` | `ticker_24hrs_request_t` | -- | none |
| 16 | `mark-price` | `mark_price_request_t` | `<symbol>` | none |
| 17 | `mark-prices` | `mark_prices_request_t` | -- | none |
| 18 | `klines` | `klines_request_t` | `<symbol> <interval> [limit]` | none |
| 19 | `continuous-kline` | `continuous_kline_request_t` | `<pair> <interval> [limit]` | none |
| 20 | `index-price-kline` | `index_price_kline_request_t` | `<pair> <interval> [limit]` | none |
| 21 | `mark-price-klines` | `mark_price_klines_request_t` | `<symbol> <interval> [limit]` | none |
| 22 | `premium-index-klines` | `premium_index_klines_request_t` | `<symbol> <interval> [limit]` | none |
| 23 | `funding-rate` | `funding_rate_request_t` | `<symbol> [limit]` | none |
| 24 | `funding-rate-info` | `funding_rate_info_request_t` | -- | none |
| 25 | `open-interest` | `open_interest_request_t` | `<symbol>` | none |
| 26 | `open-interest-stats` | `open_interest_statistics_request_t` | `<symbol> <period> [limit]` | none |
| 27 | `top-ls-account-ratio` | `top_long_short_account_ratio_request_t` | `<symbol> <period> [limit]` | none |
| 28 | `top-ls-trader-ratio` | `top_trader_long_short_ratio_request_t` | `<symbol> <period> [limit]` | none |
| 29 | `long-short-ratio` | `long_short_ratio_request_t` | `<symbol> <period> [limit]` | none |
| 30 | `taker-volume` | `taker_buy_sell_volume_request_t` | `<symbol> <period> [limit]` | none |
| 31 | `basis` | `basis_request_t` | `<pair> <period> [limit]` | none |
| 32 | `delivery-price` | `delivery_price_request_t` | `<pair>` | none |
| 33 | `composite-index-info` | `composite_index_info_request_t` | `[symbol]` | none |
| 34 | `index-constituents` | `index_constituents_request_t` | `<symbol>` | none |
| 35 | `asset-index` | `asset_index_request_t` | `[symbol]` | none |
| 36 | `insurance-fund` | `insurance_fund_request_t` / `insurance_funds_request_t` | `[symbol]` | none |
| 37 | `adl-risk` | `adl_risk_request_t` | `[symbol]` | none |
| 38 | `rpi-depth` | `rpi_depth_request_t` | `<symbol> [limit]` | none |
| 39 | `trading-schedule` | `trading_schedule_request_t` | -- | none |

---

## REST: Account (21 commands)

| # | Command | Request type | Args | Auth |
|---|---------|-------------|------|------|
| 1 | `account-info` | `account_info_request_t` | -- | signed |
| 2 | `balances` | `balances_request_t` | -- | signed |
| 3 | `position-risk` | `position_risk_request_t` | `[symbol]` | signed |
| 4 | `income-history` | `income_history_request_t` | `[symbol] [limit]` | signed |
| 5 | `account-config` | `account_config_request_t` | -- | signed |
| 6 | `symbol-config` | `symbol_config_request_t` | `[symbol]` | signed |
| 7 | `multi-assets-mode` | `get_multi_assets_mode_request_t` | -- | signed |
| 8 | `position-mode` | `get_position_mode_request_t` | -- | signed |
| 9 | `rate-limit-order` | `rate_limit_order_request_t` | -- | signed |
| 10 | `leverage-bracket` | `leverage_bracket_request_t` | `[symbol]` | signed |
| 11 | `commission-rate` | `commission_rate_request_t` | `<symbol>` | signed |
| 12 | `bnb-burn` | `get_bnb_burn_request_t` | -- | signed |
| 13 | `toggle-bnb-burn` | `toggle_bnb_burn_request_t` | `<true\|false>` | signed |
| 14 | `quantitative-rules` | `quantitative_rules_request_t` | `[symbol]` | signed |
| 15 | `pm-account-info` | `pm_account_info_request_t` | `<asset>` | signed |
| 16 | `download-id-transaction` | `download_id_transaction_request_t` | `<startTime> <endTime>` | signed |
| 17 | `download-link-transaction` | `download_link_transaction_request_t` | `<downloadId>` | signed |
| 18 | `download-id-order` | `download_id_order_request_t` | `<startTime> <endTime>` | signed |
| 19 | `download-link-order` | `download_link_order_request_t` | `<downloadId>` | signed |
| 20 | `download-id-trade` | `download_id_trade_request_t` | `<startTime> <endTime>` | signed |
| 21 | `download-link-trade` | `download_link_trade_request_t` | `<downloadId>` | signed |

---

## REST: Trade (29 commands)

| # | Command | Request type | Args | Auth |
|---|---------|-------------|------|------|
| 1 | `new-order` | `new_order_request_t` | `<sym> <side> <type> [-q Q] [-p P] [-t TIF]` | signed |
| 2 | `test-order` | `test_order_request_t` | (same as new-order) | signed |
| 3 | `modify-order` | `modify_order_request_t` | `<sym> <side> <orderId> -q Q -p P` | signed |
| 4 | `cancel-order` | `cancel_order_request_t` | `<symbol> <orderId>` | signed |
| 5 | `cancel-multiple-orders` | `cancel_multiple_orders_request_t` | `<symbol> <id1,id2,...>` | signed |
| 6 | `cancel-all-orders` | `cancel_all_open_orders_request_t` | `<symbol>` | signed |
| 7 | `auto-cancel` | `auto_cancel_request_t` | `<symbol> <countdownMs>` | signed |
| 8 | `query-order` | `query_order_request_t` | `<symbol> <orderId>` | signed |
| 9 | `query-open-order` | `query_open_order_request_t` | `<symbol> <orderId>` | signed |
| 10 | `open-orders` | `open_orders_request_t` | `[symbol]` | signed |
| 11 | `all-orders` | `all_orders_request_t` | `<symbol> [limit]` | signed |
| 12 | `position-info-v3` | `position_info_v3_request_t` | `[symbol]` | signed |
| 13 | `adl-quantile` | `adl_quantile_request_t` | `[symbol]` | signed |
| 14 | `force-orders` | `force_orders_request_t` | `[symbol] [limit]` | signed |
| 15 | `account-trades` | `account_trade_request_t` | `<symbol> [limit]` | signed |
| 16 | `change-position-mode` | `change_position_mode_request_t` | `<true\|false>` | signed |
| 17 | `change-multi-assets-mode` | `change_multi_assets_mode_request_t` | `<true\|false>` | signed |
| 18 | `change-leverage` | `change_leverage_request_t` | `<symbol> <leverage>` | signed |
| 19 | `change-margin-type` | `change_margin_type_request_t` | `<symbol> <ISOLATED\|CROSSED>` | signed |
| 20 | `modify-isolated-margin` | `modify_isolated_margin_request_t` | `<sym> <amount> <1\|2>` | signed |
| 21 | `position-margin-history` | `position_margin_history_request_t` | `<symbol> [limit]` | signed |
| 22 | `order-modify-history` | `order_modify_history_request_t` | `<symbol> [orderId]` | signed |
| 23 | `new-algo-order` | `new_algo_order_request_t` | `<sym> <side> <type> <algo> -q Q [-p P]` | signed |
| 24 | `cancel-algo-order` | `cancel_algo_order_request_t` | `<algoId>` | signed |
| 25 | `query-algo-order` | `query_algo_order_request_t` | `<algoId>` | signed |
| 26 | `all-algo-orders` | `all_algo_orders_request_t` | `<symbol> [limit]` | signed |
| 27 | `open-algo-orders` | `open_algo_orders_request_t` | -- | signed |
| 28 | `cancel-all-algo-orders` | `cancel_all_algo_orders_request_t` | -- | signed |
| 29 | `tradfi-perps` | `tradfi_perps_request_t` | -- | signed |

---

## REST: Convert (3 commands)

| # | Command | Request type | Args | Auth |
|---|---------|-------------|------|------|
| 1 | `convert-quote` | `convert_quote_request_t` | `<fromAsset> <toAsset> <fromAmount>` | signed |
| 2 | `convert-accept` | `convert_accept_request_t` | `<quoteId>` | signed |
| 3 | `convert-order-status` | `convert_order_status_request_t` | `<orderId>` | signed |

---

## WebSocket API (16 commands)

| # | Command | Request type | Auth | Args |
|---|---------|-------------|------|------|
| 1 | `ws-logon` | (session logon) | signed | -- |
| 2 | `ws-book-ticker` | `websocket_api_book_ticker_request_t` / `websocket_api_book_tickers_request_t` | none | `[symbol]` (empty = all) |
| 3 | `ws-price-ticker` | `websocket_api_price_ticker_request_t` / `websocket_api_price_tickers_request_t` | none | `[symbol]` (empty = all) |
| 4 | `ws-account-status` | `websocket_api_account_status_request_t` | signed | -- |
| 5 | `ws-account-status-v2` | `ws_account_status_v2_request_t` | signed | -- |
| 6 | `ws-account-balance` | `ws_account_balance_request_t` | signed | -- |
| 7 | `ws-order-place` | `websocket_api_new_order_request_t` | signed | `<sym> <side> <type> [-q Q] [-p P] [-t TIF]` |
| 8 | `ws-order-query` | `websocket_api_order_query_request_t` | signed | `<symbol> <orderId>` |
| 9 | `ws-order-modify` | `websocket_api_order_modify_request_t` | signed | `<sym> <side> <orderId> -q Q -p P` |
| 10 | `ws-order-cancel` | `websocket_api_cancel_order_request_t` | signed | `<symbol> <orderId>` |
| 11 | `ws-position` | `websocket_api_position_request_t` | signed | `[symbol]` |
| 12 | `ws-algo-order-place` | `websocket_api_algo_order_place_request_t` | signed | `<sym> <side> <type> <algo> -q Q` |
| 13 | `ws-algo-order-cancel` | `websocket_api_algo_order_cancel_request_t` | signed | `<algoId>` |
| 14 | `ws-user-stream-start` | `ws_user_data_stream_start_request_t` | api-key | -- |
| 15 | `ws-user-stream-ping` | `ws_user_data_stream_ping_request_t` | api-key | -- |
| 16 | `ws-user-stream-stop` | `ws_user_data_stream_stop_request_t` | api-key | -- |

---

## Market Streams (22 commands)

### Per-symbol streams

| # | Command | Subscription type | Args |
|---|---------|------------------|------|
| 1 | `stream-aggregate-trade` | `aggregate_trade_subscription` | `<symbol>` |
| 2 | `stream-book-ticker` | `book_ticker_subscription` | `<symbol>` |
| 3 | `stream-mark-price` | `mark_price_subscription` | `<symbol>` |
| 4 | `stream-kline` | `kline_subscription` | `<symbol> <interval>` |
| 5 | `stream-ticker` | `ticker_subscription` | `<symbol>` |
| 6 | `stream-mini-ticker` | `mini_ticker_subscription` | `<symbol>` |
| 7 | `stream-depth` | `partial_book_depth_subscription` | `<symbol> [levels]` |
| 8 | `stream-diff-depth` | `diff_book_depth_subscription` | `<symbol> [speed]` |
| 9 | `stream-liquidation` | `liquidation_order_subscription` | `<symbol>` |
| 10 | `stream-composite-index` | `composite_index_subscription` | `<symbol>` |
| 11 | `stream-asset-index` | `asset_index_subscription` | `<symbol>` |
| 12 | `stream-continuous-kline` | `continuous_contract_kline_subscription` | `<pair> <interval>` |
| 13 | `stream-rpi-diff-depth` | `rpi_diff_book_depth_subscription` | `<symbol>` |

### All-market streams

| # | Command | Subscription type | Args |
|---|---------|------------------|------|
| 14 | `stream-all-book-tickers` | `all_market_book_ticker_subscription` | -- |
| 15 | `stream-all-tickers` | `all_market_ticker_subscription` | -- |
| 16 | `stream-all-mini-tickers` | `all_market_mini_ticker_subscription` | -- |
| 17 | `stream-all-liquidations` | `all_market_liquidation_order_subscription` | -- |
| 18 | `stream-all-mark-prices` | `all_market_mark_price_subscription` | -- |
| 19 | `stream-all-asset-index` | `all_asset_index_subscription` | -- |

### Meta streams

| # | Command | Subscription type | Args |
|---|---------|------------------|------|
| 20 | `stream-contract-info` | `contract_info_subscription` | -- |
| 21 | `stream-trading-session` | `trading_session_subscription` | -- |

---

## User Data Streams (4 commands)

| # | Command | Request/Subscription type | Args | Auth |
|---|---------|--------------------------|------|------|
| 1 | `listen-key-start` | `start_listen_key_request_t` | -- | signed |
| 2 | `listen-key-keepalive` | `keepalive_listen_key_request_t` | -- | signed |
| 3 | `listen-key-close` | `close_listen_key_request_t` | -- | signed |
| 4 | `user-stream` | variant of 10 event types | -- | signed |

---

## Order Book (2 commands)

| # | Command | Description | Args |
|---|---------|------------|------|
| 1 | `order-book-live` | REST snapshot + stream deltas | `<symbol> [depth]` |
| 2 | `pipeline-order-book-live` | Pipelined, 3 threads | `<symbol> [depth]` |

---

## Known testnet issues

The following commands fail on the Binance USD-M Futures **testnet** due to
server-side limitations. These are not library bugs — the library types and
parsing match the documented production API.

### Endpoints not available on testnet (return `ok` instead of JSON)

7 futures data analytics endpoints return the plain text `ok` (HTTP 200)
instead of a JSON array. These endpoints are not operational on testnet.

| Command | Expected response |
|---------|-------------------|
| `open-interest-stats` | `[{...}]` |
| `top-ls-account-ratio` | `[{...}]` |
| `top-ls-trader-ratio` | `[{...}]` |
| `long-short-ratio` | `[{...}]` |
| `taker-volume` | `[{...}]` |
| `basis` | `[{...}]` |
| `delivery-price` | `[{...}]` |

### Endpoints returning non-standard responses on testnet

| Command | Testnet response | Expected (per docs) | Error |
|---------|-----------------|---------------------|-------|
| `test-order` | `{"status":"","side":"",..}` — empty enum strings | `{"status":"NEW","side":"BUY",..}` | `unexpected_enum` |
| `tradfi-perps` | Plain string `SUCCESS` | `{"code":200,"msg":"success"}` | `expected_brace` |

### Endpoints not implemented on testnet

| Command | HTTP status | Binance code | Message |
|---------|-------------|--------------|---------|
| `pm-account-info` | 404 | -5000 | `Path /fapi/v1/pmAccountInfo is invalid` |

### Endpoints returning server errors on testnet

| Command | HTTP status | Binance code | Message |
|---------|-------------|--------------|---------|
| `quantitative-rules` | 400 | -1000 | `An unknown error occured` |
| `convert-quote` | 500 | -1000 | `internal error` |
| `convert-order-status` | 500 | -1000 | `internal error` |

### Orders requiring balance

| Command | Note |
|---------|------|
| `ws-order-place` | Requires sufficient testnet balance to place |

---

## Source files

| File | Commands | Role |
|------|----------|------|
| `main.cpp` | -- | Entry point, flag parsing, `commands[]` dispatch table |
| `common.hpp/cpp` | -- | Shared state, `handle_result`, `parse_enum`, `find_flag`, executor helpers |
| `cmd_market_data.hpp/cpp` | 39 | Market data REST |
| `cmd_account.hpp/cpp` | 21 | Account REST |
| `cmd_trade.hpp/cpp` | 29 | Trade REST |
| `cmd_convert.hpp/cpp` | 3 | Convert REST |
| `cmd_ws_api.hpp/cpp` | 16 | WebSocket API |
| `cmd_stream.hpp/cpp` | 22 | Market data streams |
| `cmd_user_stream.hpp/cpp` | 4 | User data streams |
| `cmd_order_book.hpp/cpp` | 1 | Live order book |
| `cmd_pipeline_order_book.hpp/cpp` | 1 | Pipelined order book |

## Command taxonomy

```
command
+-- rest_command                          -- create_rest_client + async_execute + handle_result
|   +-- market_data_command              -- dispatches to (*rest)->market_data
|   |   +-- kline_command                -- <symbol|pair> <interval> [limit]  (6 endpoints)
|   |   +-- futures_analytics_command    -- <symbol> <period> [limit]         (5 endpoints)
|   +-- account_command                  -- dispatches to (*rest)->account
|   |   +-- download_command             -- <startTime> <endTime> or <downloadId>  (6 endpoints)
|   +-- trade_command                    -- dispatches to (*rest)->trade
|   +-- convert_command                  -- dispatches to (*rest)->convert
|   +-- user_data_stream_command         -- dispatches to (*rest)->user_data_streams
+-- ws_api_command                       -- create_ws_api_client + connect + execute + close
|   +-- ws_public_command                -- no logon required
|   +-- ws_signed_command                -- logon + execute
+-- market_stream_command                -- create_market_stream + subscribe + event loop
+-- user_stream_command                  -- listen key + subscribe + variant event loop
+-- order_book_command                   -- REST snapshot + stream deltas
```
