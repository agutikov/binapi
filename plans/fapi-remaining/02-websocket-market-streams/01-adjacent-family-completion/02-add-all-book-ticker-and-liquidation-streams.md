# Task - add all-book-ticker and liquidation streams

## Objective

Complete the all-book-ticker, symbol liquidation, and all-market liquidation families.

## Work

- validate or finish support for [`All-Book-Tickers-Stream.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Book-Tickers-Stream.md)
- validate or finish support for [`Liquidation-Order-Streams.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Liquidation-Order-Streams.md) and [`All-Market-Liquidation-Order-Streams.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Liquidation-Order-Streams.md)
- confirm symbol-required versus all-market addressing rules are enforced in [`src/binapi2/fapi/streams/market_streams.cpp`](src/binapi2/fapi/streams/market_streams.cpp)
- review payload structs for nested order payload fidelity and optional fields
- add examples for both single-symbol and all-market flows where useful
