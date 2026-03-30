# All Market Liquidation Order Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: The All Liquidation Order Snapshot Streams push force liquidation order information for all symbols in the market. For each symbol，only the latest one liquidation order within 1000ms will be pushed as the snapshot. If no liquidation happens in the interval of 1000ms, no stream will be pushed.
- **url_path**: `/market`
- **stream_name**: `!forceOrder@arr`
- **update_speed**: 1000ms

### Response Examples

```javascript
{

"e":"forceOrder", // Event Type
"E":1568014460893, // Event Time
"o":{

"s":"BTCUSDT", // Symbol
"S":"SELL", // Side
"o":"LIMIT", // Order Type
"f":"IOC", // Time in Force
"q":"0.014", // Original Quantity
"p":"9910", // Price
"ap":"9910", // Average Price
"X":"FILLED", // Order Status
"l":"0.014", // Order Last Filled Quantity
"z":"0.014", // Order Filled Accumulated Quantity
"T":1568014460893, // Order Trade Time
}
}
```

## Stream Description

The All Liquidation Order Snapshot Streams push force liquidation order information for all symbols in the market. For each symbol，only the latest one liquidation order within 1000ms will be pushed as the snapshot. If no liquidation happens in the interval of 1000ms, no stream will be pushed.

## URL PATH

`/market`

## Stream Name

`!forceOrder@arr`

## Update Speed

1000ms

## Response Example

```javascript
{

"e":"forceOrder", // Event Type
"E":1568014460893, // Event Time
"o":{

"s":"BTCUSDT", // Symbol
"S":"SELL", // Side
"o":"LIMIT", // Order Type
"f":"IOC", // Time in Force
"q":"0.014", // Original Quantity
"p":"9910", // Price
"ap":"9910", // Average Price
"X":"FILLED", // Order Status
"l":"0.014", // Order Last Filled Quantity
"z":"0.014", // Order Filled Accumulated Quantity
"T":1568014460893, // Order Trade Time
}
}
```
