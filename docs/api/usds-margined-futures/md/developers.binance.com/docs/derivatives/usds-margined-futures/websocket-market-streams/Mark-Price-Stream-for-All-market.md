# Mark Price Stream for All market

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Mark price and funding rate for all symbols pushed every 3 seconds or every second.
- **url_path**: `/market`
- **stream_name**: `!markPrice@arr` or `!markPrice@arr@1s`
- **update_speed**: 3000ms or 1000ms

### Response Examples

```javascript
[
{
"e": "markPriceUpdate", // Event type
"E": 1562305380000, // Event time
"s": "BTCUSDT", // Symbol
"p": "11185.87786614", // Mark price
"ap": "11185.87786614", // Mark price moving average
"i": "11784.62659091", // Index price
"P": "11784.25641265", // Estimated Settle Price, only useful in the last hour before the settlement starts
"r": "0.00030000", // Funding rate
"T": 1562306400000 // Next funding time
}
]
```

## Stream Description

Mark price and funding rate for all symbols pushed every 3 seconds or every second.

Note:

> TradFi symbols will be pushed through a seperate message.

## URL PATH

`/market`

## Stream Name

`!markPrice@arr` or `!markPrice@arr@1s`

## Update Speed

3000ms or 1000ms

## Response Example

```javascript
[
{
"e": "markPriceUpdate", // Event type
"E": 1562305380000, // Event time
"s": "BTCUSDT", // Symbol
"p": "11185.87786614", // Mark price
"ap": "11185.87786614", // Mark price moving average
"i": "11784.62659091", // Index price
"P": "11784.25641265", // Estimated Settle Price, only useful in the last hour before the settlement starts
"r": "0.00030000", // Funding rate
"T": 1562306400000 // Next funding time
}
]
```
