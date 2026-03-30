# Get Position Margin Change History (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get Position Margin Change History
- **http_method**: GET
- **path**: `/fapi/v1/positionMargin/history`
- **http_request**: GET `/fapi/v1/positionMargin/history`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| type | INT | NO | 1: Add position margin，2: Reduce position margin |
| startTime | LONG | NO |  |
| endTime | LONG | NO | Default current time if not pass |
| limit | INT | NO | Default: 500 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Support querying future histories that are not older than 30 days The time between `startTime` and `endTime`can't be more than 30 days

### Response Examples

```json
[
  {
    "symbol": "BTCUSDT",
    "type": 1,
    "deltaType": "USER_ADJUST",
    "amount": "23.36332311",
    "asset": "USDT",
    "time": 1578047897183,
    "positionSide": "BOTH"
  },
  {
    "symbol": "BTCUSDT",
    "type": 1,
    "deltaType": "USER_ADJUST",
    "amount": "100",
    "asset": "USDT",
    "time": 1578047900425,
    "positionSide": "LONG"
  }
]
```

## API Description

Get Position Margin Change History

## HTTP Request

GET `/fapi/v1/positionMargin/history`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| type | INT | NO | 1: Add position margin，2: Reduce position margin |
| startTime | LONG | NO |  |
| endTime | LONG | NO | Default current time if not pass |
| limit | INT | NO | Default: 500 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Support querying future histories that are not older than 30 days The time between `startTime` and `endTime`can't be more than 30 days

## Response Example

```json
[
  {
    "symbol": "BTCUSDT",
    "type": 1,
    "deltaType": "USER_ADJUST",
    "amount": "23.36332311",
    "asset": "USDT",
    "time": 1578047897183,
    "positionSide": "BOTH"
  },
  {
    "symbol": "BTCUSDT",
    "type": 1,
    "deltaType": "USER_ADJUST",
    "amount": "100",
    "asset": "USDT",
    "time": 1578047900425,
    "positionSide": "LONG"
  }
]
```
