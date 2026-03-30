# Get Funding Rate Info

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query funding rate info for symbols that had FundingRateCap/ FundingRateFloor / fundingIntervalHours adjustment
- **http_method**: GET
- **path**: `/fapi/v1/fundingInfo`
- **http_request**: GET `/fapi/v1/fundingInfo`
- **request_weight**: 0

### Response Examples

```javascript
[
{
"symbol": "BLZUSDT",
"adjustedFundingRateCap": "0.02500000",
"adjustedFundingRateFloor": "-0.02500000",
"fundingIntervalHours": 8,
"disclaimer": false // ingore
}
]
```

## API Description

Query funding rate info for symbols that had FundingRateCap/ FundingRateFloor / fundingIntervalHours adjustment

## HTTP Request

GET `/fapi/v1/fundingInfo`

## Request Weight

0

share 500/5min/IP rate limit with `GET /fapi/v1/fundingRate`

## Request Parameters

## Response Example

```javascript
[
{
"symbol": "BLZUSDT",
"adjustedFundingRateCap": "0.02500000",
"adjustedFundingRateFloor": "-0.02500000",
"fundingIntervalHours": 8,
"disclaimer": false // ingore
}
]
```
