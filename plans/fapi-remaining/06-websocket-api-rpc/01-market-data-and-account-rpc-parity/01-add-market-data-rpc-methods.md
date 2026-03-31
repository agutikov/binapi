# Task - add market-data RPC methods

## Objective

Complete the currently missing market-data RPC methods.

## Work

- implement the overview page currently tracked as [`market-data/websocket-api/md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/md) if it maps to a distinct order-book RPC method
- implement [`Symbol-Price-Ticker.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api/Symbol-Price-Ticker.md)
- align typed method declarations in [`include/binapi2/fapi/websocket_api/generated_methods.hpp`](include/binapi2/fapi/websocket_api/generated_methods.hpp) with request and response models in the websocket type layer
- add examples showing request id correlation and typed success or error handling
