# Task - add grid and strategy user events

## Objective

Implement the remaining strategy-oriented user-data event families.

## Work

- add decoding for [`Event-GRID-UPDATE.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-GRID-UPDATE.md)
- add decoding for [`Event-STRATEGY-UPDATE.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-STRATEGY-UPDATE.md)
- decide whether dedicated handler registration is needed or if tagged event dispatch is sufficient
- add representative fixtures or examples for the new strategy payloads
