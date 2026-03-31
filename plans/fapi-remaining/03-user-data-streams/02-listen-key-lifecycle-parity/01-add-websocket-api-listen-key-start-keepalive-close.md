# Task - add WebSocket API listen-key start keepalive close

## Objective

Provide parity for the WSP listen-key lifecycle pages.

## Work

- implement typed request and response support for [`Start-User-Data-Stream-Wsp.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Start-User-Data-Stream-Wsp.md)
- implement typed request and response support for [`Keepalive-User-Data-Stream-Wsp.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Keepalive-User-Data-Stream-Wsp.md)
- implement typed request and response support for [`Close-User-Data-Stream-Wsp.md`](docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/user-data-streams/Close-User-Data-Stream-Wsp.md)
- align naming and behavior with the existing REST lifecycle surface in [`include/binapi2/fapi/rest/user_data_streams.hpp`](include/binapi2/fapi/rest/user_data_streams.hpp)
