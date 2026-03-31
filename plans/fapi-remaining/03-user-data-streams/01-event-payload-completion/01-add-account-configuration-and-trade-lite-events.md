# Task - add account-configuration and trade-lite events

## Objective

Implement the remaining smaller but frequently useful user-data events.

## Work

- add event types and decoding for [`Event-Account-Configuration-Update-previous-Leverage-Update.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Account-Configuration-Update-previous-Leverage-Update.md)
- add event types and decoding for [`Event-Trade-Lite.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Trade-Lite.md)
- integrate new events into the dispatch loop in [`src/binapi2/fapi/streams/user_streams.cpp`](src/binapi2/fapi/streams/user_streams.cpp)
- preserve existing callback ergonomics for already implemented event families
