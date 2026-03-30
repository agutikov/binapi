# Get Current Position Mode(USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get user's position mode (Hedge Mode or One-way Mode ) on EVERY symbol
- **http_method**: GET
- **path**: `/fapi/v1/positionSide/dual`
- **http_request**: GET `/fapi/v1/positionSide/dual`
- **request_weight**: 30

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"dualSidePosition": true // "true": Hedge Mode; "false": One-way Mode
}
```

## API Description

Get user's position mode (Hedge Mode or One-way Mode ) on EVERY symbol

## HTTP Request

GET `/fapi/v1/positionSide/dual`

## Request Weight

30

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```javascript
{
"dualSidePosition": true // "true": Hedge Mode; "false": One-way Mode
}
```
