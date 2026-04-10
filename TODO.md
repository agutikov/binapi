













design stream listen key keepalive; I will review before implementation



---


stream application buffering for multithread implementation

building block for buffering

example of stream pipelining:
    - network thread
    - parser thread
    - logic thread with local order book





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




