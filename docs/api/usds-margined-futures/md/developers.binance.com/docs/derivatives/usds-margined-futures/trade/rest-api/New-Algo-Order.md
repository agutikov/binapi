# New Algo Order(TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Send in a new Algo order.
- **http_method**: POST
- **path**: `/fapi/v1/algoOrder`
- **http_request**: POST `/fapi/v1/algoOrder`
- **request_weight**: 0 on IP rate limit(x-mbx-used-weight-1m)

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoType | ENUM | YES | Only support `CONDITIONAL` |
| symbol | STRING | YES |  |
| side | ENUM | YES |  |
| positionSide | ENUM | NO | Default `BOTH` for One-way Mode ; `LONG` or `SHORT` for Hedge Mode. It must be sent in Hedge Mode. |
| type | ENUM | YES | For `CONDITIONAL` algoType, `STOP_MARKET`/`TAKE_PROFIT_MARKET`/`STOP`/`TAKE_PROFIT`/`TRAILING_STOP_MARKET` as order type |
| timeInForce | ENUM | NO | `IOC` or `GTC` or `FOK` or `GTX` , default `GTC` |
| quantity | DECIMAL | NO | Cannot be sent with `closePosition`=`true`(Close-All) |
| price | DECIMAL | NO |  |
| triggerPrice | DECIMAL | NO |  |
| workingType | ENUM | NO | triggerPrice triggered by: `MARK_PRICE`, `CONTRACT_PRICE`. Default `CONTRACT_PRICE` |
| priceMatch | ENUM | NO | only avaliable for `LIMIT`/`STOP`/`TAKE_PROFIT` order; can be set to `OPPONENT`/ `OPPONENT_5`/ `OPPONENT_10`/ `OPPONENT_20`: /`QUEUE`/ `QUEUE_5`/ `QUEUE_10`/ `QUEUE_20`; Can't be passed together with `price` |
| closePosition | STRING | NO | true, false；Close-All，used with `STOP_MARKET` or `TAKE_PROFIT_MARKET`. |
| priceProtect | STRING | NO | "TRUE" or "FALSE", default "FALSE". Used with `STOP_MARKET` or `TAKE_PROFIT_MARKET` order. when price reaches the triggerPrice ，the difference rate between "MARK_PRICE" and "CONTRACT_PRICE" cannot be larger than the Price Protection Threshold of the symbol. |
| reduceOnly | STRING | NO | "true" or "false". default "false". Cannot be sent in Hedge Mode; cannot be sent with `closePosition`=`true` |
| activatePrice | DECIMAL | NO | Used with `TRAILING_STOP_MARKET` orders, default as the latest price(supporting different `workingType`) |
| callbackRate | DECIMAL | NO | Used with `TRAILING_STOP_MARKET` orders, min 0.1, max 10 where 1 for 1% |
| clientAlgoId | STRING | NO | A unique id among open orders. Automatically generated if not sent. Can only be string following the rule: `^[\.A-Z\:/a-z0-9_-]{1,36}$` |
| newOrderRespType | ENUM | NO | "ACK", "RESULT", default "ACK" |
| selfTradePreventionMode | ENUM | NO | `EXPIRE_TAKER`:expire taker order when STP triggers/ `EXPIRE_MAKER`:expire taker order when STP triggers/ `EXPIRE_BOTH`:expire both orders when STP triggers; default `NONE` |
| goodTillDate | LONG | NO | order cancel time for timeInForce `GTD`, mandatory when `timeInforce` set to `GTD`; order the timestamp only retains second-level precision, ms part will be ignored; The goodTillDate timestamp must be greater than the current time plus 600 seconds and smaller than 253402300799000 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Algo order with type `STOP`, parameter `timeInForce` can be sent ( default `GTC`). Algo order with type `TAKE_PROFIT`, parameter `timeInForce` can be sent ( default `GTC`).
- Condition orders will be triggered when: If parameter`priceProtect`is sent as true: when price reaches the `triggerPrice` ，the difference rate between "MARK_PRICE" and "CONTRACT_PRICE" cannot be larger than the "triggerProtect" of the symbol "triggerProtect" of a symbol can be got from `GET /fapi/v1/exchangeInfo` `STOP`, `STOP_MARKET`: BUY: latest price ("MARK_PRICE" or "CONTRACT_PRICE") >= `triggerPrice` SELL: latest price ("MARK_PRICE" or "CONTRACT_PRICE") <= `triggerPrice` `TAKE_PROFIT`, `TAKE_PROFIT_MARKET`: BUY: latest price ("MARK_PRICE" or "CONTRACT_PRICE") <= `triggerPrice` SELL: latest price ("MARK_PRICE" or "CONTRACT_PRICE") >= `triggerPrice` `TRAILING_STOP_MARKET`: BUY: the lowest price after order placed <= `activatePrice`, and the latest price >= the lowest price * (1 + `callbackRate`) SELL: the highest price after order placed >= `activatePrice`, and the latest price <= the highest price * (1 - `callbackRate`) For `TRAILING_STOP_MARKET`, if you got such error code. `{"code": -2021, "msg": "Order would immediately trigger."}` means that the parameters you send do not meet the following requirements: BUY: `activatePrice` should be smaller than latest price. SELL: `activatePrice` should be larger than latest price. `STOP_MARKET`, `TAKE_PROFIT_MARKET` with `closePosition`=`true`: Follow the same rules for condition orders. If triggered，close all current long position( if `SELL`) or current short position( if `BUY`). Cannot be used with `quantity` paremeter Cannot be used with `reduceOnly` parameter In Hedge Mode,cannot be used with `BUY` orders in `LONG` position side. and cannot be used with `SELL` orders in `SHORT` position side `selfTradePreventionMode` is only effective when `timeInForce` set to `IOC` or `GTC` or `GTD`.

