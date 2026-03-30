# Long/Short Ratio

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query symbol Long/Short Ratio
- **http_method**: GET
- **path**: `/futures/data/globalLongShortAccountRatio`
- **http_request**: GET `/futures/data/globalLongShortAccountRatio`
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
"symbol":"BTCUSDT", // long/short account num ratio of all traders
"longShortRatio":"0.1960", //long account num ratio of all traders
"longAccount": "0.6622", // short account num ratio of all traders
"shortAccount":"0.3378",
"timestamp":"1583139600000"

},

{

"symbol":"BTCUSDT",
"longShortRatio":"1.9559",
"longAccount": "0.6617",
"shortAccount":"0.3382",
"timestamp":"1583139900000"

},

]
```

## API Description

Query symbol Long/Short Ratio

## HTTP Request

GET `/futures/data/globalLongShortAccountRatio`

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
"symbol":"BTCUSDT", // long/short account num ratio of all traders
"longShortRatio":"0.1960", //long account num ratio of all traders
"longAccount": "0.6622", // short account num ratio of all traders
"shortAccount":"0.3378",
"timestamp":"1583139600000"

},

{

"symbol":"BTCUSDT",
"longShortRatio":"1.9559",
"longAccount": "0.6617",
"shortAccount":"0.3382",
"timestamp":"1583139900000"

},

]
```
