# Futures TradFi Perps Contract(USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Sign TradFi-Perps agreement contract
- **http_method**: POST
- **path**: `/fapi/v1/stock/contract`
- **http_request**: POST `/fapi/v1/stock/contract`

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```json
{
  "code": 200,
  "msg": "success"
}
```

## API Description

Sign TradFi-Perps agreement contract

## HTTP Request

POST `/fapi/v1/stock/contract`

## Request Weigh

50

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "success"
}
```
