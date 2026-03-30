# Accept the offered quote (USER_DATA)

- **protocol**: reference
- **category**: convert
- **meta_description**: API Description

## API Summary

- **description**: Accept the offered quote by quote ID.
- **http_method**: POST
- **path**: `/fapi/v1/convert/acceptQuote`
- **http_request**: POST `/fapi/v1/convert/acceptQuote`
- **request_weight**: 200(IP)

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| quoteId | STRING | YES |  |
| recvWindow | LONG | NO | The value cannot be greater than 60000 |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"orderId":"933256278426274426",
"createTime":1623381330472,
"orderStatus":"PROCESS" //PROCESS/ACCEPT_SUCCESS/SUCCESS/FAIL
}
```

## API Description

Accept the offered quote by quote ID.

## HTTP Request

POST `/fapi/v1/convert/acceptQuote`

## Request Weight

200(IP)

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| quoteId | STRING | YES |  |
| recvWindow | LONG | NO | The value cannot be greater than 60000 |
| timestamp | LONG | YES |  |

## Response Example

```javascript
{
"orderId":"933256278426274426",
"createTime":1623381330472,
"orderStatus":"PROCESS" //PROCESS/ACCEPT_SUCCESS/SUCCESS/FAIL
}
```
