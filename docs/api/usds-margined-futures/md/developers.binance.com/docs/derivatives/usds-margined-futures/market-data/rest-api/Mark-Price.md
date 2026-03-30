# Mark Price

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Mark Price and Funding Rate
- **http_method**: GET
- **path**: `/fapi/v1/premiumIndex`
- **http_request**: GET `/fapi/v1/premiumIndex`
- **request_weight**: 1 with symbol, 10 without symbol

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Response Examples

```javascript
{
"symbol": "BTCUSDT",
"markPrice": "11793.63104562", // mark price
"indexPrice": "11781.80495970", // index price
"estimatedSettlePrice": "11781.16138815", // Estimated Settle Price, only useful in the last hour before the settlement starts.
"lastFundingRate": "0.00038246", // This is the Latest funding rate
"interestRate": "0.00010000",
"nextFundingTime": 1597392000000,
"time": 1597370495002
}
```

```javascript
[
{
"symbol": "BTCUSDT",
"markPrice": "11793.63104562", // mark price
"indexPrice": "11781.80495970", // index price
"estimatedSettlePrice": "11781.16138815", // Estimated Settle Price, only useful in the last hour before the settlement starts.
"lastFundingRate": "0.00038246", // This is the Latest funding rate
"interestRate": "0.00010000",
"nextFundingTime": 1597392000000,
"time": 1597370495002
}
]
```

## API Description

Mark Price and Funding Rate

## HTTP Request

GET `/fapi/v1/premiumIndex`

## Request Weight

1 with symbol, 10 without symbol

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

## Response Example

> Response:

```javascript
{
"symbol": "BTCUSDT",
"markPrice": "11793.63104562", // mark price
"indexPrice": "11781.80495970", // index price
"estimatedSettlePrice": "11781.16138815", // Estimated Settle Price, only useful in the last hour before the settlement starts.
"lastFundingRate": "0.00038246", // This is the Latest funding rate
"interestRate": "0.00010000",
"nextFundingTime": 1597392000000,
"time": 1597370495002
}
```

> OR (when symbol not sent)

```javascript
[
{
"symbol": "BTCUSDT",
"markPrice": "11793.63104562", // mark price
"indexPrice": "11781.80495970", // index price
"estimatedSettlePrice": "11781.16138815", // Estimated Settle Price, only useful in the last hour before the settlement starts.
"lastFundingRate": "0.00038246", // This is the Latest funding rate
"interestRate": "0.00010000",
"nextFundingTime": 1597392000000,
"time": 1597370495002
}
]
```