### Response Examples

```javascript
{
"algoId": 2146760,
"clientAlgoId": "6B2I9XVcJpCjqPAJ4YoFX7",
"algoType": "CONDITIONAL",
"orderType": "TAKE_PROFIT",
"symbol": "BNBUSDT",
"side": "SELL",
"positionSide": "BOTH",
"timeInForce": "GTC",
"quantity": "0.01",
"algoStatus": "NEW",
"triggerPrice": "750.000",
"price": "750.000",
"icebergQuantity": null,
"selfTradePreventionMode": "EXPIRE_MAKER",
"workingType": "CONTRACT_PRICE",
"priceMatch": "NONE",
"closePosition": false,
"priceProtect": false,
"reduceOnly": false,
"activatePrice": "", //TRAILING_STOP_MARKET order
"callbackRate": "", //TRAILING_STOP_MARKET order
"createTime": 1750485492076,
"updateTime": 1750485492076,
"triggerTime": 0,
"goodTillDate": 0
}
```

## API Description

Send in a new Algo order.

## HTTP Request

POST `/fapi/v1/algoOrder`

## Request Weight

0 on IP rate limit(x-mbx-used-weight-1m)

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoType | ENUM | YES | Only support `CONDITIONAL` |
| symbol | STRING | YES |  |
| side | ENUM | YES |  |
| positionSide | ENUM | NO | Default `BOTH` for One-way Mode ; `LONG` or `SHORT` for Hedge Mode. It must be sent in Hedge Mode. |
| type | ENUM | YES | For `CONDITIONAL` algoType, `STOP_MARKET`/`TAKE_PROFIT_MARKET`/`STOP`/`TAKE_PROFIT`/`TRAILING_STOP_MARKET` as order type |
| timeInForce | ENUM | NO | `IOC` or `GTC` or `FOK` or `GTX` , default `GTC` |
| quantity | DECIMAL | NO | Cannot be sent with `closePosition`=`true`(Close-All) |
| price | DECIMAL | NO |  |
| triggerPrice | DECIMAL | NO |  |
| workingType | ENUM | NO | triggerPrice triggered by: `MARK_PRICE`, `CONTRACT_PRICE`. Default `CONTRACT_PRICE` |
| priceMatch | ENUM | NO | only avaliable for `LIMIT`/`STOP`/`TAKE_PROFIT` order; can be set to `OPPONENT`/ `OPPONENT_5`/ `OPPONENT_10`/ `OPPONENT_20`: /`QUEUE`/ `QUEUE_5`/ `QUEUE_10`/ `QUEUE_20`; Can't be passed together with `price` |
| closePosition | STRING | NO | true, false；Close-All，used with `STOP_MARKET` or `TAKE_PROFIT_MARKET`. |
| priceProtect | STRING | NO | "TRUE" or "FALSE", default "FALSE". Used with `STOP_MARKET` or `TAKE_PROFIT_MARKET` order. when price reaches the triggerPrice ，the difference rate between "MARK_PRICE" and "CONTRACT_PRICE" cannot be larger than the Price Protection Threshold of the symbol. |
| reduceOnly | STRING | NO | "true" or "false". default "false". Cannot be sent in Hedge Mode; cannot be sent with `closePosition`=`true` |
| activatePrice | DECIMAL | NO | Used with `TRAILING_STOP_MARKET` orders, default as the latest price(supporting different `workingType`) |
| callbackRate | DECIMAL | NO | Used with `TRAILING_STOP_MARKET` orders, min 0.1, max 10 where 1 for 1% |
| clientAlgoId | STRING | NO | A unique id among open orders. Automatically generated if not sent. Can only be string following the rule: `^[\.A-Z\:/a-z0-9_-]{1,36}$` |
| newOrderRespType | ENUM | NO | "ACK", "RESULT", default "ACK" |
| selfTradePreventionMode | ENUM | NO | `EXPIRE_TAKER`:expire taker order when STP triggers/ `EXPIRE_MAKER`:expire taker order when STP triggers/ `EXPIRE_BOTH`:expire both orders when STP triggers; default `NONE` |
| goodTillDate | LONG | NO | order cancel time for timeInForce `GTD`, mandatory when `timeInforce` set to `GTD`; order the timestamp only retains second-level precision, ms part will be ignored; The goodTillDate timestamp must be greater than the current time plus 600 seconds and smaller than 253402300799000 |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Algo order with type `STOP`, parameter `timeInForce` can be sent ( default `GTC`). Algo order with type `TAKE_PROFIT`, parameter `timeInForce` can be sent ( default `GTC`).

