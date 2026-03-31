# Task - add order-modify and algo RPC methods

## Objective

Complete the missing trade RPC methods around order mutation and algorithmic workflows.

## Work

- implement [`Modify-Order.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Modify-Order.md)
- implement [`New-Algo-Order.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/New-Algo-Order.md)
- implement [`Cancel-Algo-Order.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/websocket-api/Cancel-Algo-Order.md)
- ensure request serialization, auth signing, and correlated response parsing stay aligned with the existing client in [`src/binapi2/fapi/websocket_api/client.cpp`](src/binapi2/fapi/websocket_api/client.cpp)
