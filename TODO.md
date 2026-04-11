




ws-api with keys
rest with keys
streams with keys


---

add all requests and streams to demo-cli - every available API


--------------------------------------------------------------------------------


finish testing secure endpoints, streams and ws api with testnet


all tests from demo-cli, including local order book



--------------------------------------------------------------------------------



binapi2 connect to real account



--------------------------------------------------------------------------------


document overview of data types - rest, wsapi and streams:
- what contains
- what means
- what can do with the data
- duplication
    - between rest and wsapi
    - between streams and 2 other
- detailed explanation of the fields meaning



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

review reconnects, should not work theoretically...

--------------------------------------------------------------------------------

review security - secrets storage and usage

--------------------------------------------------------------------------------



scripts/audit_types.py
clangd
joern
clang-refactor





