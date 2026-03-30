# Individual Symbol Mini Ticker Stream

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: 24hr rolling window mini-ticker statistics for a single symbol. These are NOT the statistics of the UTC day, but a 24hr rolling window from requestTime to 24hrs before.
- **url_path**: `/market`
- **stream_name**: `<symbol>@miniTicker`
- **update_speed**: 2s

### Response Examples

```javascript
{
"e": "24hrMiniTicker", // Event type
"E": 123456789, // Event time
"s": "BTCUSDT", // Symbol
"c": "0.0025", // Close price
"o": "0.0010", // Open price
"h": "0.0025", // High price
"l": "0.0010", // Low price
"v": "10000", // Total traded base asset volume
"q": "18" // Total traded quote asset volume
}
```

## Stream Description

24hr rolling window mini-ticker statistics for a single symbol. These are NOT the statistics of the UTC day, but a 24hr rolling window from requestTime to 24hrs before.

## URL PATH

`/market`

## Stream Name

`<symbol>@miniTicker`

## Update Speed

2s

## Response Example

```javascript
{
"e": "24hrMiniTicker", // Event type
"E": 123456789, // Event time
"s": "BTCUSDT", // Symbol
"c": "0.0025", // Close price
"o": "0.0010", // Open price
"h": "0.0025", // High price
"l": "0.0010", // Low price
"v": "10000", // Total traded base asset volume
"q": "18" // Total traded quote asset volume
}
```
