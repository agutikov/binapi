# Top Trader Long/Short Ratio (Positions)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: The proportion of net long and net short positions to total open positions of the top 20% users with the highest margin balance. Long Position % = Long positions of top traders / Total open positions of top traders Short Position % = Short positions of top traders / Total open positions of top traders Long/Short Ratio (Positions) = Long Position % / Short Position %
- **http_method**: GET
- **path**: `/futures/data/topLongShortPositionRatio`
- **http_request**: GET `/futures/data/topLongShortPositionRatio`
- **request_weight**: 0

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| period | ENUM | YES | "5m","15m","30m","1h","2h","4h","6h","12h","1d" |
| limit | LONG | NO | default 30, max 500 |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |

### Request Parameter Notes

- If startTime and endTime are not sent, the most recent data is returned. Only the data of the latest 30 days is available. IP rate limit 1000 requests/5min

### Response Examples

```javascript
[
{
"symbol":"BTCUSDT",
"longShortRatio":"1.4342",// long/short position ratio of top traders
"longAccount": "0.5891", // long positions ratio of top traders
"shortAccount":"0.4108", // short positions ratio of top traders
"timestamp":"1583139600000"

},

{

"symbol":"BTCUSDT",
"longShortRatio":"1.4337",
"longAccount": "0.3583",
"shortAccount":"0.6417",
"timestamp":"1583139900000"

},

]
```

## API Description

The proportion of net long and net short positions to total open positions of the top 20% users with the highest margin balance. Long Position % = Long positions of top traders / Total open positions of top traders Short Position % = Short positions of top traders / Total open positions of top traders Long/Short Ratio (Positions) = Long Position % / Short Position %

## HTTP Request

GET `/futures/data/topLongShortPositionRatio`

## Request Weight

0

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| period | ENUM | YES | "5m","15m","30m","1h","2h","4h","6h","12h","1d" |
| limit | LONG | NO | default 30, max 500 |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |

> If startTime and endTime are not sent, the most recent data is returned. Only the data of the latest 30 days is available. IP rate limit 1000 requests/5min

## Response Example

```javascript
[
{
"symbol":"BTCUSDT",
"longShortRatio":"1.4342",// long/short position ratio of top traders
"longAccount": "0.5891", // long positions ratio of top traders
"shortAccount":"0.4108", // short positions ratio of top traders
"timestamp":"1583139600000"

},

{

"symbol":"BTCUSDT",
"longShortRatio":"1.4337",
"longAccount": "0.3583",
"shortAccount":"0.6417",
"timestamp":"1583139900000"

},

]
```
