# FAPI remaining implementation plan

This hierarchy turns the remaining USD-M Futures work identified in [`plans/status.md`](plans/status.md) into execution-ready planning artifacts.

## Structure

- top level folders are phases
- nested folders are stages when a phase has a large or tightly related task set
- each phase and stage has a dedicated [`README.md`](plans/fapi-remaining/README.md)
- each task is a separate markdown file focused on one actionable implementation outcome

## Planning rules used

- order follows the most implementation-efficient path, while still respecting the major domain boundaries in [`plans/binapi2-fapi-implementation-plan.md`](plans/binapi2-fapi-implementation-plan.md)
- tasks assume the current implemented baseline documented in [`plans/status.md`](plans/status.md)
- validation and tracker maintenance are part of the plan, not deferred cleanup

## Phase overview

1. [`01-foundation-and-inventory`](plans/fapi-remaining/01-foundation-and-inventory)
2. [`02-websocket-market-streams`](plans/fapi-remaining/02-websocket-market-streams)
3. [`03-user-data-streams`](plans/fapi-remaining/03-user-data-streams)
4. [`04-public-rest-market-data`](plans/fapi-remaining/04-public-rest-market-data)
5. [`05-authenticated-rest-account-and-trade`](plans/fapi-remaining/05-authenticated-rest-account-and-trade)
6. [`06-websocket-api-rpc`](plans/fapi-remaining/06-websocket-api-rpc)
7. [`07-convert-and-portfolio-margin`](plans/fapi-remaining/07-convert-and-portfolio-margin)
8. [`08-validation-examples-and-closeout`](plans/fapi-remaining/08-validation-examples-and-closeout)
