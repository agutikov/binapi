# Change Margin Type(TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Change symbol level margin type
- **http_method**: POST
- **path**: `/fapi/v1/marginType`
- **http_request**: POST `/fapi/v1/marginType`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| marginType | ENUM | YES | ISOLATED, CROSSED |
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

Change symbol level margin type

## HTTP Request

POST `/fapi/v1/marginType`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| marginType | ENUM | YES | ISOLATED, CROSSED |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "code": 200,
  "msg": "success"
}
```
