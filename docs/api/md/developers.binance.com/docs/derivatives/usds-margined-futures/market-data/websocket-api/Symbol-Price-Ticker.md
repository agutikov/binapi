# Symbol Price Ticker

- **protocol**: websocket_api
- **category**: websocket-api
- **meta_description**: API Description

## API Summary

- **description**: Latest price for a symbol or symbols.
- **method**: `ticker.price`

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Request Parameter Notes

- If the symbol is not sent, prices for all symbols will be returned in an array.

### Request Examples

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "method": "ticker.price",
  "params": {
    "symbol": "BTCUSDT"
  }
}
```

### Response Examples

```javascript
{
"id": "9d32157c-a556-4d27-9866-66760a174b57",
"status": 200,
"result": {
"symbol": "BTCUSDT",
"price": "6000.01",
"time": 1589437530011 // Transaction time
},
"rateLimits": [
{
"rateLimitType": "REQUEST_WEIGHT",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 2400,
"count": 2
}
]
}
```

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "status": 200,
  "result": [
    {
      "symbol": "BTCUSDT",
      "price": "6000.01",
      "time": 1589437530011
    }
  ],
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 2
    }
  ]
}
```

## API Description

Latest price for a symbol or symbols.

## Method

`ticker.price`

## Request

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "method": "ticker.price",
  "params": {
    "symbol": "BTCUSDT"
  }
}
```

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
"id": "9d32157c-a556-4d27-9866-66760a174b57",
"status": 200,
"result": {
"symbol": "BTCUSDT",
"price": "6000.01",
"time": 1589437530011 // Transaction time
},
"rateLimits": [
{
"rateLimitType": "REQUEST_WEIGHT",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 2400,
"count": 2
}
]
}
```

> OR

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "status": 200,
  "result": [
    {
      "symbol": "BTCUSDT",
      "price": "6000.01",
      "time": 1589437530011
    }
  ],
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 2
    }
  ]
}
```
