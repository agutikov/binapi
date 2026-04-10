







include/binapi2/fapi/streams/market_streams.hpp

what is string argument of async_connect ?

how multiple subscriptions per connection are expected to be used?
    - how multiple event types should be handled?
    - without variant?
    - while dynamic variant is impossible I don't see we can use it

hide async_read_text, provide only C++-ish events
    - and optional recoreder

do we need async_read_event while have generator?

shouldn't be stream_recorder async?
like:
    - websocket_client is producer of json strings
    - then goes optional configurable buffering
    - then goes 2 both optional consumers:
        - recoreder
        - parser
    - then parser is a consumer of strings, generator/producer of structs
        - with optional configurable output buffering
    - subscription == parser
    - how this can work with multiple subscriptions?
        - variant
        - multiple parsers per one connection (string producer)


Ok, basic_market_streams, while own transport - represent connection.
But then subscription should be represented by something - by some producer, source of events of certain type.



connection-related refactoring for streams

streams:
    - mapping (stream/generator) <-> (connection) <-> (service)
    - what is market_streams or user_streams now?


streams.md: what's missing
    - application buffering
    - separate connections (transport) from subscriptions
    - flexible composition:
        - multiple connections
        - multiple subscriptions per connection
        - balancing???
    - auto-reconnect
    - keepalive




--------------------------------------------------------------------------------


thread pool

example of stream pipelining:
    - network thread
    - parser thread
    - logic thread with local order book



I don't think auto-balanced thread bool is feasible now, right?


--------------------------------------------------------------------------------



binapi2-fapi-demo-cli load keys from storage

add support of both:
- libsecret
- systemd-creds


SecretProvider
 ├── LibSecretProvider
 ├── SystemdCredsProvider
 └── TestSecretProvider (for tests only)


implement this in a separate generic library

add required interface for SecretProvider in binapi2 that works with both implementation



--------------------------------------------------------------------------------




ws-api with keys
rest with keys
streams with keys



--------------------------------------------------------------------------------


finish testing secure endpoints, streams and ws api with testnet


all tests from demo-cli, including local order book



--------------------------------------------------------------------------------



binapi2 connect to real account



--------------------------------------------------------------------------------


record something



--------------------------------------------------------------------------------


review what would need to be done to move on:
    - Folly
    - Seastar
    - Boost.Fibers
    - CppCoro
    - liburing4cpp
    - libco
    - bthread
    - uvw
    - asynC++
    - asyncio(C++)
    - packio
    - libufinex

provide analysis table


--------------------------------------------------------------------------------



scripts/audit_types.py
with clangd or joern (on host)



Joern + clangd + clang-rename + mcp + python tooling  Agent/skill



or maybe skip all this until transition to fix8




