# RPI Diff. Book Depth Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Bids and asks including RPI orders, pushed every 500 milliseconds
- **url_path**: `/public`
- **stream_name**: `<symbol>@rpiDepth@500ms`
- **update_speed**: 500ms

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

Bids and asks including RPI orders, pushed every 500 milliseconds

## URL PATH

`/public`

## Stream Name

`<symbol>@rpiDepth@500ms`

Note:

> RPI(Retail Price Improvement) orders are included and aggreated in the response message. When the quantity of a price level to be updated is equal to 0, it means either all quotations for this price have been filled/canceled, or the quantity of crossed RPI orders for this price are hidden

## Update Speed

500ms

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
