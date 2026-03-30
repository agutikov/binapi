# Change Initial Leverage(TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Change user's initial leverage of specific symbol market.
- **http_method**: POST
- **path**: `/fapi/v1/leverage`
- **http_request**: POST `/fapi/v1/leverage`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| leverage | INT | YES | target initial leverage: int from 1 to 125 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```json
{
  "leverage": 21,
  "maxNotionalValue": "1000000",
  "symbol": "BTCUSDT"
}
```

## API Description

Change user's initial leverage of specific symbol market.

## HTTP Request

POST `/fapi/v1/leverage`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| leverage | INT | YES | target initial leverage: int from 1 to 125 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "leverage": 21,
  "maxNotionalValue": "1000000",
  "symbol": "BTCUSDT"
}
```
