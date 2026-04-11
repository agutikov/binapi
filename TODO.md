






---

??? transport_logger usage?


---



design stream listen key keepalive






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
And does it make sense
provide analysis table

sort by expected: latency, throughput, complexity


--------------------------------------------------------------------------------


streams and wsapi optimization - is a wide infinite task, postpone it, but provide rough start analysis



--------------------------------------------------------------------------------



scripts/audit_types.py
clangd
joern
clang-refactor





