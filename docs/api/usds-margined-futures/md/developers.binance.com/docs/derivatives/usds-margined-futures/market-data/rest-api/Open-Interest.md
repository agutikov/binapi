# Open Interest

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get present open interest of a specific symbol.
- **http_method**: GET
- **path**: `/fapi/v1/openInterest`
- **http_request**: GET `/fapi/v1/openInterest`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |

### Response Examples

```javascript
{
"openInterest": "10659.509",
"symbol": "BTCUSDT",
"time": 1589437530011 // Transaction time
}
```

## API Description

Get present open interest of a specific symbol.

## HTTP Request

GET `/fapi/v1/openInterest`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |

## Response Example

```javascript
{
"openInterest": "10659.509",
"symbol": "BTCUSDT",
"time": 1589437530011 // Transaction time
}
```
