











review and update documents in docs/binapi2 according to current state, remove plans



--------------------------------------------------------------------------------



Throughput benchmarks w/o WS transport.
    - record some samples from testnet
        - or generate if no keys available
    - write a benchmark that repeats those samples


--------------------------------------------------------------------------------



user streams

and other clarifications


parsing json into variant



--------------------------------------------------------------------------------


REST benchmark with postman mock

--------------------------------------------------------------------------------




connection pooling for REST and WSAPI


---

connection-related refactoring for streams

streams:
    - mapping stream/generator-connection-service
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


---

update benchmarks


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


implement this in a separate library

add required interface for SecretProvider in binapi2 that works with both implementation




--------------------------------------------------------------------------------


finish testing secure endpoints, streams and ws api with testnet


all tests from demo-cli, including local order book



--------------------------------------------------------------------------------



binapi2 connect to real account

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

