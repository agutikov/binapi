# Task - decide and implement portfolio-margin coverage

## Objective

Resolve the currently TBD state of the portfolio-margin docs area.

## Work

- inspect [`portfolio-margin-endpoints.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/portfolio-margin-endpoints.md) and determine whether its documented operations belong in [`binapi2::fapi`](include/binapi2/fapi) or require a separate subsystem
- if the scope belongs in FAPI, define staged endpoint coverage, shared types, and public API boundaries
- if the scope should remain separate, document the exclusion clearly in the tracker and top-level plan so full FAPI completion is not ambiguously blocked
