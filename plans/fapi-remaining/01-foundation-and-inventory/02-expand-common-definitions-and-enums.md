# Task - expand common definitions and enums

## Objective

Broaden the shared type layer so later phases do not need to invent local one-off enum or fragment definitions.

## Work

- complete missing common definitions from [`/docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/common-definition.md)
- extend [`include/binapi2/fapi/types/enums.hpp`](include/binapi2/fapi/types/enums.hpp) and [`include/binapi2/fapi/types/common.hpp`](include/binapi2/fapi/types/common.hpp) with closed-set values needed by remaining account, trade, stream, and convert payloads
- centralize reusable fragments for download jobs, schedule entries, configuration payloads, account mode payloads, and algorithmic order fragments
- ensure Glaze mappings are consistent for all newly introduced reusable structs

## Deliverables

- broader shared type vocabulary
- fewer duplicate structs in later phases
