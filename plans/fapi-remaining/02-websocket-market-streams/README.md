# Phase 2 - websocket market-stream completion

This phase completes the remaining market-stream families around the already working subscription and read-loop model in [`include/binapi2/fapi/streams/market_streams.hpp`](include/binapi2/fapi/streams/market_streams.hpp) and [`src/binapi2/fapi/streams/market_streams.cpp`](src/binapi2/fapi/streams/market_streams.cpp).

It finishes the direct stream-family coverage before introducing more invasive websocket session behavior changes.

## Stages

- [`01-adjacent-family-completion`](plans/fapi-remaining/02-websocket-market-streams/01-adjacent-family-completion)
- [`02-specialized-market-streams`](plans/fapi-remaining/02-websocket-market-streams/02-specialized-market-streams)
- [`03-session-behavior-and-order-book-support`](plans/fapi-remaining/02-websocket-market-streams/03-session-behavior-and-order-book-support)
