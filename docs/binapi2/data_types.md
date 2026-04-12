# FAPI Data Types

Catalog of all C++ data types in `include/binapi2/fapi/types/` with references to API documentation and implementation status.

Status meanings:
- **complete** = all documented fields are implemented
- **partial** = some documented fields are missing
- **extra** = has fields beyond what docs specify (not a bug, noted for reference)


## Enums

Source: `include/binapi2/fapi/types/enums.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `security_type_t (internal) | complete | Not from API docs |
| `order_side_t` | [common-definition.md] | complete | |
| `order_type_t` | [common-definition.md] | complete | |
| `time_in_force_t` | [common-definition.md] | complete | |
| `kline_interval_t` | [common-definition.md] | complete | |
| `position_side_t` | [common-definition.md] | complete | |
| `working_type_t` | [common-definition.md] | complete | |
| `response_type_t` | [common-definition.md] | complete | |
| `margin_type_t` | [common-definition.md] | complete | |
| `contract_type_t` | [common-definition.md] | complete | Extra: `tradifi_perpetual` (undocumented) |
| `contract_status_t` | [common-definition.md] | complete | |
| `order_status_t` | [common-definition.md] | complete | |
| `stp_mode_t` | [common-definition.md] | complete | |
| `price_match_t` | [common-definition.md] | complete | |
| `income_type_t` | [common-definition.md] | complete | |
| `futures_data_period_t` | [common-definition.md] | complete | |

