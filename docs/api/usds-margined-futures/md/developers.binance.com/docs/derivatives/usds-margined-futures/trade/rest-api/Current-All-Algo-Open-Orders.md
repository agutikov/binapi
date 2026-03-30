# Current All Algo Open Orders (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get all algo open orders on a symbol.
- **http_method**: GET
- **path**: `/fapi/v1/openAlgoOrders`
- **http_request**: GET `/fapi/v1/openAlgoOrders`
- **request_weight**: 1 for a single symbol; 40 when the symbol parameter is omitted

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoType | STRING | NO |  |
| symbol | STRING | NO |  |
| algoId | LONG | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- If the symbol is not sent, orders for all symbols will be returned in an array.

### Response Examples

```json
[
  {
    "algoId": 2148627,
    "clientAlgoId": "MRumok0dkhrP4kCm12AHaB",
    "algoType": "CONDITIONAL",
    "orderType": "TAKE_PROFIT",
    "symbol": "BNBUSDT",
    "side": "SELL",
    "positionSide": "BOTH",
    "timeInForce": "GTC",
    "quantity": "0.01",
    "algoStatus": "NEW",
    "actualOrderId": "",
    "actualPrice": "0.00000",
    "triggerPrice": "750.000",
    "price": "750.000",
    "icebergQuantity": null,
    "tpTriggerPrice": "0.000",
    "tpPrice": "0.000",
    "slTriggerPrice": "0.000",
    "slPrice": "0.000",
    "tpOrderType": "",
    "selfTradePreventionMode": "EXPIRE_MAKER",
    "workingType": "CONTRACT_PRICE",
    "priceMatch": "NONE",
    "closePosition": false,
    "priceProtect": false,
    "reduceOnly": false,
    "createTime": 1750514941540,
    "updateTime": 1750514941540,
    "triggerTime": 0,
    "goodTillDate": 0
  }
]
```

## API Description

Get all algo open orders on a symbol.

## HTTP Request

GET `/fapi/v1/openAlgoOrders`

## Request Weight

1 for a single symbol; 40 when the symbol parameter is omitted

Careful when accessing this with no symbol.

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoType | STRING | NO |  |
| symbol | STRING | NO |  |
| algoId | LONG | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> If the symbol is not sent, orders for all symbols will be returned in an array.

## Response Example

```json
[
  {
    "algoId": 2148627,
    "clientAlgoId": "MRumok0dkhrP4kCm12AHaB",
    "algoType": "CONDITIONAL",
    "orderType": "TAKE_PROFIT",
    "symbol": "BNBUSDT",
    "side": "SELL",
    "positionSide": "BOTH",
    "timeInForce": "GTC",
    "quantity": "0.01",
    "algoStatus": "NEW",
    "actualOrderId": "",
    "actualPrice": "0.00000",
    "triggerPrice": "750.000",
    "price": "750.000",
    "icebergQuantity": null,
    "tpTriggerPrice": "0.000",
    "tpPrice": "0.000",
    "slTriggerPrice": "0.000",
    "slPrice": "0.000",
    "tpOrderType": "",
    "selfTradePreventionMode": "EXPIRE_MAKER",
    "workingType": "CONTRACT_PRICE",
    "priceMatch": "NONE",
    "closePosition": false,
    "priceProtect": false,
    "reduceOnly": false,
    "createTime": 1750514941540,
    "updateTime": 1750514941540,
    "triggerTime": 0,
    "goodTillDate": 0
  }
]
```
