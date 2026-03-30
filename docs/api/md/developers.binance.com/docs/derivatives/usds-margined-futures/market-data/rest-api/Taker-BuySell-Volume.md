# Taker Buy/Sell Volume

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Taker Buy/Sell Volume
- **http_method**: GET
- **path**: `/futures/data/takerlongshortRatio`
- **http_request**: GET `/futures/data/takerlongshortRatio`
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
"buySellRatio":"1.5586",
"buyVol": "387.3300",
"sellVol":"248.5030",
"timestamp":"1585614900000"
},
{
"buySellRatio":"1.3104",
"buyVol": "343.9290",
"sellVol":"248.5030",
"timestamp":"1583139900000"
},
]
```

## API Description

Taker Buy/Sell Volume

## HTTP Request

GET `/futures/data/takerlongshortRatio`

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
"buySellRatio":"1.5586",
"buyVol": "387.3300",
"sellVol":"248.5030",
"timestamp":"1585614900000"
},
{
"buySellRatio":"1.3104",
"buyVol": "343.9290",
"sellVol":"248.5030",
"timestamp":"1583139900000"
},
]
```
