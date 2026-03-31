# Task - add Convert quote, accept, and status REST surface

## Objective

Implement the full USD-M Futures Convert REST slice.

## Work

- implement [`Send-quote-request.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Send-quote-request.md)
- implement [`Accept-Quote.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Accept-Quote.md)
- implement [`Order-Status.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/convert/Order-Status.md)
- add a dedicated Convert service surface under [`include/binapi2/fapi/rest`](include/binapi2/fapi/rest) and matching typed models under [`include/binapi2/fapi/types`](include/binapi2/fapi/types)
- add at least one example that demonstrates the full quote to accept to status flow without implying unsafe live-trading defaults
