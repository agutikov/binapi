# Get Current Multi-Assets Mode (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get user's Multi-Assets mode (Multi-Assets Mode or Single-Asset Mode) on Every symbol
- **http_method**: GET
- **path**: `/fapi/v1/multiAssetsMargin`
- **http_request**: GET `/fapi/v1/multiAssetsMargin`
- **request_weight**: 30

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"multiAssetsMargin": true // "true": Multi-Assets Mode; "false": Single-Asset Mode
}
```

## API Description

Get user's Multi-Assets mode (Multi-Assets Mode or Single-Asset Mode) on Every symbol

## HTTP Request

GET `/fapi/v1/multiAssetsMargin`

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
"multiAssetsMargin": true // "true": Multi-Assets Mode; "false": Single-Asset Mode
}
```
