# Get BNB Burn Status (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get user's BNB Fee Discount (Fee Discount On or Fee Discount Off )
- **http_method**: GET
- **path**: `/fapi/v1/feeBurn`
- **http_request**: GET `/fapi/v1/feeBurn`
- **request_weight**: 30

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"feeBurn": true // "true": Fee Discount On; "false": Fee Discount Off
}
```

## API Description

Get user's BNB Fee Discount (Fee Discount On or Fee Discount Off )

## HTTP Request

GET `/fapi/v1/feeBurn`

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
"feeBurn": true // "true": Fee Discount On; "false": Fee Discount Off
}
```
