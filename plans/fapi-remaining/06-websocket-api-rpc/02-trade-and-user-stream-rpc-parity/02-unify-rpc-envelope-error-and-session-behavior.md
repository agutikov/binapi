# Task - unify RPC envelope, error, and session behavior

## Objective

Harden the RPC layer so the growing method surface remains maintainable.

## Work

- review request id generation, session logon state, signed method flow, and error envelope handling against [`/docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-api-general-info.md)
- factor repeated response parsing and outbound method wiring into reusable helpers if current handwritten client methods are becoming repetitive
- ensure WSP user-data lifecycle methods from Phase 3 plug naturally into the same RPC framework
