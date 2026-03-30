# Mark Price Kline/Candlestick Data

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Kline/candlestick bars for the mark price of a symbol. Klines are uniquely identified by their open time.
- **http_method**: GET
- **path**: `/fapi/v1/markPriceKlines`
- **http_request**: GET `/fapi/v1/markPriceKlines`
- **request_weight**: based on parameter `LIMIT`

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| interval | ENUM | YES |  |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |
| limit | INT | NO | Default 500; max 1500. |

### Request Parameter Notes

- If startTime and endTime are not sent, the most recent klines are returned.

### Response Examples

```javascript
[
[
1591256460000, // Open time
"9653.29201333", // Open
"9654.56401333", // High
"9653.07367333", // Low
"9653.07367333", // Close (or latest price)
"0 ", // Ignore
1591256519999, // Close time
"0", // Ignore
60, // Ignore
"0", // Ignore
"0", // Ignore
"0" // Ignore
]
]
```

## API Description

Kline/candlestick bars for the mark price of a symbol. Klines are uniquely identified by their open time.

## HTTP Request

GET `/fapi/v1/markPriceKlines`

## Request Weight

based on parameter `LIMIT`

| LIMIT | weight |
| --- | --- |
| [1,100) | 1 |
| [100, 500) | 2 |
| [500, 1000] | 5 |
| > 1000 | 10 |

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| interval | ENUM | YES |  |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |
| limit | INT | NO | Default 500; max 1500. |

> If startTime and endTime are not sent, the most recent klines are returned.

## Response Example

```javascript
[
[
1591256460000, // Open time
"9653.29201333", // Open
"9654.56401333", // High
"9653.07367333", // Low
"9653.07367333", // Close (or latest price)
"0 ", // Ignore
1591256519999, // Close time
"0", // Ignore
60, // Ignore
"0", // Ignore
"0", // Ignore
"0" // Ignore
]
]
```
