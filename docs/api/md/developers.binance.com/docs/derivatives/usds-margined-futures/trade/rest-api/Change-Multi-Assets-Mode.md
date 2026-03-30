# Change Multi-Assets Mode (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Change user's Multi-Assets mode (Multi-Assets Mode or Single-Asset Mode) on Every symbol
- **http_method**: POST
- **path**: `/fapi/v1/multiAssetsMargin`
- **http_request**: POST `/fapi/v1/multiAssetsMargin`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| multiAssetsMargin | STRING | YES | "true": Multi-Assets Mode; "false": Single-Asset Mode |
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

Change user's Multi-Assets mode (Multi-Assets Mode or Single-Asset Mode) on Every symbol

## HTTP Request

POST `/fapi/v1/multiAssetsMargin`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| multiAssetsMargin | STRING | YES | "true": Multi-Assets Mode; "false": Single-Asset Mode |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "success"
}
```
