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
| `security_type` | (internal) | complete | Not from API docs |
| `order_side` | [common-definition.md] | complete | |
| `order_type` | [common-definition.md] | complete | |
| `time_in_force` | [common-definition.md] | partial | Missing `rpi` (RPI - Retail Price Improvement) |
| `kline_interval` | [common-definition.md] | complete | |
| `position_side` | [common-definition.md] | complete | |
| `working_type` | [common-definition.md] | complete | |
| `response_type` | [common-definition.md] | complete | |
| `margin_type` | [common-definition.md] | complete | |
| `contract_type` | [common-definition.md] | complete | Extra: `tradifi_perpetual` (undocumented) |
| `contract_status` | [common-definition.md] | complete | |
| `order_status` | [common-definition.md] | complete | |
| `stp_mode` | [common-definition.md] | complete | |
| `price_match` | [common-definition.md] | complete | |
| `income_type` | [common-definition.md] | complete | |
| `futures_data_period` | [common-definition.md] | complete | |

[common-definition.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md


## Common types

Source: `include/binapi2/fapi/types/common.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `empty_response` | (internal) | complete | |
| `rate_limit` | [common-definition.md] | complete | |
| `server_time_response` | [Check-Server-Time.md] | complete | |
| `binance_error_document` | [error-code.md] | complete | |
| `price_level` | [Order-Book.md] | complete | |
| `exchange_info_asset` | [Exchange-Information.md] | complete | |
| `symbol_filter` | [Exchange-Information.md] | complete | |
| `symbol_info` | [Exchange-Information.md] | complete | |
| `exchange_info_response` | [Exchange-Information.md] | partial | Missing `exchangeFilters` (always empty array) |
| `listen_key_response` | [Start-User-Data-Stream.md] | complete | |

[Check-Server-Time.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
[error-code.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
[Order-Book.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
[Exchange-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
[Start-User-Data-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream.md


## Market data types

Source: `include/binapi2/fapi/types/market_data.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `ping_request` | [Check-Server-Time.md] | complete | |
| `server_time_request` | [Check-Server-Time.md] | complete | |
| `exchange_info_request` | [Exchange-Information.md] | complete | |
| `order_book_request` | [Order-Book.md] | complete | |
| `order_book_response` | [Order-Book.md] | complete | |
| `recent_trades_request` | [Recent-Trades-List.md] | complete | |
| `recent_trade` | [Recent-Trades-List.md] | complete | |
| `aggregate_trades_request` | [Compressed-Aggregate-Trades-List.md] | complete | |
| `aggregate_trade` | [Compressed-Aggregate-Trades-List.md] | complete | |
| `historical_trades_request` | [Old-Trades-Lookup.md] | complete | |
| `kline_request` | [Kline-Candlestick-Data.md] | complete | |
| `continuous_kline_request` | [Continuous-Contract-Kline.md] | complete | |
| `index_price_kline_request` | [Index-Price-Kline.md] | complete | |
| `kline` | [Kline-Candlestick-Data.md] | complete | |
| `book_ticker_request` | [Symbol-Order-Book-Ticker.md] | complete | |
| `book_ticker` | [Symbol-Order-Book-Ticker.md] | complete | Extra: `lastUpdateId` not in doc |
| `price_ticker_request` | [Symbol-Price-Ticker.md] | complete | |
| `price_ticker` | [Symbol-Price-Ticker.md] | complete | |
| `ticker_24hr_request` | [24hr-Ticker.md] | complete | |
| `ticker_24hr` | [24hr-Ticker.md] | complete | |
| `mark_price_request` | [Mark-Price.md] | complete | |
| `mark_price` | [Mark-Price.md] | partial | Missing `interestRate` |
| `funding_rate_history_request` | [Get-Funding-Rate-History.md] | complete | |
| `funding_rate_history_entry` | [Get-Funding-Rate-History.md] | complete | |
| `funding_rate_info` | [Get-Funding-Rate-Info.md] | complete | |
| `open_interest_request` | [Open-Interest.md] | complete | |
| `open_interest` | [Open-Interest.md] | complete | |
| `futures_data_request` | [Open-Interest-Statistics.md] | complete | |
| `open_interest_statistics_entry` | [Open-Interest-Statistics.md] | complete | |
| `long_short_ratio_entry` | [Long-Short-Ratio.md] | complete | Also used for Top-Long-Short-Account-Ratio, Top-Trader-Long-Short-Ratio |
| `taker_buy_sell_volume_entry` | [Taker-BuySell-Volume.md] | complete | |
| `basis_request` | [Basis.md] | complete | |
| `basis_entry` | [Basis.md] | complete | |
| `price_ticker_v2_request` | [Symbol-Price-Ticker-v2.md] | complete | |
| `delivery_price_request` | [Delivery-Price.md] | complete | |
| `delivery_price_entry` | [Delivery-Price.md] | complete | |
| `composite_index_info_request` | [Composite-Index.md] | complete | |
| `composite_index_base_asset` | [Composite-Index.md] | complete | |
| `composite_index_info` | [Composite-Index.md] | complete | |
| `index_constituents_request` | [Index-Constituents.md] | complete | |
| `index_constituent` | [Index-Constituents.md] | complete | |
| `index_constituents_response` | [Index-Constituents.md] | complete | |
| `asset_index_request` | [Multi-Assets-Mode-Asset-Index.md] | complete | |
| `asset_index` | [Multi-Assets-Mode-Asset-Index.md] | complete | |
| `insurance_fund_request` | [Insurance-Fund-Balance.md] | complete | |
| `insurance_fund_asset` | [Insurance-Fund-Balance.md] | complete | |
| `insurance_fund_response` | [Insurance-Fund-Balance.md] | complete | |
| `adl_risk_request` | [ADL-Risk.md] | complete | |
| `adl_risk_entry` | [ADL-Risk.md] | complete | |
| `rpi_depth_request` | [Order-Book-RPI.md] | complete | |
| `trading_schedule_request` | [Trading-Schedule.md] | complete | |
| `trading_schedule_response` | [Trading-Schedule.md] | partial | `marketSchedules` type is wrong: should be nested struct with `sessions` array (each having `startTime`, `endTime`, `type`), not `map<string, vector<string>>` |

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
| `account_asset` | [Account-Information-V3.md] | complete | |
| `account_position` | [Account-Information-V3.md] | complete | |
| `account_information` | [Account-Information-V3.md] | complete | |
| `futures_account_balance` | [Futures-Account-Balance-V3.md] | complete | |
| `position_risk_request` | [Position-Information-V2.md] | complete | |
| `position_risk` | [Position-Information-V2.md] | complete | |
| `account_config_response` | [Account-Config.md] | partial | Missing `updateTime` |
| `symbol_config_request` | [Symbol-Config.md] | complete | |
| `symbol_config_entry` | [Symbol-Config.md] | partial | `isAutoAddMargin` is `std::string` but doc shows boolean |
| `multi_assets_mode_response` | [Get-Current-Multi-Assets-Mode.md] | complete | |
| `position_mode_response` | [Get-Current-Position-Mode.md] | complete | |
| `income_history_request` | [Get-Income-History.md] | complete | |
| `income_history_entry` | [Get-Income-History.md] | complete | |
| `leverage_bracket_request` | [Notional-and-Leverage-Brackets.md] | complete | |
| `leverage_bracket_entry` | [Notional-and-Leverage-Brackets.md] | complete | |
| `symbol_leverage_brackets` | [Notional-and-Leverage-Brackets.md] | complete | |
| `commission_rate_request` | [User-Commission-Rate.md] | complete | |
| `commission_rate_response` | [User-Commission-Rate.md] | complete | |
| `download_id_request` | [Get-Download-Id-Transaction.md] | complete | |
| `download_id_response` | [Get-Download-Id-Transaction.md] | complete | |
| `download_link_request` | [Get-Transaction-Download-Link.md] | complete | |
| `download_link_response` | [Get-Transaction-Download-Link.md] | complete | |
| `bnb_burn_status_response` | [Get-BNB-Burn-Status.md] | complete | |
| `toggle_bnb_burn_request` | [Toggle-BNB-Burn.md] | complete | |
| `trading_status_indicator` | [Quantitative-Rules.md] | complete | |
| `quantitative_rules_request` | [Quantitative-Rules.md] | complete | |
| `quantitative_rules_response` | [Quantitative-Rules.md] | partial | Missing `updateTime` |
| `pm_account_info_request` | [portfolio-margin-endpoints.md] | complete | |
| `pm_account_info_response` | [portfolio-margin-endpoints.md] | complete | |

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
| `new_order_request` | [New-Order-Test.md] | partial | Missing 11 optional params: `positionSide`, `reduceOnly`, `closePosition`, `activationPrice`, `callbackRate`, `workingType`, `priceProtect`, `newOrderRespType`, `priceMatch`, `selfTradePreventionMode`, `goodTillDate` |
| `order_response` | [New-Order-Test.md] | partial | Missing `activatePrice`, `priceRate` |
| `modify_order_request` | [Modify-Order.md] | complete | |
| `cancel_order_request` | [Cancel-Order.md] | complete | |
| `query_order_request` | [Query-Order.md] | complete | |
| `test_new_order_request` | [New-Order-Test.md] | partial | Alias for `new_order_request` (inherits issues) |
| `batch_orders_request` | [Place-Multiple-Orders.md] | complete | |
| `cancel_multiple_orders_request` | [Cancel-Multiple-Orders.md] | complete | |
| `cancel_all_open_orders_request` | [Cancel-All-Open-Orders.md] | complete | |
| `code_msg_response` | [error-code.md] | complete | |
| `auto_cancel_request` | [Auto-Cancel-All-Open-Orders.md] | complete | |
| `auto_cancel_response` | [Auto-Cancel-All-Open-Orders.md] | complete | |
| `query_open_order_request` | [Query-Current-Open-Order.md] | complete | |
| `all_open_orders_request` | [Current-All-Open-Orders.md] | complete | |
| `all_orders_request` | [All-Orders.md] | complete | |
| `position_risk_v3` | [Position-Information-V3.md] | complete | |
| `position_info_v3_request` | [Position-Information-V3.md] | complete | |
| `adl_quantile_values` | [Position-ADL-Quantile.md] | partial | Missing `HEDGE` field (used in crossed-margin hedge mode) |
| `adl_quantile_entry` | [Position-ADL-Quantile.md] | complete | |
| `adl_quantile_request` | [Position-ADL-Quantile.md] | complete | |
| `force_orders_request` | [Users-Force-Orders.md] | complete | |
| `account_trade_request` | [Account-Trade-List.md] | complete | |
| `account_trade_entry` | [Account-Trade-List.md] | complete | |
| `change_position_mode_request` | [Change-Position-Mode.md] | complete | |
| `change_multi_assets_mode_request` | [Change-Multi-Assets-Mode.md] | complete | |
| `change_leverage_request` | [Change-Initial-Leverage.md] | complete | |
| `change_leverage_response` | [Change-Initial-Leverage.md] | complete | |
| `change_margin_type_request` | [Change-Margin-Type.md] | complete | |
| `modify_isolated_margin_request` | [Modify-Isolated-Position-Margin.md] | complete | |
| `modify_isolated_margin_response` | [Modify-Isolated-Position-Margin.md] | complete | |
| `position_margin_history_request` | [Get-Position-Margin-Change-History.md] | complete | |
| `position_margin_history_entry` | [Get-Position-Margin-Change-History.md] | complete | |
| `order_modify_history_request` | [Get-Order-Modify-History.md] | complete | |
| `new_algo_order_request` | [New-Algo-Order.md] | partial | Missing 9 optional params: `priceMatch`, `closePosition`, `priceProtect`, `reduceOnly`, `activatePrice`, `callbackRate`, `newOrderRespType`, `selfTradePreventionMode`, `goodTillDate` |
| `algo_order_response` | [New-Algo-Order.md] | partial | Missing 18 fields: `orderType`, `timeInForce`, `quantity`, `triggerPrice`, `price`, `icebergQuantity`, `selfTradePreventionMode`, `workingType`, `priceMatch`, `closePosition`, `priceProtect`, `reduceOnly`, `activatePrice`, `callbackRate`, `createTime`, `updateTime`, `triggerTime`, `goodTillDate` |
| `cancel_algo_order_request` | [Cancel-Algo-Order.md] | complete | |
| `query_algo_order_request` | [Query-Algo-Order.md] | complete | |
| `all_algo_orders_request` | [Query-All-Algo-Orders.md] | complete | |
| `tradfi_perps_request` | [TradFi-Perps.md] | complete | |

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

Source: `include/binapi2/fapi/types/streams.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `book_ticker_stream_event` | [Individual-Symbol-Book-Ticker-Streams.md] | complete | |
| `aggregate_trade_stream_event` | [Aggregate-Trade-Streams.md] | partial | Missing `nq` (normal quantity without RPI) |
| `mark_price_stream_event` | [Mark-Price-Stream.md] | complete | |
| `all_market_mark_price_stream_event` | [Mark-Price-Stream-All.md] | complete | |
| `depth_stream_event` | [Diff-Book-Depth-Streams.md] | complete | |
| `mini_ticker_stream_event` | [All-Market-Mini-Tickers-Stream.md] | complete | |
| `all_market_mini_ticker_stream_event` | [All-Market-Mini-Tickers-Stream.md] | complete | |
| `ticker_stream_event` | [All-Market-Tickers-Streams.md] | complete | |
| `all_market_ticker_stream_event` | [All-Market-Tickers-Streams.md] | complete | |
| `liquidation_order_stream_data` | [All-Market-Liquidation-Order-Streams.md] | complete | |
| `liquidation_order_stream_event` | [All-Market-Liquidation-Order-Streams.md] | complete | |
| `kline_stream_data` | [Kline-Candlestick-Streams.md] | complete | |
| `kline_stream_event` | [Kline-Candlestick-Streams.md] | complete | |
| `continuous_contract_kline_stream_data` | [Continuous-Contract-Kline-Streams.md] | complete | |
| `continuous_contract_kline_stream_event` | [Continuous-Contract-Kline-Streams.md] | complete | |
| `composite_index_constituent` | [Composite-Index-Streams.md] | complete | |
| `composite_index_stream_event` | [Composite-Index-Streams.md] | complete | |
| `contract_info_bracket` | [Contract-Info-Stream.md] | complete | |
| `contract_info_stream_event` | [Contract-Info-Stream.md] | complete | |
| `asset_index_stream_event` | [Asset-Index-Stream.md] | complete | |
| `all_asset_index_stream_event` | [Asset-Index-Stream.md] | complete | |
| `trading_session_stream_event` | [Trading-Session-Stream.md] | complete | |

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

Source: `include/binapi2/fapi/types/streams.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `account_update_balance` | [Event-Balance-and-Position-Update.md] | complete | |
| `account_update_position` | [Event-Balance-and-Position-Update.md] | partial | Missing `bep` (breakeven price) |
| `account_update_data` | [Event-Balance-and-Position-Update.md] | complete | |
| `account_update_event` | [Event-Balance-and-Position-Update.md] | complete | |
| `order_trade_update_order` | [Event-Order-Update.md] | partial | Only 11 of ~37 fields implemented. Missing 26 fields: `sp`, `l`, `z`, `L`, `N`, `n`, `T`, `t`, `b`, `a`, `m`, `R`, `wt`, `ot`, `ps`, `cp`, `AP`, `cr`, `pP`, `si`, `ss`, `rp`, `V`, `pm`, `gtd`, `er` |
| `order_trade_update_event` | [Event-Order-Update.md] | complete | |
| `margin_call_position` | [Event-Margin-Call.md] | complete | |
| `margin_call_event` | [Event-Margin-Call.md] | complete | |
| `listen_key_expired_event` | [Event-User-Data-Stream-Expired.md] | complete | Extra: has `T` field not in doc |
| `account_config_leverage` | [Event-Account-Config-Update.md] | complete | |
| `account_config_multi_assets` | [Event-Account-Config-Update.md] | complete | |
| `account_config_update_event` | [Event-Account-Config-Update.md] | complete | |
| `trade_lite_event` | [Event-Trade-Lite.md] | complete | |
| `algo_order_update_data` | [Event-Algo-Order-Update.md] | complete | |
| `algo_order_update_event` | [Event-Algo-Order-Update.md] | complete | |
| `conditional_order_reject_data` | [Event-Conditional-Order-Trigger-Reject.md] | complete | |
| `conditional_order_trigger_reject_event` | [Event-Conditional-Order-Trigger-Reject.md] | complete | |
| `grid_update_data` | [Event-GRID-UPDATE.md] | complete | |
| `grid_update_event` | [Event-GRID-UPDATE.md] | complete | |
| `strategy_update_data` | [Event-STRATEGY-UPDATE.md] | complete | |
| `strategy_update_event` | [Event-STRATEGY-UPDATE.md] | complete | |

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
| `convert_quote_request` | [Send-quote-request.md] | complete | |
| `convert_quote_response` | [Send-quote-request.md] | complete | |
| `convert_accept_request` | [Accept-Quote.md] | complete | |
| `convert_accept_response` | [Accept-Quote.md] | complete | |
| `convert_order_status_request` | [Order-Status.md] | complete | |
| `convert_order_status_response` | [Order-Status.md] | complete | |

[Send-quote-request.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Send-quote-request.md
[Accept-Quote.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Accept-Quote.md
[Order-Status.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Order-Status.md


## WebSocket API types

Source: `include/binapi2/fapi/types/websocket_api.hpp`

| Type | Doc | Status | Notes |
|---|---|---|---|
| `websocket_api_error` | [websocket-api-general-info.md] | complete | |
| `session_logon_request` | [websocket-api-general-info.md] | complete | |
| `websocket_api_status` | [websocket-api-general-info.md] | complete | |
| `session_logon_result` | [websocket-api-general-info.md] | partial | Has `authorizedSinceConnect` (nonexistent in doc); should be `authorizedSince` + `connectedSince` (two separate fields) |
| `websocket_api_signed_request` | [websocket-api-general-info.md] | complete | |
| `websocket_api_order_place_request` | [WS-New-Order.md] | complete | |
| `websocket_api_order_query_request` | [WS-Query-Order.md] | complete | |
| `websocket_api_order_cancel_request` | [WS-Cancel-Order.md] | complete | |
| `websocket_api_book_ticker_request` | [WS-Symbol-Order-Book-Ticker.md] | complete | |
| `websocket_api_price_ticker_request` | [WS-Symbol-Price-Ticker.md] | complete | |
| `websocket_api_order_modify_request` | [WS-Modify-Order.md] | complete | |
| `websocket_api_position_request` | [WS-Position-Information.md] | complete | |
| `websocket_api_algo_order_place_request` | [WS-New-Algo-Order.md] | partial | Missing 10 optional params: `priceMatch`, `closePosition`, `priceProtect`, `reduceOnly`, `activatePrice`, `callbackRate`, `clientAlgoId`, `newOrderRespType`, `selfTradePreventionMode`, `goodTillDate`; also `type` should be mandatory |
| `websocket_api_algo_order_cancel_request` | [WS-Cancel-Algo-Order.md] | complete | |
| `websocket_api_user_data_stream_request` | [Start-User-Data-Stream-Wsp.md] | complete | |
| `websocket_api_listen_key_result` | [Start-User-Data-Stream-Wsp.md] | complete | |
| `websocket_api_response<T>` | [websocket-api-general-info.md] | complete | |

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


## Summary of incomplete types

Types requiring attention, ordered by severity:

| Type | File | Missing |
|---|---|---|
| `order_trade_update_order` | streams.hpp | 26 of ~37 fields missing |
| `algo_order_response` | trade.hpp | 18 response fields missing |
| `new_order_request` | trade.hpp | 11 optional request params missing |
| `websocket_api_algo_order_place_request` | websocket_api.hpp | 10 optional params missing |
| `new_algo_order_request` | trade.hpp | 9 optional request params missing |
| `order_response` | trade.hpp | 2 response fields missing (`activatePrice`, `priceRate`) |
| `session_logon_result` | websocket_api.hpp | Wrong field name: `authorizedSinceConnect` should be `authorizedSince` + `connectedSince` |
| `trading_schedule_response` | market_data.hpp | `marketSchedules` has wrong nested type structure |
| `mark_price` | market_data.hpp | Missing `interestRate` |
| `account_update_position` | streams.hpp | Missing `bep` (breakeven price) |
| `aggregate_trade_stream_event` | streams.hpp | Missing `nq` (normal quantity without RPI) |
| `account_config_response` | account.hpp | Missing `updateTime` |
| `quantitative_rules_response` | account.hpp | Missing `updateTime` |
| `symbol_config_entry` | account.hpp | `isAutoAddMargin` type mismatch (string vs bool) |
| `adl_quantile_values` | trade.hpp | Missing `HEDGE` field |
| `time_in_force` | enums.hpp | Missing `rpi` enum value |
| `exchange_info_response` | common.hpp | Missing `exchangeFilters` (always empty) |
