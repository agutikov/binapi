# Symbol Configuration(USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get current account symbol configuration.
- **http_method**: GET
- **path**: `/fapi/v1/symbolConfig`
- **http_request**: GET `/fapi/v1/symbolConfig`
- **request_weight**: 5

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
[
{
"symbol": "BTCUSDT",
"marginType": "CROSSED",
"isAutoAddMargin": false,
"leverage": 21,
"maxNotionalValue": "1000000",
}
]
```

## API Description

Get current account symbol configuration.

## HTTP Request

GET `/fapi/v1/symbolConfig`



## Request Weight

5

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```javascript
[
{
"symbol": "BTCUSDT",
"marginType": "CROSSED",
"isAutoAddMargin": false,
"leverage": 21,
"maxNotionalValue": "1000000",
}
]
```
