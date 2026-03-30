# Individual Symbol Book Ticker Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Pushes any update to the best bid or ask's price or quantity in real-time for a specified symbol.
- **url_path**: `/public`
- **stream_name**: `<symbol>@bookTicker`
- **update_speed**: Real-time

### Response Examples

```javascript
{
"e":"bookTicker", // event type
"u":400900217, // order book updateId
"E": 1568014460893, // event time
"T": 1568014460891, // transaction time
"s":"BNBUSDT", // symbol
"b":"25.35190000", // best bid price
"B":"31.21000000", // best bid qty
"a":"25.36520000", // best ask price
"A":"40.66000000" // best ask qty
}
```

## Stream Description

Pushes any update to the best bid or ask's price or quantity in real-time for a specified symbol.

## URL PATH

`/public`

## Stream Name

`<symbol>@bookTicker`

Note:

> Retail Price Improvement(RPI) orders are not visible and excluded in the response message.

## Update Speed

Real-time

## Response Example

```javascript
{
"e":"bookTicker", // event type
"u":400900217, // order book updateId
"E": 1568014460893, // event time
"T": 1568014460891, // transaction time
"s":"BNBUSDT", // symbol
"b":"25.35190000", // best bid price
"B":"31.21000000", // best bid qty
"a":"25.36520000", // best ask price
"A":"40.66000000" // best ask qty
}
```
