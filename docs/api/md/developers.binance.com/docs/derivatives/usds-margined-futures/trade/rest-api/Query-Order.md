# Query Order (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Check an order's status.
- **http_method**: GET
- **path**: `/fapi/v1/order`
- **http_request**: GET `/fapi/v1/order`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| orderId | LONG | NO |  |
| origClientOrderId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Notes:
- Either `orderId` or `origClientOrderId` must be sent. `orderId` is self-increment for each specific `symbol`

### Response Examples

```json
{
"avgPrice": "0.00000",
"clientOrderId": "abc",
"cumQuote": "0",
"executedQty": "0",
"orderId": 1917641,
"origQty": "0.40",
"origType": "TRAILING_STOP_MARKET",
"price": "0",
"reduceOnly": false,
"side": "BUY",
"positionSide": "SHORT",
"status": "NEW",
"stopPrice": "9300", // please ignore when order type is TRAILING_STOP_MARKET
"closePosition": false, // if Close-All
"symbol": "BTCUSDT",
"time": 1579276756075, // order time
"timeInForce": "GTC",
"type": "TRAILING_STOP_MARKET",
"activatePrice": "9020", // activation price, only return with TRAILING_STOP_MARKET order
"priceRate": "0.3", // callback rate, only return with TRAILING_STOP_MARKET order
"updateTime": 1579276756075, // update time
"workingType": "CONTRACT_PRICE",
"priceProtect": false // if conditional order trigger is protected
}
```

## API Description

Check an order's status.

- These orders will not be found: order status is `CANCELED` or `EXPIRED` AND order has NO filled trade AND created time + 3 days < current time order create time + 90 days < current time
  - order status is `CANCELED` or `EXPIRED` AND order has NO filled trade AND created time + 3 days < current time
  - order create time + 90 days < current time


## HTTP Request

GET `/fapi/v1/order`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| orderId | LONG | NO |  |
| origClientOrderId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

Notes:

> Either `orderId` or `origClientOrderId` must be sent. `orderId` is self-increment for each specific `symbol`

## Response Example

```json
{
"avgPrice": "0.00000",
"clientOrderId": "abc",
"cumQuote": "0",
"executedQty": "0",
"orderId": 1917641,
"origQty": "0.40",
"origType": "TRAILING_STOP_MARKET",
"price": "0",
"reduceOnly": false,
"side": "BUY",
"positionSide": "SHORT",
"status": "NEW",
"stopPrice": "9300", // please ignore when order type is TRAILING_STOP_MARKET
"closePosition": false, // if Close-All
"symbol": "BTCUSDT",
"time": 1579276756075, // order time
"timeInForce": "GTC",
"type": "TRAILING_STOP_MARKET",
"activatePrice": "9020", // activation price, only return with TRAILING_STOP_MARKET order
"priceRate": "0.3", // callback rate, only return with TRAILING_STOP_MARKET order
"updateTime": 1579276756075, // update time
"workingType": "CONTRACT_PRICE",
"priceProtect": false // if conditional order trigger is protected
}
```
