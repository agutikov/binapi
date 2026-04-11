






---

stream_buffer, all 4 APIs and 3 buffers = 12 combinations:
- tests
- benchmarks

benchmark for stream_recorder - for generic , asio and spdlog implementations

---


add async_read_event to user_stream - like dynamic stream it produces only one variant type

make a table of streams and their features in docs

---


stream application buffering for multithread implementation

building block for buffering between async_read_text and parser already exist - stream_buffer

IMPORTANT: preserve existing async_read_event and generators

separate more the subscription mechanism and messages consuming
can we use consumer as template argument like transport?


What would be the generic aproach for optional buffering (and composition) of:
- string messages for recording
- string messages for transferring to separate parser
- materialized C++ structs


finally:
example of stream pipelining in examples/binapi2/fapi/async-demo-cli
    - network thread
    - parser thread
    - logic thread with local order book
In addition to current single-thread async order book


---

??? transport_logger usage?


---



design stream listen key keepalive; show for review






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


streams and wsapi optimization - is a wide infinite task, postpone it, but provide rough start analysis



--------------------------------------------------------------------------------



scripts/audit_types.py
with clangd or joern (on host)



Joern + clangd + clang-rename + mcp + python tooling  Agent/skill



or maybe skip all this until transition to fix8




