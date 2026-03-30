# Trading Session Stream

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Trading session information for the underlying assets of TradFi Perpetual contracts—covering the U.S. equity market and the commodity market—is updated every second. Trading session information for different underlying markets is pushed in separate messages. Session types for the equity market include "PRE_MARKET", "REGULAR", "AFTER_MARKET", "OVERNIGHT", and "NO_TRADING". Session types for the commodity market include "REGULAR" and "NO_TRADING".
- **url_path**: `/market`
- **stream_name**: `tradingSession`
- **update_speed**: 1s

### Response Examples

```javascript
{
"e": "EquityUpdate", // Event type, can also be CommodityUpdate
"E": 1765244143062, // Event time
"t": 1765242000000, // Session start time
"T": 1765270800000, // Session end time
"S": "OVERNIGHT" // Session type
}
```

## Stream Description

Trading session information for the underlying assets of TradFi Perpetual contracts—covering the U.S. equity market and the commodity market—is updated every second. Trading session information for different underlying markets is pushed in separate messages. Session types for the equity market include "PRE_MARKET", "REGULAR", "AFTER_MARKET", "OVERNIGHT", and "NO_TRADING". Session types for the commodity market include "REGULAR" and "NO_TRADING".

## URL PATH

`/market`

## Stream Name

`tradingSession`

## Update Speed

1s

## Response Example

```javascript
{
"e": "EquityUpdate", // Event type, can also be CommodityUpdate
"E": 1765244143062, // Event time
"t": 1765242000000, // Session start time
"T": 1765270800000, // Session end time
"S": "OVERNIGHT" // Session type
}
```
