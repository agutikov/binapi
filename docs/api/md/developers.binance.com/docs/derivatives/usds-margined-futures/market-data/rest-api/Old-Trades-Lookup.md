# Old Trades Lookup (MARKET_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get older market historical trades.
- **http_method**: GET
- **path**: `/fapi/v1/historicalTrades`
- **http_request**: GET `/fapi/v1/historicalTrades`
- **request_weight**: 20

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 100; max 500. |
| fromId | LONG | NO | TradeId to fetch from. Default gets most recent trades. |

### Request Parameter Notes

- Market trades means trades filled in the order book. Only market trades will be returned, which means the insurance fund trades and ADL trades won't be returned. Only supports data from within the last one month

### Response Examples

```javascript
[
{
"id": 28457,
"price": "4.00000100",
"qty": "12.00000000",
"quoteQty": "8000.00",
"time": 1499865549590,
"isBuyerMaker": true,
"isRPITrade": true,
}
]
```

## API Description

Get older market historical trades.

## HTTP Request

GET `/fapi/v1/historicalTrades`

## Request Weight

20

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 100; max 500. |
| fromId | LONG | NO | TradeId to fetch from. Default gets most recent trades. |

> Market trades means trades filled in the order book. Only market trades will be returned, which means the insurance fund trades and ADL trades won't be returned. Only supports data from within the last one month

## Response Example

```javascript
[
{
"id": 28457,
"price": "4.00000100",
"qty": "12.00000000",
"quoteQty": "8000.00",
"time": 1499865549590,
"isBuyerMaker": true,
"isRPITrade": true,
}
]
```
