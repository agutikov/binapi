# Task - add live subscribe and unsubscribe control

## Objective

Support the runtime control-message flow documented in [`Live-Subscribing-Unsubscribing-to-streams.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/Live-Subscribing-Unsubscribing-to-streams.md).

## Work

- extend websocket stream transport or stream manager abstractions to send subscribe, unsubscribe, list-subscriptions, and property control frames if documented
- define typed control payloads and typed success or error handling
- preserve backward compatibility for the existing direct-connect helpers
- add example coverage for dynamic subscription management
