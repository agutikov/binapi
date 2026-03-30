# Toggle BNB Burn On Futures Trade (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Change user's BNB Fee Discount (Fee Discount On or Fee Discount Off ) on EVERY symbol
- **http_method**: POST
- **path**: `/fapi/v1/feeBurn`
- **http_request**: POST `/fapi/v1/feeBurn`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| feeBurn | STRING | YES | "true": Fee Discount On; "false": Fee Discount Off |
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

Change user's BNB Fee Discount (Fee Discount On or Fee Discount Off ) on EVERY symbol

## HTTP Request

POST `/fapi/v1/feeBurn`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| feeBurn | STRING | YES | "true": Fee Discount On; "false": Fee Discount Off |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "success"
}
```
