# Continuous Contract Kline/Candlestick Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Contract type:
- **url_path**: `/market`
- **stream_name**: `<pair>_<contractType>@continuousKline_<interval>`
- **update_speed**: 250ms

### Response Examples

```javascript
{
"e":"continuous_kline", // Event type
"E":1607443058651, // Event time
"ps":"BTCUSDT", // Pair
"ct":"PERPETUAL" // Contract type
"k":{
"t":1607443020000, // Kline start time
"T":1607443079999, // Kline close time
"i":"1m", // Interval
"f":116467658886, // First updateId
"L":116468012423, // Last updateId
"o":"18787.00", // Open price
"c":"18804.04", // Close price
"h":"18804.04", // High price
"l":"18786.54", // Low price
"v":"197.664", // volume
"n": 543, // Number of trades
"x":false, // Is this kline closed?
"q":"3715253.19494", // Quote asset volume
"V":"184.769", // Taker buy volume
"Q":"3472925.84746", //Taker buy quote asset volume
"B":"0" // Ignore
}
}
```

## Stream Description

Contract type:

- perpetual
- current_quarter
- next_quarter
- tradifi_perpetual

Kline/Candlestick chart intervals:

s -> seconds; m -> minutes; h -> hours; d -> days; w -> weeks; M -> months

- 1s
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

`<pair>_<contractType>@continuousKline_<interval>`

## Update Speed

250ms

## Response Example

```javascript
{
"e":"continuous_kline", // Event type
"E":1607443058651, // Event time
"ps":"BTCUSDT", // Pair
"ct":"PERPETUAL" // Contract type
"k":{
"t":1607443020000, // Kline start time
"T":1607443079999, // Kline close time
"i":"1m", // Interval
"f":116467658886, // First updateId
"L":116468012423, // Last updateId
"o":"18787.00", // Open price
"c":"18804.04", // Close price
"h":"18804.04", // High price
"l":"18786.54", // Low price
"v":"197.664", // volume
"n": 543, // Number of trades
"x":false, // Is this kline closed?
"q":"3715253.19494", // Quote asset volume
"V":"184.769", // Taker buy volume
"Q":"3472925.84746", //Taker buy quote asset volume
"B":"0" // Ignore
}
}
```
