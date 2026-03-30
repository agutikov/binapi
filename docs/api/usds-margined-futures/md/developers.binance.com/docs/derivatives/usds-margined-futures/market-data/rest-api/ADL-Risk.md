# ADL Risk

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query the symbol-level ADL risk rating. The ADL risk rating measures the likelihood of ADL during liquidation, and the rating takes into account the insurance fund balance, position concentration on the symbol, order book depth, price volatility, average leverage, unrealized PnL, and margin utilization at the symbol level. The rating can be high, medium and low, and is updated every 30 minutes.
- **http_method**: GET
- **path**: `/fapi/v1/symbolAdlRisk`
- **http_request**: GET `/fapi/v1/symbolAdlRisk`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Response Examples

```javascript
{
"symbol": "BTCUSDT",
"adlRisk": "low", // ADL Risk rating
"updateTime": 1597370495002
}
```

```javascript
[
{
"symbol": "BTCUSDT",
"adlRisk": "low", // ADL Risk rating
"updateTime": 1597370495002
},
{
"symbol": "ETHUSDT",
"adlRisk": "high", // ADL Risk rating
"updateTime": 1597370495004
}
]
```

## API Description

Query the symbol-level ADL risk rating. The ADL risk rating measures the likelihood of ADL during liquidation, and the rating takes into account the insurance fund balance, position concentration on the symbol, order book depth, price volatility, average leverage, unrealized PnL, and margin utilization at the symbol level. The rating can be high, medium and low, and is updated every 30 minutes.

## HTTP Request

GET `/fapi/v1/symbolAdlRisk`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

## Response Example

> Response:

```javascript
{
"symbol": "BTCUSDT",
"adlRisk": "low", // ADL Risk rating
"updateTime": 1597370495002
}
```

> OR (when symbol not sent)

```javascript
[
{
"symbol": "BTCUSDT",
"adlRisk": "low", // ADL Risk rating
"updateTime": 1597370495002
},
{
"symbol": "ETHUSDT",
"adlRisk": "high", // ADL Risk rating
"updateTime": 1597370495004
}
]
```
