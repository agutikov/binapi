# Query User Rate Limit (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query User Rate Limit
- **http_method**: GET
- **path**: `/fapi/v1/rateLimit/order`
- **http_request**: GET `/fapi/v1/rateLimit/order`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
[
{
"rateLimitType": "ORDERS",
"interval": "SECOND",
"intervalNum": 10,
"limit": 10000,
},
{
"rateLimitType": "ORDERS",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 20000,
}
]
```

## API Description

Query User Rate Limit

## HTTP Request

GET `/fapi/v1/rateLimit/order`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```javascript
[
{
"rateLimitType": "ORDERS",
"interval": "SECOND",
"intervalNum": 10,
"limit": 10000,
},
{
"rateLimitType": "ORDERS",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 20000,
}
]
```
