# Task - add all-market mini-ticker and ticker stream polish

## Objective

Finish the recently added all-market mini-ticker and all-market ticker support so it is production-ready, fully typed, and example-backed.

## Work

- verify payload shape fidelity for [`All-Market-Mini-Tickers-Stream.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Mini-Tickers-Stream.md) and [`All-Market-Tickers-Streams.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/All-Market-Tickers-Streams.md)
- confirm [`include/binapi2/fapi/types/streams.hpp`](include/binapi2/fapi/types/streams.hpp) covers array-envelope semantics and optional fields correctly
- add or refine examples under [`examples/binapi2/fapi`](examples/binapi2/fapi) to demonstrate iteration over array events
- ensure tracker entries are updated from TBD to implemented only after compile and payload review are complete
