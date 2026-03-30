# Change Position Mode(TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Change user's position mode (Hedge Mode or One-way Mode ) on EVERY symbol
- **http_method**: POST
- **path**: `/fapi/v1/positionSide/dual`
- **http_request**: POST `/fapi/v1/positionSide/dual`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| dualSidePosition | STRING | YES | "true": Hedge Mode; "false": One-way Mode |
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

Change user's position mode (Hedge Mode or One-way Mode ) on EVERY symbol

## HTTP Request

POST `/fapi/v1/positionSide/dual`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| dualSidePosition | STRING | YES | "true": Hedge Mode; "false": One-way Mode |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "success"
}
```
