# Cancel Algo Order (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Cancel an active algo order.
- **http_method**: DELETE
- **path**: `/fapi/v1/algoOrder`
- **http_request**: DELETE `/fapi/v1/algoOrder`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoId | LONG | NO |  |
| clientAlgoId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Either `algoId` or `clientAlgoId` must be sent.

### Response Examples

```json
{
  "algoId": 2146760,
  "clientAlgoId": "6B2I9XVcJpCjqPAJ4YoFX7",
  "code": "200",
  "msg": "success"
}
```

## API Description

Cancel an active algo order.

## HTTP Request

DELETE `/fapi/v1/algoOrder`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoId | LONG | NO |  |
| clientAlgoId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Either `algoId` or `clientAlgoId` must be sent.

## Response Example

```json
{
  "algoId": 2146760,
  "clientAlgoId": "6B2I9XVcJpCjqPAJ4YoFX7",
  "code": "200",
  "msg": "success"
}
```
