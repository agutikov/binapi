# Recent Trades List

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get recent market trades
- **http_method**: GET
- **path**: `/fapi/v1/trades`
- **http_request**: GET `/fapi/v1/trades`
- **request_weight**: 5

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 500; max 1000. |

### Request Parameter Notes

- Market trades means trades filled in the order book. Only market trades will be returned, which means the insurance fund trades and ADL trades won't be returned.

### Response Examples

```javascript
[
{
"id": 28457,
"price": "4.00000100",
"qty": "12.00000000",
"quoteQty": "48.00",
"time": 1499865549590,
"isBuyerMaker": true,
"isRPITrade": true,
}
]
```

## API Description

Get recent market trades

## HTTP Request

GET `/fapi/v1/trades`

## Request Weight

5

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 500; max 1000. |

> Market trades means trades filled in the order book. Only market trades will be returned, which means the insurance fund trades and ADL trades won't be returned.

## Response Example

```javascript
[
{
"id": 28457,
"price": "4.00000100",
"qty": "12.00000000",
"quoteQty": "48.00",
"time": 1499865549590,
"isBuyerMaker": true,
"isRPITrade": true,
}
]
```
