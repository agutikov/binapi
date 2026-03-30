# Symbol Price Ticker(Deprecated)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Latest price for a symbol or symbols.
- **http_method**: GET
- **path**: `/fapi/v1/ticker/price`
- **http_request**: GET `/fapi/v1/ticker/price`

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Request Parameter Notes

- If the symbol is not sent, prices for all symbols will be returned in an array.

### Response Examples

```javascript
{
"symbol": "BTCUSDT",
"price": "6000.01",
"time": 1589437530011 // Transaction time
}
```

```json
[
  {
    "symbol": "BTCUSDT",
    "price": "6000.01",
    "time": 1589437530011
  }
]
```

## API Description

Latest price for a symbol or symbols.

## HTTP Request

GET `/fapi/v1/ticker/price`

Weight:

1 for a single symbol; 2 when the symbol parameter is omitted

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

> If the symbol is not sent, prices for all symbols will be returned in an array.

## Response Example

```javascript
{
"symbol": "BTCUSDT",
"price": "6000.01",
"time": 1589437530011 // Transaction time
}
```

> OR

```json
[
  {
    "symbol": "BTCUSDT",
    "price": "6000.01",
    "time": 1589437530011
  }
]
```
