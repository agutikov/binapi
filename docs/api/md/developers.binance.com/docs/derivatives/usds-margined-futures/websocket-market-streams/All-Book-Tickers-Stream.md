# All Book Tickers Stream

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Pushes any update to the best bid or ask's price or quantity in real-time for all symbols.
- **url_path**: `/public`
- **stream_name**: `!bookTicker`
- **update_speed**: 5s

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

Pushes any update to the best bid or ask's price or quantity in real-time for all symbols.

## URL PATH

`/public`

## Stream Name

`!bookTicker`

Note:

> Retail Price Improvement(RPI) orders are not visible and excluded in the response message.

## Update Speed

5s

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
