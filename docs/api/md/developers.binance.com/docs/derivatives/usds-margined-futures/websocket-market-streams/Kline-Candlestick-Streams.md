# Kline/Candlestick Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: The Kline/Candlestick Stream push updates to the current klines/candlestick every 250 milliseconds (if existing).
- **url_path**: `/market`
- **stream_name**: `<symbol>@kline_<interval>`
- **update_speed**: 250ms

### Response Examples

```javascript
{
"e": "kline", // Event type
"E": 1638747660000, // Event time
"s": "BTCUSDT", // Symbol
"k": {
"t": 1638747660000, // Kline start time
"T": 1638747719999, // Kline close time
"s": "BTCUSDT", // Symbol
"i": "1m", // Interval
"f": 100, // First trade ID
"L": 200, // Last trade ID
"o": "0.0010", // Open price
"c": "0.0020", // Close price
"h": "0.0025", // High price
"l": "0.0015", // Low price
"v": "1000", // Base asset volume
"n": 100, // Number of trades
"x": false, // Is this kline closed?
"q": "1.0000", // Quote asset volume
"V": "500", // Taker buy base asset volume
"Q": "0.500", // Taker buy quote asset volume
"B": "123456" // Ignore
}
}
```

## Stream Description

The Kline/Candlestick Stream push updates to the current klines/candlestick every 250 milliseconds (if existing).

Kline/Candlestick chart intervals:

m -> minutes; h -> hours; d -> days; w -> weeks; M -> months

- 1m
- 3m
- 5m
- 15m
- 30m
- 1h
- 2h
- 4h
- 6h
- 8h
- 12h
- 1d
- 3d
- 1w
- 1M

## URL PATH

`/market`

## Stream Name

`<symbol>@kline_<interval>`

## Update Speed

250ms

## Response Example

```javascript
{
"e": "kline", // Event type
"E": 1638747660000, // Event time
"s": "BTCUSDT", // Symbol
"k": {
"t": 1638747660000, // Kline start time
"T": 1638747719999, // Kline close time
"s": "BTCUSDT", // Symbol
"i": "1m", // Interval
"f": 100, // First trade ID
"L": 200, // Last trade ID
"o": "0.0010", // Open price
"c": "0.0020", // Close price
"h": "0.0025", // High price
"l": "0.0015", // Low price
"v": "1000", // Base asset volume
"n": 100, // Number of trades
"x": false, // Is this kline closed?
"q": "1.0000", // Quote asset volume
"V": "500", // Taker buy base asset volume
"Q": "0.500", // Taker buy quote asset volume
"B": "123456" // Ignore
}
}
```
