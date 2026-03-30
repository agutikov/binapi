# User Commission Rate (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get User Commission Rate
- **http_method**: GET
- **path**: `/fapi/v1/commissionRate`
- **http_request**: GET `/fapi/v1/commissionRate`
- **request_weight**: 20

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"symbol": "BTCUSDT",
"makerCommissionRate": "0.0002", // 0.02%
"takerCommissionRate": "0.0004", // 0.04%
"rpiCommissionRate": "0.00005" // 0.005%
}
```

## API Description

Get User Commission Rate

## HTTP Request

GET `/fapi/v1/commissionRate`

## Request Weight

20

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```javascript
{
"symbol": "BTCUSDT",
"makerCommissionRate": "0.0002", // 0.02%
"takerCommissionRate": "0.0004", // 0.04%
"rpiCommissionRate": "0.00005" // 0.005%
}
```
