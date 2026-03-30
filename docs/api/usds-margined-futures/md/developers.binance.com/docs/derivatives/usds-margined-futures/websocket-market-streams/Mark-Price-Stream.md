# Mark Price Stream

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Mark price and funding rate for a single symbol pushed every 3 seconds or every second.
- **url_path**: `/market`
- **stream_name**: `<symbol>@markPrice` or `<symbol>@markPrice@1s`
- **update_speed**: 3000ms or 1000ms

### Response Examples

```javascript
{
"e": "markPriceUpdate", // Event type
"E": 1562305380000, // Event time
"s": "BTCUSDT", // Symbol
"p": "11794.15000000", // Mark price
"ap": "11794.15000000", // Mark price moving average
"i": "11784.62659091", // Index price
"P": "11784.25641265", // Estimated Settle Price, only useful in the last hour before the settlement starts
"r": "0.00038167", // Funding rate
"T": 1562306400000 // Next funding time
}
```

## Stream Description

Mark price and funding rate for a single symbol pushed every 3 seconds or every second.

## URL PATH

`/market`

## Stream Name

`<symbol>@markPrice` or `<symbol>@markPrice@1s`

## Update Speed

3000ms or 1000ms

## Response Example

```javascript
{
"e": "markPriceUpdate", // Event type
"E": 1562305380000, // Event time
"s": "BTCUSDT", // Symbol
"p": "11794.15000000", // Mark price
"ap": "11794.15000000", // Mark price moving average
"i": "11784.62659091", // Index price
"P": "11784.25641265", // Estimated Settle Price, only useful in the last hour before the settlement starts
"r": "0.00038167", // Funding rate
"T": 1562306400000 // Next funding time
}
```
