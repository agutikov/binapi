# Individual Symbol Ticker Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: 24hr rolling window ticker statistics for a single symbol. These are NOT the statistics of the UTC day, but a 24hr rolling window from requestTime to 24hrs before.
- **url_path**: `/market`
- **stream_name**: `<symbol>@ticker`
- **update_speed**: 2000ms

### Response Examples

```javascript
{
"e": "24hrTicker", // Event type
"E": 123456789, // Event time
"s": "BTCUSDT", // Symbol
"p": "0.0015", // Price change
"P": "250.00", // Price change percent
"w": "0.0018", // Weighted average price
"c": "0.0025", // Last price
"Q": "10", // Last quantity
"o": "0.0010", // Open price
"h": "0.0025", // High price
"l": "0.0010", // Low price
"v": "10000", // Total traded base asset volume
"q": "18", // Total traded quote asset volume
"O": 0, // Statistics open time
"C": 86400000, // Statistics close time
"F": 0, // First trade ID
"L": 18150, // Last trade Id
"n": 18151 // Total number of trades
}
```

## Stream Description

24hr rolling window ticker statistics for a single symbol. These are NOT the statistics of the UTC day, but a 24hr rolling window from requestTime to 24hrs before.

## URL PATH

`/market`

## Stream Name

`<symbol>@ticker`

## Update Speed

2000ms

## Response Example

```javascript
{
"e": "24hrTicker", // Event type
"E": 123456789, // Event time
"s": "BTCUSDT", // Symbol
"p": "0.0015", // Price change
"P": "250.00", // Price change percent
"w": "0.0018", // Weighted average price
"c": "0.0025", // Last price
"Q": "10", // Last quantity
"o": "0.0010", // Open price
"h": "0.0025", // High price
"l": "0.0010", // Low price
"v": "10000", // Total traded base asset volume
"q": "18", // Total traded quote asset volume
"O": 0, // Statistics open time
"C": 86400000, // Statistics close time
"F": 0, // First trade ID
"L": 18150, // Last trade Id
"n": 18151 // Total number of trades
}
```
