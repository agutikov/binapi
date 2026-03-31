# Task - add account RPC version and position methods

## Objective

Complete the missing account and position RPC methods closest to current support.

## Work

- implement the explicit V2 variant from [`Account-Information-V2.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/account/websocket-api/Account-Information-V2.md)
- implement [`Position-Info-V2.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Info-V2.md)
- implement [`Position-Information.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Position-Information.md)
- ensure typed response envelopes reuse account and trade structs introduced in earlier REST phases where possible
