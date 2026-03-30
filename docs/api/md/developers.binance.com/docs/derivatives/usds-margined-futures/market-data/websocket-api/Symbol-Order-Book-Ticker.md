# Symbol Order Book Ticker

- **protocol**: websocket_api
- **category**: websocket-api
- **meta_description**: API Description

## API Summary

- **description**: Best price/qty on the order book for a symbol or symbols.
- **method**: `ticker.book`
- **request_weight**: 2 for a single symbol; 5 when the symbol parameter is omitted

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Request Parameter Notes

- If the symbol is not sent, bookTickers for all symbols will be returned in an array. The field `X-MBX-USED-WEIGHT-1M` in response header is not accurate from this endpoint, please ignore.

### Request Examples

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "method": "ticker.book",
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
"lastUpdateId": 1027024,
"symbol": "BTCUSDT",
"bidPrice": "4.00000000",
"bidQty": "431.00000000",
"askPrice": "4.00000200",
"askQty": "9.00000000",
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
      "lastUpdateId": 1027024,
      "symbol": "BTCUSDT",
      "bidPrice": "4.00000000",
      "bidQty": "431.00000000",
      "askPrice": "4.00000200",
      "askQty": "9.00000000",
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

Best price/qty on the order book for a symbol or symbols.

## Method

`ticker.book`

Note:

> Retail Price Improvement(RPI) orders are not visible and excluded in the response message.

## Request

```json
{
  "id": "9d32157c-a556-4d27-9866-66760a174b57",
  "method": "ticker.book",
  "params": {
    "symbol": "BTCUSDT"
  }
}
```

## Request Weight

2 for a single symbol; 5 when the symbol parameter is omitted

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

> If the symbol is not sent, bookTickers for all symbols will be returned in an array. The field `X-MBX-USED-WEIGHT-1M` in response header is not accurate from this endpoint, please ignore.

## Response Example

```javascript
{
"id": "9d32157c-a556-4d27-9866-66760a174b57",
"status": 200,
"result": {
"lastUpdateId": 1027024,
"symbol": "BTCUSDT",
"bidPrice": "4.00000000",
"bidQty": "431.00000000",
"askPrice": "4.00000200",
"askQty": "9.00000000",
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
      "lastUpdateId": 1027024,
      "symbol": "BTCUSDT",
      "bidPrice": "4.00000000",
      "bidQty": "431.00000000",
      "askPrice": "4.00000200",
      "askQty": "9.00000000",
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
