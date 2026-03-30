# Kline/Candlestick Data

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Kline/candlestick bars for a symbol. Klines are uniquely identified by their open time.
- **http_method**: GET
- **path**: `/fapi/v1/klines`
- **http_request**: GET `/fapi/v1/klines`
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
1499040000000, // Open time
"0.01634790", // Open
"0.80000000", // High
"0.01575800", // Low
"0.01577100", // Close
"148976.11427815", // Volume
1499644799999, // Close time
"2434.19055334", // Quote asset volume
308, // Number of trades
"1756.87402397", // Taker buy base asset volume
"28.46694368", // Taker buy quote asset volume
"17928899.62484339" // Ignore.
]
]
```

## API Description

Kline/candlestick bars for a symbol. Klines are uniquely identified by their open time.

## HTTP Request

GET `/fapi/v1/klines`

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
1499040000000, // Open time
"0.01634790", // Open
"0.80000000", // High
"0.01575800", // Low
"0.01577100", // Close
"148976.11427815", // Volume
1499644799999, // Close time
"2434.19055334", // Quote asset volume
308, // Number of trades
"1756.87402397", // Taker buy base asset volume
"28.46694368", // Taker buy quote asset volume
"17928899.62484339" // Ignore.
]
]
```
