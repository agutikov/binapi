




review and update documents in docs/binapi2 according to current state, remove plans


--------------------------------------------------------------------------------




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

Throughput benchmarks w/o WS transport.
    - record some samples from testnet
    - write a benchmark that repeats those samples


--------------------------------------------------------------------------------


REST benchmark with postman mock

--------------------------------------------------------------------------------



connection pooling for REST

is it possible to pool connections for streams or ws_api?

update benchmark


--------------------------------------------------------------------------------


thread pool

update both benchmarks


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

