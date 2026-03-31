# Task - align status inventory with code surface

## Objective

Reconcile the inventory in [`plans/status.md`](plans/status.md) with the actual current FAPI tree under [`include/binapi2/fapi`](include/binapi2/fapi), [`src/binapi2/fapi`](src/binapi2/fapi), and [`examples/binapi2/fapi`](examples/binapi2/fapi).

## Work

- verify every implemented item cited by the tracker still maps to a concrete public declaration and implementation
- normalize naming so tracker entries match endpoint and stream helper names used in [`include/binapi2/fapi/rest/generated_endpoints.hpp`](include/binapi2/fapi/rest/generated_endpoints.hpp) and [`include/binapi2/fapi/websocket_api/generated_methods.hpp`](include/binapi2/fapi/websocket_api/generated_methods.hpp)
- add explicit notes where one helper covers multiple docs pages or one docs page maps to multiple variants
- mark examples that already demonstrate implemented coverage versus examples still missing

## Deliverables

- updated implementation inventory notes
- reduced ambiguity for later tracker closeout
