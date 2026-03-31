# Task - add local order-book management helper

## Objective

Implement a documented local order-book orchestration helper based on snapshot plus diff-stream synchronization.

## Work

- codify the procedure from [`How-to-manage-a-local-order-book-correctly.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/How-to-manage-a-local-order-book-correctly.md)
- integrate REST depth snapshots with diff-depth or partial-depth stream sequencing
- define recovery behavior for sequence gaps, reconnects, and stale snapshots
- expose a reusable helper rather than forcing every example to reimplement the algorithm
