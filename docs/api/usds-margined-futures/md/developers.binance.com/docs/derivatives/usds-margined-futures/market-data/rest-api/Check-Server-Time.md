# Check Server Time

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Test connectivity to the Rest API and get the current server time.
- **http_method**: GET
- **path**: `/fapi/v1/time`
- **http_request**: GET `/fapi/v1/time`
- **request_weight**: 1

### Request Parameter Notes

- NONE

### Response Examples

```json
{
  "serverTime": 1499827319559
}
```

## API Description

Test connectivity to the Rest API and get the current server time.

## HTTP Request

GET `/fapi/v1/time`

## Request Weight

1

## Request Parameters

NONE

## Response Example

```json
{
  "serverTime": 1499827319559
}
```
