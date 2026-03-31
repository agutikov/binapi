# Task - add algo and conditional-order user events

## Objective

Implement algorithmic and rejection-related user-data events.

## Work

- add decoding for [`Event-Algo-Order-Update.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Algo-Order-Update.md)
- add decoding for [`Event-Conditional-Order-Trigger-Reject.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Event-Conditional-Order-Trigger-Reject.md)
- model nested payload fragments so later REST and websocket algo-order phases can reuse them
- update user-stream dispatcher tests or smoke examples accordingly
