# Get Order Modify History (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get order modification history
- **http_method**: GET
- **path**: `/fapi/v1/orderAmendment`
- **http_request**: GET `/fapi/v1/orderAmendment`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| orderId | LONG | NO |  |
| origClientOrderId | STRING | NO |  |
| startTime | LONG | NO | Timestamp in ms to get modification history from INCLUSIVE |
| endTime | LONG | NO | Timestamp in ms to get modification history until INCLUSIVE |
| limit | INT | NO | Default 50; max 100 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Either `orderId` or `origClientOrderId` must be sent, and the `orderId` will prevail if both are sent. Order modify history longer than 3 month is not avaliable

### Response Examples

```javascript
[
{
"amendmentId": 5363, // Order modification ID
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629184560899, // Order modification time
"amendment": {
"price": {
"before": "30004",
"after": "30003.2"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 3 // Order modification count, representing the number of times the order has been modified
}
},
{
"amendmentId": 5361,
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629184533946,
"amendment": {
"price": {
"before": "30005",
"after": "30004"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 2
}
},
{
"amendmentId": 5325,
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629182711787,
"amendment": {
"price": {
"before": "30002",
"after": "30005"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 1
}
}
]
```

## API Description

Get order modification history

## HTTP Request

GET `/fapi/v1/orderAmendment`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| orderId | LONG | NO |  |
| origClientOrderId | STRING | NO |  |
| startTime | LONG | NO | Timestamp in ms to get modification history from INCLUSIVE |
| endTime | LONG | NO | Timestamp in ms to get modification history until INCLUSIVE |
| limit | INT | NO | Default 50; max 100 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Either `orderId` or `origClientOrderId` must be sent, and the `orderId` will prevail if both are sent. Order modify history longer than 3 month is not avaliable

## Response Example

```javascript
[
{
"amendmentId": 5363, // Order modification ID
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629184560899, // Order modification time
"amendment": {
"price": {
"before": "30004",
"after": "30003.2"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 3 // Order modification count, representing the number of times the order has been modified
}
},
{
"amendmentId": 5361,
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629184533946,
"amendment": {
"price": {
"before": "30005",
"after": "30004"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 2
}
},
{
"amendmentId": 5325,
"symbol": "BTCUSDT",
"pair": "BTCUSDT",
"orderId": 20072994037,
"clientOrderId": "LJ9R4QZDihCaS8UAOOLpgW",
"time": 1629182711787,
"amendment": {
"price": {
"before": "30002",
"after": "30005"
},
"origQty": {
"before": "1",
"after": "1"
},
"count": 1
}
}
]
```
