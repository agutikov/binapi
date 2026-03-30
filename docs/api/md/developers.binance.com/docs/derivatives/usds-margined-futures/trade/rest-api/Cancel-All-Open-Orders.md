# Cancel All Open Orders (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Cancel All Open Orders
- **http_method**: DELETE
- **path**: `/fapi/v1/allOpenOrders`
- **http_request**: DELETE `/fapi/v1/allOpenOrders`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```json
{
  "code": 200,
  "msg": "The operation of cancel all open order is done."
}
```

## API Description

Cancel All Open Orders

## HTTP Request

DELETE `/fapi/v1/allOpenOrders`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "The operation of cancel all open order is done."
}
```
