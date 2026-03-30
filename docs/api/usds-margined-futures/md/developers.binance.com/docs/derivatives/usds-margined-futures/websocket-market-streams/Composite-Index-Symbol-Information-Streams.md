# Composite Index Symbol Information Streams

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: Composite index information for index symbols pushed every second.
- **url_path**: `/market`
- **stream_name**: `<symbol>@compositeIndex`
- **update_speed**: 1000ms

### Response Examples

```javascript
{
"e":"compositeIndex", // Event type
"E":1602310596000, // Event time
"s":"DEFIUSDT", // Symbol
"p":"554.41604065", // Price
"C":"baseAsset",
"c":[ // Composition
{
"b":"BAL", // Base asset
"q":"USDT", // Quote asset
"w":"1.04884844", // Weight in quantity
"W":"0.01457800", // Weight in percentage
"i":"24.33521021" // Index price
},
{
"b":"BAND",
"q":"USDT" ,
"w":"3.53782729",
"W":"0.03935200",
"i":"7.26420084"
}
]
}
```

## Stream Description

Composite index information for index symbols pushed every second.

## URL PATH

`/market`

## Stream Name

`<symbol>@compositeIndex`

## Update Speed

1000ms

## Response Example

```javascript
{
"e":"compositeIndex", // Event type
"E":1602310596000, // Event time
"s":"DEFIUSDT", // Symbol
"p":"554.41604065", // Price
"C":"baseAsset",
"c":[ // Composition
{
"b":"BAL", // Base asset
"q":"USDT", // Quote asset
"w":"1.04884844", // Weight in quantity
"W":"0.01457800", // Weight in percentage
"i":"24.33521021" // Index price
},
{
"b":"BAND",
"q":"USDT" ,
"w":"3.53782729",
"W":"0.03935200",
"i":"7.26420084"
}
]
}
```
