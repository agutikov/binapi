







update documents
this is final version of binapi2 library
prepare for release - review and update all documentation
docs/binapi2
README.md


---


document overview of data types - rest, wsapi and streams:
- what contains
- what means
- what can do with the data
- duplication
    - between rest and wsapi
    - between streams and 2 other
- detailed explanation of the fields meaning

this should be an crypto exchange client library user guide in docs/binapi2/guide/
combining information from binance api docs, library docs and generic crypto trading info


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



================================================================================


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





