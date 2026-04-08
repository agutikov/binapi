# FAPI implementation status

Status meanings used here:
- implemented = direct current endpoint, method, or stream support exists in the FAPI codebase
- partial = only a subset, adjacent variant, or general/reference coverage exists
- superseded = a newer API version is implemented instead (e.g. V3 replaces V2)
- informational = doc is a reference or notice, not an endpoint requiring implementation
- stub = doc is a placeholder with no endpoint specification
- TBD = no direct support exists in current FAPI code


## Top-level docs

| Doc | Status | Source files |
|---|---|---|
| [general-info.md] | partial | [client.cpp], [signing.cpp], [signing.hpp] |
| [common-definition.md] | implemented | [types/enums.hpp], [types/common.hpp] |
| [error-code.md] | partial | [error.hpp], [result.hpp], [client.hpp] |
| [websocket-api-general-info.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [ws-api/generated_methods.hpp] |
| [websocket-market-streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp] |
| [user-data-streams.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [rest/user_data_streams.hpp] |
| [convert.md] | implemented | [rest/convert.hpp], [rest/convert.cpp], [types/convert.hpp] |
| [portfolio-margin-endpoints.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |

## Market data REST docs

| Doc | Status | Source files |
|---|---|---|
| [Check-Server-Time.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Exchange-Information.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Order-Book.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Recent-Trades-List.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Compressed-Aggregate-Trades-List.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Kline-Candlestick-Data.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Continuous-Contract-Kline-Candlestick-Data.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Index-Price-Kline-Candlestick-Data.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Mark-Price-Kline-Candlestick-Data.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Premium-Index-Kline-Data.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Mark-Price.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Get-Funding-Rate-History.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Get-Funding-Rate-Info.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Open-Interest.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Open-Interest-Statistics.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Top-Long-Short-Account-Ratio.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Top-Trader-Long-Short-Ratio.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Long-Short-Ratio.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Taker-BuySell-Volume.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |
| [Basis.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [24hr-Ticker-Price-Change-Statistics.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Symbol-Price-Ticker.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Symbol-Price-Ticker-v2.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [rest/generated_endpoints.hpp] |
| [Symbol-Order-Book-Ticker.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp] |
| [Delivery-Price.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Composite-Index-Symbol-Information.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Index-Constituents.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Multi-Assets-Mode-Asset-Index.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Insurance-Fund-Balance.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [ADL-Risk.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Order-Book-RPI.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [rest/generated_endpoints.hpp] |
| [Trading-Schedule.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp], [rest/generated_endpoints.hpp] |
| [Delist-Schedule.md] | informational | Not a separate endpoint; doc directs users to check `deliveryDate` in `exchange_info()` response. |
| [Old-Trades-Lookup.md] | implemented | [rest/market_data.hpp], [rest/market_data.cpp], [types/market_data.hpp] |

## Market data WebSocket API docs

| Doc | Status | Source files |
|---|---|---|
| [market-data/websocket-api] | partial | Order-book RPC overview; no dedicated `depth` WS API method. |
| [WS-Symbol-Order-Book-Ticker.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [WS-Symbol-Price-Ticker.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |

## Account REST docs

| Doc | Status | Source files |
|---|---|---|
| [account/rest-api] | stub | Transfer overview page; docs contain only a placeholder with no endpoint specification. |
| [Account-Information-V2.md] | superseded | Legacy V2 endpoint superseded by V3; implemented as `account_service::account_information()` using `/fapi/v3/account`. |
| [Account-Information-V3.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Futures-Account-Balance-V2.md] | superseded | Legacy V2 endpoint superseded by V3; implemented as `account_service::balances()` using `/fapi/v3/balance`. |
| [Futures-Account-Balance-V3.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Account-Config.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Symbol-Config.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Current-Multi-Assets-Mode.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Current-Position-Mode.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Income-History.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Notional-and-Leverage-Brackets.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [User-Commission-Rate.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Query-Rate-Limit.md] | implemented | [rest/account.hpp], [rest/account.cpp] |
| [Get-Future-Account-Transaction-History-List.md] | stub | Docs contain only a placeholder with no endpoint specification. |
| [Get-Download-Id-For-Futures-Transaction-History.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Futures-Transaction-History-Download-Link-by-Id.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Download-Id-For-Futures-Order-History.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Futures-Order-History-Download-Link-by-Id.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Download-Id-For-Futures-Trade-History.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-Futures-Trade-Download-Link-by-Id.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Get-BNB-Burn-Status.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Toggle-BNB-Burn-On-Futures-Trade.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Futures-Trading-Quantitative-Rules-Indicators.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |

## Account WebSocket API docs

| Doc | Status | Source files |
|---|---|---|
| [WS-Account-Information.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [WS-Account-Information-V2.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [WS-Futures-Account-Balance.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |

## Trade REST docs

| Doc | Status | Source files |
|---|---|---|
| [New-Order-Test.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Place-Multiple-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Modify-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Modify-Multiple-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Query-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Cancel-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Cancel-Multiple-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Cancel-All-Open-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Auto-Cancel-All-Open-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Query-Current-Open-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [All-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Current-All-Open-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Position-Information-V2.md] | implemented | [rest/account.hpp], [rest/account.cpp], [types/account.hpp] |
| [Position-Information-V3.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Position-ADL-Quantile-Estimation.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Users-Force-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Account-Trade-List.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Change-Position-Mode.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Change-Multi-Assets-Mode.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Change-Initial-Leverage.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Change-Margin-Type.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Modify-Isolated-Position-Margin.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Get-Position-Margin-Change-History.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Get-Order-Modify-History.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Current-All-Algo-Open-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Cancel-All-Algo-Open-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [New-Algo-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Cancel-Algo-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Query-Algo-Order.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [Query-All-Algo-Orders.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |
| [TradFi-Perps.md] | implemented | [rest/trade.hpp], [rest/trade.cpp], [types/trade.hpp] |

## Trade WebSocket API docs

| Doc | Status | Source files |
|---|---|---|
| [WS-New-Algo-Order.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [types/websocket_api.hpp] |
| [WS-Cancel-Algo-Order.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [types/websocket_api.hpp] |
| [WS-Modify-Order.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [types/websocket_api.hpp] |
| [WS-Query-Order.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [WS-Cancel-Order.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [WS-Position-Info-V2.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [types/websocket_api.hpp] |
| [WS-Position-Information.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp], [types/websocket_api.hpp] |

## User-data stream docs

| Doc | Status | Source files |
|---|---|---|
| [Start-User-Data-Stream.md] | implemented | [rest/user_data_streams.hpp], [rest/user_data_streams.cpp] |
| [Keepalive-User-Data-Stream.md] | implemented | [rest/user_data_streams.hpp], [rest/user_data_streams.cpp] |
| [Close-User-Data-Stream.md] | implemented | [rest/user_data_streams.hpp], [rest/user_data_streams.cpp] |
| [Start-User-Data-Stream-Wsp.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [Keepalive-User-Data-Stream-Wsp.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [Close-User-Data-Stream-Wsp.md] | implemented | [ws-api/client.hpp], [ws-api/client.cpp] |
| [Event-Balance-and-Position-Update.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Order-Update.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Margin-Call.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-User-Data-Stream-Expired.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Account-Configuration-Update.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Algo-Order-Update.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Conditional-Order-Trigger-Reject.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-GRID-UPDATE.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-STRATEGY-UPDATE.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |
| [Event-Trade-Lite.md] | implemented | [streams/user_streams.hpp], [streams/user_streams.cpp], [types/streams.hpp] |

## WebSocket market stream docs

| Doc | Status | Source files |
|---|---|---|
| [Aggregate-Trade-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Mark-Price-Stream.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Individual-Symbol-Book-Ticker-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Diff-Book-Depth-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Mark-Price-Stream-for-All-market.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [All-Book-Tickers-Stream.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [All-Market-Liquidation-Order-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [All-Market-Mini-Tickers-Stream.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [All-Market-Tickers-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Composite-Index-Symbol-Information-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Continuous-Contract-Kline-Candlestick-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Contract-Info-Stream.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Diff-Book-Depth-Streams-RPI.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [How-to-manage-a-local-order-book-correctly.md] | implemented | [streams/local_order_book.hpp], [streams/local_order_book.cpp] |
| [Important-WebSocket-Change-Notice.md] | informational | Migration notice documenting `/public`, `/market`, `/private` URL split. Current code uses legacy `/ws` path in [config.hpp]. |
| [Individual-Symbol-Mini-Ticker-Stream.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |
| [Individual-Symbol-Ticker-Streams.md] | implemented | [streams/market_streams.hpp], [streams/market_streams.cpp], [types/streams.hpp] |


<!-- API doc links -->

[general-info.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/general-info.md
[common-definition.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md
[error-code.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/error-code.md
[websocket-api-general-info.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md
[websocket-market-streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams.md
[user-data-streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams.md
[convert.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert.md
[portfolio-margin-endpoints.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md

[Check-Server-Time.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Check-Server-Time.md
[Exchange-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
[Order-Book.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book.md
[Recent-Trades-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Recent-Trades-List.md
[Compressed-Aggregate-Trades-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Compressed-Aggregate-Trades-List.md
[Kline-Candlestick-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Kline-Candlestick-Data.md
[Continuous-Contract-Kline-Candlestick-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Continuous-Contract-Kline-Candlestick-Data.md
[Index-Price-Kline-Candlestick-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Price-Kline-Candlestick-Data.md
[Mark-Price-Kline-Candlestick-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price-Kline-Candlestick-Data.md
[Premium-Index-Kline-Data.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Premium-Index-Kline-Data.md
[Mark-Price.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Mark-Price.md
[Get-Funding-Rate-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-History.md
[Get-Funding-Rate-Info.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Get-Funding-Rate-Info.md
[Open-Interest.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest.md
[Open-Interest-Statistics.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Open-Interest-Statistics.md
[Top-Long-Short-Account-Ratio.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Top-Long-Short-Account-Ratio.md
[Top-Trader-Long-Short-Ratio.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Top-Trader-Long-Short-Ratio.md
[Long-Short-Ratio.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Long-Short-Ratio.md
[Taker-BuySell-Volume.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Taker-BuySell-Volume.md
[Basis.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Basis.md
[24hr-Ticker-Price-Change-Statistics.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/24hr-Ticker-Price-Change-Statistics.md
[Symbol-Price-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker.md
[Symbol-Price-Ticker-v2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Price-Ticker-v2.md
[Symbol-Order-Book-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Symbol-Order-Book-Ticker.md
[Delivery-Price.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delivery-Price.md
[Composite-Index-Symbol-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Composite-Index-Symbol-Information.md
[Index-Constituents.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Index-Constituents.md
[Multi-Assets-Mode-Asset-Index.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Multi-Assets-Mode-Asset-Index.md
[Insurance-Fund-Balance.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Insurance-Fund-Balance.md
[ADL-Risk.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/ADL-Risk.md
[Order-Book-RPI.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Order-Book-RPI.md
[Trading-Schedule.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Trading-Schedule.md
[Delist-Schedule.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Delist-Schedule.md
[Old-Trades-Lookup.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Old-Trades-Lookup.md

[market-data/websocket-api]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api
[WS-Symbol-Order-Book-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Order-Book-Ticker.md
[WS-Symbol-Price-Ticker.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Price-Ticker.md

[account/rest-api]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api
[Account-Information-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V2.md
[Account-Information-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Information-V3.md
[Futures-Account-Balance-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Account-Balance-V2.md
[Futures-Account-Balance-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Account-Balance-V3.md
[Account-Config.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Account-Config.md
[Symbol-Config.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Symbol-Config.md
[Get-Current-Multi-Assets-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Multi-Assets-Mode.md
[Get-Current-Position-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Current-Position-Mode.md
[Get-Income-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Income-History.md
[Notional-and-Leverage-Brackets.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Notional-and-Leverage-Brackets.md
[User-Commission-Rate.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/User-Commission-Rate.md
[Query-Rate-Limit.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Query-Rate-Limit.md
[Get-Future-Account-Transaction-History-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Future-Account-Transaction-History-List.md
[Get-Download-Id-For-Futures-Transaction-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Transaction-History.md
[Get-Futures-Transaction-History-Download-Link-by-Id.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Transaction-History-Download-Link-by-Id.md
[Get-Download-Id-For-Futures-Order-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Order-History.md
[Get-Futures-Order-History-Download-Link-by-Id.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Order-History-Download-Link-by-Id.md
[Get-Download-Id-For-Futures-Trade-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Download-Id-For-Futures-Trade-History.md
[Get-Futures-Trade-Download-Link-by-Id.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-Futures-Trade-Download-Link-by-Id.md
[Get-BNB-Burn-Status.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Get-BNB-Burn-Status.md
[Toggle-BNB-Burn-On-Futures-Trade.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Toggle-BNB-Burn-On-Futures-Trade.md
[Futures-Trading-Quantitative-Rules-Indicators.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/rest-api/Futures-Trading-Quantitative-Rules-Indicators.md

[WS-Account-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/websocket-api/Account-Information.md
[WS-Account-Information-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/websocket-api/Account-Information-V2.md
[WS-Futures-Account-Balance.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/websocket-api/Futures-Account-Balance.md

[New-Order-Test.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Order-Test.md
[Place-Multiple-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Place-Multiple-Orders.md
[Modify-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Order.md
[Modify-Multiple-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Multiple-Orders.md
[Query-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Order.md
[Cancel-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Order.md
[Cancel-Multiple-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Multiple-Orders.md
[Cancel-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Open-Orders.md
[Auto-Cancel-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Auto-Cancel-All-Open-Orders.md
[Query-Current-Open-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Current-Open-Order.md
[All-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/All-Orders.md
[Current-All-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Open-Orders.md
[Position-Information-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V2.md
[Position-Information-V3.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-Information-V3.md
[Position-ADL-Quantile-Estimation.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Position-ADL-Quantile-Estimation.md
[Users-Force-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Users-Force-Orders.md
[Account-Trade-List.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Account-Trade-List.md
[Change-Position-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Position-Mode.md
[Change-Multi-Assets-Mode.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Multi-Assets-Mode.md
[Change-Initial-Leverage.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Initial-Leverage.md
[Change-Margin-Type.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Change-Margin-Type.md
[Modify-Isolated-Position-Margin.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Modify-Isolated-Position-Margin.md
[Get-Position-Margin-Change-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Position-Margin-Change-History.md
[Get-Order-Modify-History.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Get-Order-Modify-History.md
[Current-All-Algo-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Algo-Open-Orders.md
[Cancel-All-Algo-Open-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Algo-Open-Orders.md
[New-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/New-Algo-Order.md
[Cancel-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-Algo-Order.md
[Query-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-Algo-Order.md
[Query-All-Algo-Orders.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Query-All-Algo-Orders.md
[TradFi-Perps.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/TradFi-Perps.md

[WS-New-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Algo-Order.md
[WS-Cancel-Algo-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Algo-Order.md
[WS-Modify-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Modify-Order.md
[WS-Query-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Query-Order.md
[WS-Cancel-Order.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Order.md
[WS-Position-Info-V2.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Info-V2.md
[WS-Position-Information.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Information.md

[Start-User-Data-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream.md
[Keepalive-User-Data-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Keepalive-User-Data-Stream.md
[Close-User-Data-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Close-User-Data-Stream.md
[Start-User-Data-Stream-Wsp.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream-Wsp.md
[Keepalive-User-Data-Stream-Wsp.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Keepalive-User-Data-Stream-Wsp.md
[Close-User-Data-Stream-Wsp.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Close-User-Data-Stream-Wsp.md
[Event-Balance-and-Position-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Balance-and-Position-Update.md
[Event-Order-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Order-Update.md
[Event-Margin-Call.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Margin-Call.md
[Event-User-Data-Stream-Expired.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-User-Data-Stream-Expired.md
[Event-Account-Configuration-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md
[Event-Algo-Order-Update.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md
[Event-Conditional-Order-Trigger-Reject.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md
[Event-GRID-UPDATE.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md
[Event-STRATEGY-UPDATE.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md
[Event-Trade-Lite.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md

[Aggregate-Trade-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Aggregate-Trade-Streams.md
[Mark-Price-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream.md
[Individual-Symbol-Book-Ticker-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Book-Ticker-Streams.md
[Diff-Book-Depth-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams.md
[Mark-Price-Stream-for-All-market.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Mark-Price-Stream-for-All-market.md
[All-Book-Tickers-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Book-Tickers-Stream.md
[All-Market-Liquidation-Order-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md
[All-Market-Mini-Tickers-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md
[All-Market-Tickers-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md
[Composite-Index-Symbol-Information-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Composite-Index-Symbol-Information-Streams.md
[Continuous-Contract-Kline-Candlestick-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Continuous-Contract-Kline-Candlestick-Streams.md
[Contract-Info-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Contract-Info-Stream.md
[Diff-Book-Depth-Streams-RPI.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Diff-Book-Depth-Streams-RPI.md
[How-to-manage-a-local-order-book-correctly.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/How-to-manage-a-local-order-book-correctly.md
[Important-WebSocket-Change-Notice.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Important-WebSocket-Change-Notice.md
[Individual-Symbol-Mini-Ticker-Stream.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Mini-Ticker-Stream.md
[Individual-Symbol-Ticker-Streams.md]: /docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Individual-Symbol-Ticker-Streams.md

<!-- Source file links -->

[client.cpp]: /src/binapi2/fapi/client.cpp
[signing.cpp]: /src/binapi2/fapi/signing.cpp
[signing.hpp]: /include/binapi2/fapi/signing.hpp
[client.hpp]: /include/binapi2/fapi/client.hpp
[error.hpp]: /include/binapi2/fapi/error.hpp
[result.hpp]: /include/binapi2/fapi/result.hpp
[config.hpp]: /include/binapi2/fapi/config.hpp

[types/enums.hpp]: /include/binapi2/fapi/types/enums.hpp
[types/common.hpp]: /include/binapi2/fapi/types/common.hpp
[types/market_data.hpp]: /include/binapi2/fapi/types/market_data.hpp
[types/account.hpp]: /include/binapi2/fapi/types/account.hpp
[types/trade.hpp]: /include/binapi2/fapi/types/trade.hpp
[types/streams.hpp]: /include/binapi2/fapi/types/streams.hpp
[types/convert.hpp]: /include/binapi2/fapi/types/convert.hpp
[types/websocket_api.hpp]: /include/binapi2/fapi/types/websocket_api.hpp

[rest/market_data.hpp]: /include/binapi2/fapi/rest/market_data.hpp
[rest/market_data.cpp]: /src/binapi2/fapi/rest/market_data.cpp
[rest/account.hpp]: /include/binapi2/fapi/rest/account.hpp
[rest/account.cpp]: /src/binapi2/fapi/rest/account.cpp
[rest/trade.hpp]: /include/binapi2/fapi/rest/trade.hpp
[rest/trade.cpp]: /src/binapi2/fapi/rest/trade.cpp
[rest/convert.hpp]: /include/binapi2/fapi/rest/convert.hpp
[rest/convert.cpp]: /src/binapi2/fapi/rest/convert.cpp
[rest/user_data_streams.hpp]: /include/binapi2/fapi/rest/user_data_streams.hpp
[rest/user_data_streams.cpp]: /src/binapi2/fapi/rest/user_data_streams.cpp
[rest/generated_endpoints.hpp]: /include/binapi2/fapi/rest/generated_endpoints.hpp

[ws-api/client.hpp]: /include/binapi2/fapi/websocket_api/client.hpp
[ws-api/client.cpp]: /src/binapi2/fapi/websocket_api/client.cpp
[ws-api/generated_methods.hpp]: /include/binapi2/fapi/websocket_api/generated_methods.hpp

[streams/market_streams.hpp]: /include/binapi2/fapi/streams/market_streams.hpp
[streams/market_streams.cpp]: /src/binapi2/fapi/streams/market_streams.cpp
[streams/user_streams.hpp]: /include/binapi2/fapi/streams/user_streams.hpp
[streams/user_streams.cpp]: /src/binapi2/fapi/streams/user_streams.cpp
[streams/local_order_book.hpp]: /include/binapi2/fapi/streams/local_order_book.hpp
[streams/local_order_book.cpp]: /src/binapi2/fapi/streams/local_order_book.cpp
