# Diff. Book Depth Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Bids and asks, pushed every 250 milliseconds, 500 milliseconds, 100 milliseconds (if existing)
- **url_path**: `/public`
- **stream_name**: `<symbol>@depth` OR `<symbol>@depth@500ms` OR `<symbol>@depth@100ms`
- **update_speed**: 250ms, 500ms, 100ms

### Response Examples

```javascript
{
"e": "depthUpdate", // Event type
"E": 123456789, // Event time
"T": 123456788, // Transaction time
"s": "BTCUSDT", // Symbol
"U": 157, // First update ID in event
"u": 160, // Final update ID in event
"pu": 149, // Final update Id in last stream(ie `u` in last stream)
"b": [ // Bids to be updated
[
"0.0024", // Price level to be updated
"10" // Quantity
]
],
"a": [ // Asks to be updated
[
"0.0026", // Price level to be updated
"100" // Quantity
]
]
}
```

## Stream Description

Bids and asks, pushed every 250 milliseconds, 500 milliseconds, 100 milliseconds (if existing)

## URL PATH

`/public`

## Stream Name

`<symbol>@depth` OR `<symbol>@depth@500ms` OR `<symbol>@depth@100ms`

Note:

> Retail Price Improvement(RPI) orders are not visible and excluded in the response message.

## Update Speed

250ms, 500ms, 100ms

## Response Example

```javascript
{
"e": "depthUpdate", // Event type
"E": 123456789, // Event time
"T": 123456788, // Transaction time
"s": "BTCUSDT", // Symbol
"U": 157, // First update ID in event
"u": 160, // Final update ID in event
"pu": 149, // Final update Id in last stream(ie `u` in last stream)
"b": [ // Bids to be updated
[
"0.0024", // Price level to be updated
"10" // Quantity
]
],
"a": [ // Asks to be updated
[
"0.0026", // Price level to be updated
"100" // Quantity
]
]
}
```