[common-definition.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md


## Strong types

Source: `include/binapi2/fapi/types/detail/symbol.hpp`, `include/binapi2/fapi/types/detail/pair.hpp`, `include/binapi2/fapi/types/detail/decimal.hpp`

| Type | Source | Notes |
|---|---|---|
| `symbol_t` | `types/detail/symbol.hpp` | Strong type for trading symbols (e.g., "BTCUSDT"). Constructors normalize input to uppercase — Binance REST accepts any case but streams require lowercase topics, handled by `stream_traits`. |
| `pair_t` | `types/detail/pair.hpp` | Strong type for trading pairs (e.g., "BTCUSDT"). Same uppercase normalization as `symbol_t`. |
| `decimal_t` | `types/detail/decimal.hpp` | String-backed 128-bit fixed-point decimal. JSON deserializer accepts both quoted strings and unquoted numbers. Has `fmt::formatter<decimal_t>` for spdlog/fmt integration. |
| `timestamp_ms_t` | `types/detail/timestamp.hpp` | Millisecond Unix timestamp wrapper over `std::uint64_t`. |
| `enum_set<E>` | `types/detail/enum_set.hpp` | Compact bitset for enum flag sets with JSON array serialization. |


## Request tags

Source: `include/binapi2/fapi/types/request_tags.hpp`

Empty tag base types for service-level request type categorization. Each request type's endpoint_traits carries the tag of the service it belongs to. Used by per-service concepts to constrain `async_execute`.

| Tag | Service |
|---|---|
| `rest_market_data_tag` | `rest::market_data_service` |
| `rest_account_tag` | `rest::account_service` |
| `rest_trade_tag` | `rest::trade_service` |
| `rest_convert_tag` | `rest::convert_service` |
| `rest_user_data_stream_tag` | `rest::user_data_stream_service` |
| `ws_api_tag` | `websocket_api::client` |
| `stream_tag` | `streams::market_streams` |


## Subscription types

Source: `include/binapi2/fapi/types/subscriptions.hpp`

Subscription parameter types for WebSocket market streams. Each subscription type has a corresponding `stream_traits` specialization (in `streams/stream_traits.hpp`) that maps it to a target URL and event type. Used by `market_streams::subscribe()`.


## Common types

Source: `include/binapi2/fapi/types/common.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `empty_response_t` | (internal) | complete | |
| `rate_limit_t` | [common-definition.md] | complete | |
| `server_time_response_t` | [Check-Server-Time.md] | complete | |
| `binance_error_document_t` | [error-code.md] | complete | |
| `price_level_t` | [Order-Book.md] | complete | |
| `exchange_info_asset_t` | [Exchange-Information.md] | complete | |
| `symbol_filter_t` | [Exchange-Information.md] | complete | |
| `symbol_info_t` | [Exchange-Information.md] | complete | |
| `exchange_info_response_t` | [Exchange-Information.md] | complete | |
| `listen_key_response_t` | [Start-User-Data-Stream.md] | complete | |

[Check-Server-Time.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
[error-code.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
[Order-Book.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
[Exchange-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
[Start-User-Data-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream.md


## Market data types

Source: `include/binapi2/fapi/types/market_data.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `ping_request_t` | [Check-Server-Time.md] | complete | |
| `server_time_request_t` | [Check-Server-Time.md] | complete | |
| `exchange_info_request_t` | [Exchange-Information.md] | complete | |
| `order_book_request_t` | [Order-Book.md] | complete | |
| `order_book_response_t` | [Order-Book.md] | complete | |
| `recent_trades_request_t` | [Recent-Trades-List.md] | complete | |
| `recent_trade_t` | [Recent-Trades-List.md] | complete | |
| `aggregate_trades_request_t` | [Compressed-Aggregate-Trades-List.md] | complete | |
| `aggregate_trade_t` | [Compressed-Aggregate-Trades-List.md] | complete | |
| `historical_trades_request_t` | [Old-Trades-Lookup.md] | complete | |
| `kline_request_t` | [Kline-Candlestick-Data.md] | complete | |
| `continuous_kline_request_t` | [Continuous-Contract-Kline.md] | complete | |
| `index_price_kline_request_t` | [Index-Price-Kline.md] | complete | |
| `kline_t` | [Kline-Candlestick-Data.md] | complete | |
| `book_ticker_request_t` | [Symbol-Order-Book-Ticker.md] | complete | |
| `book_ticker_t` | [Symbol-Order-Book-Ticker.md] | complete | Extra: `lastUpdateId` not in doc |
| `price_ticker_request_t` | [Symbol-Price-Ticker.md] | complete | |
| `price_ticker_t` | [Symbol-Price-Ticker.md] | complete | |
| `ticker_24hr_request_t` | [24hr-Ticker.md] | complete | |
| `ticker_24hr_t` | [24hr-Ticker.md] | complete | |
| `mark_price_request_t` | [Mark-Price.md] | complete | |
| `mark_price_t` | [Mark-Price.md] | complete | |
| `funding_rate_history_request_t` | [Get-Funding-Rate-History.md] | complete | |
| `funding_rate_history_entry_t` | [Get-Funding-Rate-History.md] | complete | |
| `funding_rate_info_t` | [Get-Funding-Rate-Info.md] | complete | |
| `open_interest_request_t` | [Open-Interest.md] | complete | |
| `open_interest_t` | [Open-Interest.md] | complete | |
| `futures_data_request_t` | [Open-Interest-Statistics.md] | complete | |
| `open_interest_statistics_entry_t` | [Open-Interest-Statistics.md] | complete | |
| `long_short_ratio_entry_t` | [Long-Short-Ratio.md] | complete | Also used for Top-Long-Short-Account-Ratio, Top-Trader-Long-Short-Ratio |
| `taker_buy_sell_volume_entry_t` | [Taker-BuySell-Volume.md] | complete | |
| `basis_request_t` | [Basis.md] | complete | |
| `basis_entry_t` | [Basis.md] | complete | |
| `price_ticker_v2_request_t` | [Symbol-Price-Ticker-v2.md] | complete | |
| `delivery_price_request_t` | [Delivery-Price.md] | complete | |
| `delivery_price_entry_t` | [Delivery-Price.md] | complete | |
| `composite_index_info_request_t` | [Composite-Index.md] | complete | |
| `composite_index_base_asset_t` | [Composite-Index.md] | complete | |
| `composite_index_info_t` | [Composite-Index.md] | complete | |
| `index_constituents_request_t` | [Index-Constituents.md] | complete | |
| `index_constituent_t` | [Index-Constituents.md] | complete | |
| `index_constituents_response_t` | [Index-Constituents.md] | complete | |
| `asset_index_request_t` | [Multi-Assets-Mode-Asset-Index.md] | complete | |
| `asset_index_t` | [Multi-Assets-Mode-Asset-Index.md] | complete | |
| `insurance_fund_request_t` | [Insurance-Fund-Balance.md] | complete | |
| `insurance_fund_asset_t` | [Insurance-Fund-Balance.md] | complete | |
| `insurance_fund_response_t` | [Insurance-Fund-Balance.md] | complete | |
| `adl_risk_request_t` | [ADL-Risk.md] | complete | |
| `adl_risk_entry_t` | [ADL-Risk.md] | complete | |
| `rpi_depth_request_t` | [Order-Book-RPI.md] | complete | |
| `trading_schedule_request_t` | [Trading-Schedule.md] | complete | |
| `trading_session_entry_t` | [Trading-Schedule.md] | complete | |
| `market_schedule_t` | [Trading-Schedule.md] | complete | |
| `trading_schedule_response_t` | [Trading-Schedule.md] | complete | |

Note: Mark-Price-Kline-Candlestick-Data and Premium-Index-Kline-Data endpoints have no dedicated request types; they reuse `kline_request`/`kline` at the endpoint level.

[Recent-Trades-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
[Compressed-Aggregate-Trades-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
[Old-Trades-Lookup.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Old-Trades-Lookup.md
[Kline-Candlestick-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
[Continuous-Contract-Kline.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Continuous-Contract-Kline-Candlestick-Data.md
[Index-Price-Kline.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Price-Kline-Candlestick-Data.md
[Symbol-Order-Book-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
[Symbol-Price-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
[24hr-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
[Mark-Price.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
[Get-Funding-Rate-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
[Get-Funding-Rate-Info.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-Info.md
[Open-Interest.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
[Open-Interest-Statistics.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
[Long-Short-Ratio.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Long-Short-Ratio.md
[Taker-BuySell-Volume.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Taker-BuySell-Volume.md
[Basis.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
[Symbol-Price-Ticker-v2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker-v2.md
[Delivery-Price.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
[Composite-Index.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
[Index-Constituents.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
[Multi-Assets-Mode-Asset-Index.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
[Insurance-Fund-Balance.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
[ADL-Risk.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
[Order-Book-RPI.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book-RPI.md
[Trading-Schedule.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md


## Account types

Source: `include/binapi2/fapi/types/account.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `account_asset_t` | [Account-Information-V3.md] | complete | |
| `account_position_t` | [Account-Information-V3.md] | complete | |
| `account_information_t` | [Account-Information-V3.md] | complete | |
| `futures_account_balance_t` | [Futures-Account-Balance-V3.md] | complete | |
| `position_risk_request_t` | [Position-Information-V2.md] | complete | |
| `position_risk_t` | [Position-Information-V2.md] | complete | |
| `account_config_response_t` | [Account-Config.md] | complete | |
| `symbol_config_request_t` | [Symbol-Config.md] | complete | |
| `symbol_config_entry_t` | [Symbol-Config.md] | complete | |
| `multi_assets_mode_response_t` | [Get-Current-Multi-Assets-Mode.md] | complete | |
| `position_mode_response_t` | [Get-Current-Position-Mode.md] | complete | |
| `income_history_request_t` | [Get-Income-History.md] | complete | |
| `income_history_entry_t` | [Get-Income-History.md] | complete | |
| `leverage_bracket_request_t` | [Notional-and-Leverage-Brackets.md] | complete | |
| `leverage_bracket_entry_t` | [Notional-and-Leverage-Brackets.md] | complete | |
| `symbol_leverage_brackets_t` | [Notional-and-Leverage-Brackets.md] | complete | |
| `commission_rate_request_t` | [User-Commission-Rate.md] | complete | |
| `commission_rate_response_t` | [User-Commission-Rate.md] | complete | |
| `download_id_request_t` | [Get-Download-Id-Transaction.md] | complete | |
| `download_id_response_t` | [Get-Download-Id-Transaction.md] | complete | |
| `download_link_request_t` | [Get-Transaction-Download-Link.md] | complete | |
| `download_link_response_t` | [Get-Transaction-Download-Link.md] | complete | |
| `bnb_burn_status_response_t` | [Get-BNB-Burn-Status.md] | complete | |
| `toggle_bnb_burn_request_t` | [Toggle-BNB-Burn.md] | complete | |
| `trading_status_indicator_t` | [Quantitative-Rules.md] | complete | |
| `quantitative_rules_request_t` | [Quantitative-Rules.md] | complete | |
| `quantitative_rules_response_t` | [Quantitative-Rules.md] | complete | |
| `pm_account_info_request_t` | [portfolio-margin-endpoints.md] | complete | |
| `pm_account_info_response_t` | [portfolio-margin-endpoints.md] | complete | |

[Account-Information-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
[Futures-Account-Balance-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Account-Balance-V3.md
[Position-Information-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
[Account-Config.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Config.md
[Symbol-Config.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
[Get-Current-Multi-Assets-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Multi-Assets-Mode.md
[Get-Current-Position-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Position-Mode.md
[Get-Income-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
[Notional-and-Leverage-Brackets.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
[User-Commission-Rate.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
[Get-Download-Id-Transaction.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
[Get-Transaction-Download-Link.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
[Get-BNB-Burn-Status.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-BNB-Burn-Status.md
[Toggle-BNB-Burn.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Toggle-BNB-Burn-On-Futures-Trade.md
[Quantitative-Rules.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md
[portfolio-margin-endpoints.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md


## Trade types

Source: `include/binapi2/fapi/types/trade.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `new_order_request_t` | [New-Order-Test.md] | complete | |
| `order_response_t` | [New-Order-Test.md] | complete | |
| `modify_order_request_t` | [Modify-Order.md] | complete | |
| `cancel_order_request_t` | [Cancel-Order.md] | complete | |
| `query_order_request_t` | [Query-Order.md] | complete | |
| `test_new_order_request_t` | [New-Order-Test.md] | complete | Alias for `new_order_request_t` |
| `batch_orders_request_t` | [Place-Multiple-Orders.md] | complete | |
| `cancel_multiple_orders_request_t` | [Cancel-Multiple-Orders.md] | complete | |
| `cancel_all_open_orders_request_t` | [Cancel-All-Open-Orders.md] | complete | |
| `code_msg_response_t` | [error-code.md] | complete | |
| `auto_cancel_request_t` | [Auto-Cancel-All-Open-Orders.md] | complete | |
| `auto_cancel_response_t` | [Auto-Cancel-All-Open-Orders.md] | complete | |
| `query_open_order_request_t` | [Query-Current-Open-Order.md] | complete | |
| `all_open_orders_request_t` | [Current-All-Open-Orders.md] | complete | |
| `all_orders_request_t` | [All-Orders.md] | complete | |
| `position_risk_v3_t` | [Position-Information-V3.md] | complete | |
| `position_info_v3_request_t` | [Position-Information-V3.md] | complete | |
| `adl_quantile_values_t` | [Position-ADL-Quantile.md] | complete | |
| `adl_quantile_entry_t` | [Position-ADL-Quantile.md] | complete | |
| `adl_quantile_request_t` | [Position-ADL-Quantile.md] | complete | |
| `force_orders_request_t` | [Users-Force-Orders.md] | complete | |
| `account_trade_request_t` | [Account-Trade-List.md] | complete | |
| `account_trade_entry_t` | [Account-Trade-List.md] | complete | |
| `change_position_mode_request_t` | [Change-Position-Mode.md] | complete | |
| `change_multi_assets_mode_request_t` | [Change-Multi-Assets-Mode.md] | complete | |
| `change_leverage_request_t` | [Change-Initial-Leverage.md] | complete | |
| `change_leverage_response_t` | [Change-Initial-Leverage.md] | complete | |
| `change_margin_type_request_t` | [Change-Margin-Type.md] | complete | |
| `modify_isolated_margin_request_t` | [Modify-Isolated-Position-Margin.md] | complete | |
| `modify_isolated_margin_response_t` | [Modify-Isolated-Position-Margin.md] | complete | |
| `position_margin_history_request_t` | [Get-Position-Margin-Change-History.md] | complete | |
| `position_margin_history_entry_t` | [Get-Position-Margin-Change-History.md] | complete | |
| `order_modify_history_request_t` | [Get-Order-Modify-History.md] | complete | |
| `new_algo_order_request_t` | [New-Algo-Order.md] | complete | |
| `algo_order_response_t` | [New-Algo-Order.md] | complete | |
| `cancel_algo_order_request_t` | [Cancel-Algo-Order.md] | complete | |
| `query_algo_order_request_t` | [Query-Algo-Order.md] | complete | |
| `all_algo_orders_request_t` | [Query-All-Algo-Orders.md] | complete | |
| `tradfi_perps_request_t` | [TradFi-Perps.md] | complete | |

[New-Order-Test.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
[Modify-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Order.md
[Cancel-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Order.md
[Query-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Order.md
[Place-Multiple-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Place-Multiple-Orders.md
[Cancel-Multiple-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Multiple-Orders.md
[Cancel-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Open-Orders.md
[Auto-Cancel-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
[Query-Current-Open-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Current-Open-Order.md
[Current-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Open-Orders.md
[All-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/All-Orders.md
[Position-Information-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
[Position-ADL-Quantile.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
[Users-Force-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Users-Force-Orders.md
[Account-Trade-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
[Change-Position-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Position-Mode.md
[Change-Multi-Assets-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Multi-Assets-Mode.md
[Change-Initial-Leverage.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
[Change-Margin-Type.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Margin-Type.md
[Modify-Isolated-Position-Margin.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
[Get-Position-Margin-Change-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
[Get-Order-Modify-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Order-Modify-History.md
[New-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
[Cancel-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Algo-Order.md
[Query-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Algo-Order.md
[Query-All-Algo-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-All-Algo-Orders.md
[TradFi-Perps.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/TradFi-Perps.md


## WebSocket market stream types

Source: `include/binapi2/fapi/types/market_stream_events.hpp`
(also available via convenience header `include/binapi2/fapi/types/streams.hpp` which includes both market and user stream events)

| Type | Doc | Status | Notes |
|---|---|---|---|
| `book_ticker_stream_event_t` | [Individual-Symbol-Book-Ticker-Streams.md] | complete | |
| `aggregate_trade_stream_event_t` | [Aggregate-Trade-Streams.md] | complete | |
| `mark_price_stream_event_t` | [Mark-Price-Stream.md] | complete | |
| `all_market_mark_price_stream_event_t` | [Mark-Price-Stream-All.md] | complete | |
| `depth_stream_event_t` | [Diff-Book-Depth-Streams.md] | complete | |
| `mini_ticker_stream_event_t` | [All-Market-Mini-Tickers-Stream.md] | complete | |
| `all_market_mini_ticker_stream_event_t` | [All-Market-Mini-Tickers-Stream.md] | complete | |
| `ticker_stream_event_t` | [All-Market-Tickers-Streams.md] | complete | |
| `all_market_ticker_stream_event_t` | [All-Market-Tickers-Streams.md] | complete | |
| `liquidation_order_stream_data_t` | [All-Market-Liquidation-Order-Streams.md] | complete | |
| `liquidation_order_stream_event_t` | [All-Market-Liquidation-Order-Streams.md] | complete | |
| `kline_stream_data_t` | [Kline-Candlestick-Streams.md] | complete | |
| `kline_stream_event_t` | [Kline-Candlestick-Streams.md] | complete | |
| `continuous_contract_kline_stream_data_t` | [Continuous-Contract-Kline-Streams.md] | complete | |
| `continuous_contract_kline_stream_event_t` | [Continuous-Contract-Kline-Streams.md] | complete | |
| `composite_index_constituent_t` | [Composite-Index-Streams.md] | complete | |
| `composite_index_stream_event_t` | [Composite-Index-Streams.md] | complete | |
| `contract_info_bracket_t` | [Contract-Info-Stream.md] | complete | |
| `contract_info_stream_event_t` | [Contract-Info-Stream.md] | complete | |
| `asset_index_stream_event_t` | [Asset-Index-Stream.md] | complete | |
| `all_asset_index_stream_event_t` | [Asset-Index-Stream.md] | complete | |
| `trading_session_stream_event_t` | [Trading-Session-Stream.md] | complete | |

[Individual-Symbol-Book-Ticker-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Book-Ticker-Streams.md
[Aggregate-Trade-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Aggregate-Trade-Streams.md
[Mark-Price-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream.md
[Mark-Price-Stream-All.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream-for-All-market.md
[Diff-Book-Depth-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams.md
[All-Market-Mini-Tickers-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
[All-Market-Tickers-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
[All-Market-Liquidation-Order-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
[Kline-Candlestick-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Kline-Candlestick-Streams.md
[Continuous-Contract-Kline-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
[Composite-Index-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
[Contract-Info-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
[Asset-Index-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Asset-Index-Stream.md
[Trading-Session-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Trading-Session-Stream.md


## User data stream types

Source: `include/binapi2/fapi/types/user_stream_events.hpp`
(also available via convenience header `include/binapi2/fapi/types/streams.hpp`)

Note: `user_stream_event_t` is a `std::variant` of 10 event types defined at the bottom of `user_stream_events.hpp`. This variant is used by `streams::user_streams::subscribe()` which returns a `cobalt::generator<result<user_stream_event_t>>`.

| Type | Doc | Status | Notes |
|---|---|---|---|
| `account_update_balance_t` | [Event-Balance-and-Position-Update.md] | complete | |
| `account_update_position_t` | [Event-Balance-and-Position-Update.md] | complete | |
| `account_update_data_t` | [Event-Balance-and-Position-Update.md] | complete | |
| `account_update_event_t` | [Event-Balance-and-Position-Update.md] | complete | |
| `order_trade_update_order_t` | [Event-Order-Update.md] | complete | |
| `order_trade_update_event_t` | [Event-Order-Update.md] | complete | |
| `margin_call_position_t` | [Event-Margin-Call.md] | complete | |
| `margin_call_event_t` | [Event-Margin-Call.md] | complete | |
| `listen_key_expired_event_t` | [Event-User-Data-Stream-Expired.md] | complete | Extra: has `T` field not in doc |
| `account_config_leverage_t` | [Event-Account-Config-Update.md] | complete | |
| `account_config_multi_assets_t` | [Event-Account-Config-Update.md] | complete | |
| `account_config_update_event_t` | [Event-Account-Config-Update.md] | complete | |
| `trade_lite_event_t` | [Event-Trade-Lite.md] | complete | |
| `algo_order_update_data_t` | [Event-Algo-Order-Update.md] | complete | |
| `algo_order_update_event_t` | [Event-Algo-Order-Update.md] | complete | |
| `conditional_order_reject_data_t` | [Event-Conditional-Order-Trigger-Reject.md] | complete | |
| `conditional_order_trigger_reject_event_t` | [Event-Conditional-Order-Trigger-Reject.md] | complete | |
| `grid_update_data_t` | [Event-GRID-UPDATE.md] | complete | |
| `grid_update_event_t` | [Event-GRID-UPDATE.md] | complete | |
| `strategy_update_data_t` | [Event-STRATEGY-UPDATE.md] | complete | |
| `strategy_update_event_t` | [Event-STRATEGY-UPDATE.md] | complete | |

[Event-Balance-and-Position-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
[Event-Order-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
[Event-Margin-Call.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
[Event-User-Data-Stream-Expired.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-User-Data-Stream-Expired.md
[Event-Account-Config-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
[Event-Trade-Lite.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md
[Event-Algo-Order-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
[Event-Conditional-Order-Trigger-Reject.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
[Event-GRID-UPDATE.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
[Event-STRATEGY-UPDATE.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md


## Convert types

Source: `include/binapi2/fapi/types/convert.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `convert_quote_request_t` | [Send-quote-request.md] | complete | |
| `convert_quote_response_t` | [Send-quote-request.md] | complete | |
| `convert_accept_request_t` | [Accept-Quote.md] | complete | |
| `convert_accept_response_t` | [Accept-Quote.md] | complete | |
| `convert_order_status_request_t` | [Order-Status.md] | complete | |
| `convert_order_status_response_t` | [Order-Status.md] | complete | |

[Send-quote-request.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Send-quote-request.md
[Accept-Quote.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Accept-Quote.md
[Order-Status.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Order-Status.md


## WebSocket API types

Source: `include/binapi2/fapi/types/websocket_api.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `websocket_api_error_t` | [websocket-api-general-info.md] | complete | |
| `session_logon_request_t` | [websocket-api-general-info.md] | complete | |
| `websocket_api_status_t` | [websocket-api-general-info.md] | complete | |
| `session_logon_result_t` | [websocket-api-general-info.md] | complete | |
| `websocket_api_signed_request_t` | [websocket-api-general-info.md] | complete | |
| `websocket_api_order_place_request_t` | [WS-New-Order.md] | complete | |
| `websocket_api_order_query_request_t` | [WS-Query-Order.md] | complete | |
| `websocket_api_order_cancel_request_t` | [WS-Cancel-Order.md] | complete | |
| `websocket_api_book_ticker_request_t` | [WS-Symbol-Order-Book-Ticker.md] | complete | |
| `websocket_api_price_ticker_request_t` | [WS-Symbol-Price-Ticker.md] | complete | |
| `websocket_api_order_modify_request_t` | [WS-Modify-Order.md] | complete | |
| `websocket_api_position_request_t` | [WS-Position-Information.md] | complete | |
| `websocket_api_algo_order_place_request_t` | [WS-New-Algo-Order.md] | complete | |
| `websocket_api_algo_order_cancel_request_t` | [WS-Cancel-Algo-Order.md] | complete | |
| `websocket_api_user_data_stream_request_t` | [Start-User-Data-Stream-Wsp.md] | complete | |
| `websocket_api_listen_key_result_t` | [Start-User-Data-Stream-Wsp.md] | complete | |
| `websocket_api_response<T>_t` | [websocket-api-general-info.md] | complete | |

[websocket-api-general-info.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
[WS-New-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Order.md
[WS-Query-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Query-Order.md
[WS-Cancel-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Order.md
[WS-Symbol-Order-Book-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Order-Book-Ticker.md
[WS-Symbol-Price-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Price-Ticker.md
[WS-Modify-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Modify-Order.md
[WS-Position-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Information.md
[WS-New-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Algo-Order.md
[WS-Cancel-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Algo-Order.md
[Start-User-Data-Stream-Wsp.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream-Wsp.md


## Summary

All data types are complete. No types have missing fields relative to their API documentation.

Key structural notes:
- Stream event types are split into `market_stream_events.hpp` and `user_stream_events.hpp`. The convenience header `streams.hpp` includes both.
- Subscription types live in `types/subscriptions.hpp` (not `streams/`).
- `request_tags.hpp` provides empty tag types for per-service concept constraints.
- `pair_t` and `symbol_t` are strong types for trading pairs and symbols.
- `decimal_t` has a `fmt::formatter` specialization for direct use with spdlog/fmt.
