# Get Download Id For Futures Order History (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get Download Id For Futures Order History
- **http_method**: GET
- **path**: `/fapi/v1/order/asyn`
- **http_request**: GET `/fapi/v1/order/asyn`
- **request_weight**: 1000

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| startTime | LONG | YES | Timestamp in ms |
| endTime | LONG | YES | Timestamp in ms |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Request Limitation is 10 times per month, shared by front end download page and rest api The time between `startTime` and `endTime` can not be longer than 1 year

### Response Examples

```javascript
{
"avgCostTimestampOfLast30d":7241837, // Average time taken for data download in the past 30 days
"downloadId":"546975389218332672",
}
```

## API Description

Get Download Id For Futures Order History

## HTTP Request

GET `/fapi/v1/order/asyn`

## Request Weight

1000

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| startTime | LONG | YES | Timestamp in ms |
| endTime | LONG | YES | Timestamp in ms |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Request Limitation is 10 times per month, shared by front end download page and rest api The time between `startTime` and `endTime` can not be longer than 1 year

## Response Example

```javascript
{
"avgCostTimestampOfLast30d":7241837, // Average time taken for data download in the past 30 days
"downloadId":"546975389218332672",
}
```
