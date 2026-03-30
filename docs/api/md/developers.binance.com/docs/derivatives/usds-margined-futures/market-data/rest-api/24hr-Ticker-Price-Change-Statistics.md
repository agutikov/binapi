# 24hr Ticker Price Change Statistics

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: 24 hour rolling window price change statistics. Careful when accessing this with no symbol.
- **http_method**: GET
- **path**: `/fapi/v1/ticker/24hr`
- **http_request**: GET `/fapi/v1/ticker/24hr`
- **request_weight**: 1 for a single symbol; 40 when the symbol parameter is omitted

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Request Parameter Notes

- If the symbol is not sent, tickers for all symbols will be returned in an array.

### Response Examples

```javascript
{
"symbol": "BTCUSDT",
"priceChange": "-94.99999800",
"priceChangePercent": "-95.960",
"weightedAvgPrice": "0.29628482",
"lastPrice": "4.00000200",
"lastQty": "200.00000000",
"openPrice": "99.00000000",
"highPrice": "100.00000000",
"lowPrice": "0.10000000",
"volume": "8913.30000000",
"quoteVolume": "15.30000000",
"openTime": 1499783499040,
"closeTime": 1499869899040,
"firstId": 28385, // First tradeId
"lastId": 28460, // Last tradeId
"count": 76 // Trade count
}
```

```javascript
[
{
"symbol": "BTCUSDT",
"priceChange": "-94.99999800",
"priceChangePercent": "-95.960",
"weightedAvgPrice": "0.29628482",
"lastPrice": "4.00000200",
"lastQty": "200.00000000",
"openPrice": "99.00000000",
"highPrice": "100.00000000",
"lowPrice": "0.10000000",
"volume": "8913.30000000",
"quoteVolume": "15.30000000",
"openTime": 1499783499040,
"closeTime": 1499869899040,
"firstId": 28385, // First tradeId
"lastId": 28460, // Last tradeId
"count": 76 // Trade count
}
]
```

## API Description

24 hour rolling window price change statistics. Careful when accessing this with no symbol.

## HTTP Request

GET `/fapi/v1/ticker/24hr`

## Request Weight

1 for a single symbol; 40 when the symbol parameter is omitted

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

> If the symbol is not sent, tickers for all symbols will be returned in an array.

## Response Example

> Response:

```javascript
{
"symbol": "BTCUSDT",
"priceChange": "-94.99999800",
"priceChangePercent": "-95.960",
"weightedAvgPrice": "0.29628482",
"lastPrice": "4.00000200",
"lastQty": "200.00000000",
"openPrice": "99.00000000",
"highPrice": "100.00000000",
"lowPrice": "0.10000000",
"volume": "8913.30000000",
"quoteVolume": "15.30000000",
"openTime": 1499783499040,
"closeTime": 1499869899040,
"firstId": 28385, // First tradeId
"lastId": 28460, // Last tradeId
"count": 76 // Trade count
}
```

> OR

```javascript
[
{
"symbol": "BTCUSDT",
"priceChange": "-94.99999800",
"priceChangePercent": "-95.960",
"weightedAvgPrice": "0.29628482",
"lastPrice": "4.00000200",
"lastQty": "200.00000000",
"openPrice": "99.00000000",
"highPrice": "100.00000000",
"lowPrice": "0.10000000",
"volume": "8913.30000000",
"quoteVolume": "15.30000000",
"openTime": 1499783499040,
"closeTime": 1499869899040,
"firstId": 28385, // First tradeId
"lastId": 28460, // Last tradeId
"count": 76 // Trade count
}
]
```