> Condition orders will be triggered when: If parameter`priceProtect`is sent as true: when price reaches the `triggerPrice` ，the difference rate between "MARK_PRICE" and "CONTRACT_PRICE" cannot be larger than the "triggerProtect" of the symbol "triggerProtect" of a symbol can be got from `GET /fapi/v1/exchangeInfo` `STOP`, `STOP_MARKET`: BUY: latest price ("MARK_PRICE" or "CONTRACT_PRICE") >= `triggerPrice` SELL: latest price ("MARK_PRICE" or "CONTRACT_PRICE") <= `triggerPrice` `TAKE_PROFIT`, `TAKE_PROFIT_MARKET`: BUY: latest price ("MARK_PRICE" or "CONTRACT_PRICE") <= `triggerPrice` SELL: latest price ("MARK_PRICE" or "CONTRACT_PRICE") >= `triggerPrice` `TRAILING_STOP_MARKET`: BUY: the lowest price after order placed <= `activatePrice`, and the latest price >= the lowest price * (1 + `callbackRate`) SELL: the highest price after order placed >= `activatePrice`, and the latest price <= the highest price * (1 - `callbackRate`) For `TRAILING_STOP_MARKET`, if you got such error code. `{"code": -2021, "msg": "Order would immediately trigger."}` means that the parameters you send do not meet the following requirements: BUY: `activatePrice` should be smaller than latest price. SELL: `activatePrice` should be larger than latest price. `STOP_MARKET`, `TAKE_PROFIT_MARKET` with `closePosition`=`true`: Follow the same rules for condition orders. If triggered，close all current long position( if `SELL`) or current short position( if `BUY`). Cannot be used with `quantity` paremeter Cannot be used with `reduceOnly` parameter In Hedge Mode,cannot be used with `BUY` orders in `LONG` position side. and cannot be used with `SELL` orders in `SHORT` position side `selfTradePreventionMode` is only effective when `timeInForce` set to `IOC` or `GTC` or `GTD`.

## Response Example

```javascript
{
"algoId": 2146760,
"clientAlgoId": "6B2I9XVcJpCjqPAJ4YoFX7",
"algoType": "CONDITIONAL",
"orderType": "TAKE_PROFIT",
"symbol": "BNBUSDT",
"side": "SELL",
"positionSide": "BOTH",
"timeInForce": "GTC",
"quantity": "0.01",
"algoStatus": "NEW",
"triggerPrice": "750.000",
"price": "750.000",
"icebergQuantity": null,
"selfTradePreventionMode": "EXPIRE_MAKER",
"workingType": "CONTRACT_PRICE",
"priceMatch": "NONE",
"closePosition": false,
"priceProtect": false,
"reduceOnly": false,
"activatePrice": "", //TRAILING_STOP_MARKET order
"callbackRate": "", //TRAILING_STOP_MARKET order
"createTime": 1750485492076,
"updateTime": 1750485492076,
"triggerTime": 0,
"goodTillDate": 0
}
```
