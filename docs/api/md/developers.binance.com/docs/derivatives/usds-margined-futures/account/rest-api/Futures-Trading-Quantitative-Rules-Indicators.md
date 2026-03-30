# Futures Trading Quantitative Rules Indicators (USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Futures trading quantitative rules indicators, for more information on this, please refer to the Futures Trading Quantitative Rules
- **http_method**: GET
- **path**: `/fapi/v1/apiTradingStatus`
- **http_request**: GET `/fapi/v1/apiTradingStatus`
- **request_weight**: None

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"indicators": { // indicator: quantitative rules indicators, value: user's indicators value, triggerValue: trigger indicator value threshold of quantitative rules.
"BTCUSDT": [
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "UFR", // Unfilled Ratio (UFR)
"value": 0.05, // Current value
"triggerValue": 0.995 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "IFER", // IOC/FOK Expiration Ratio (IFER)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "GCR", // GTC Cancellation Ratio (GCR)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "DR", // Dust Ratio (DR)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
}
],
"ETHUSDT": [
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "UFR",
"value": 0.05,
"triggerValue": 0.995
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "IFER",
"value": 0.99,
"triggerValue": 0.99
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "GCR",
"value": 0.99,
"triggerValue": 0.99
}
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "DR",
"value": 0.99,
"triggerValue": 0.99
}
]
},
"updateTime": 1545741270000
}
```

```javascript
{
"indicators":{
"ACCOUNT":[
{
"indicator":"TMV", // Too many violations under multiple symbols trigger account violation
"value":10,
"triggerValue":1,
"plannedRecoverTime":1644919865000,
"isLocked":true
}
]
},
"updateTime":1644913304748
}
```

## API Description

Futures trading quantitative rules indicators, for more information on this, please refer to the Futures Trading Quantitative Rules

## HTTP Request

GET `/fapi/v1/apiTradingStatus`

## Request Weight

- 1 for a single symbol
- 10 when the symbol parameter is omitted

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

> Response:

```javascript
{
"indicators": { // indicator: quantitative rules indicators, value: user's indicators value, triggerValue: trigger indicator value threshold of quantitative rules.
"BTCUSDT": [
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "UFR", // Unfilled Ratio (UFR)
"value": 0.05, // Current value
"triggerValue": 0.995 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "IFER", // IOC/FOK Expiration Ratio (IFER)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "GCR", // GTC Cancellation Ratio (GCR)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "DR", // Dust Ratio (DR)
"value": 0.99, // Current value
"triggerValue": 0.99 // Trigger value
}
],
"ETHUSDT": [
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "UFR",
"value": 0.05,
"triggerValue": 0.995
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "IFER",
"value": 0.99,
"triggerValue": 0.99
},
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "GCR",
"value": 0.99,
"triggerValue": 0.99
}
{
"isLocked": true,
"plannedRecoverTime": 1545741270000,
"indicator": "DR",
"value": 0.99,
"triggerValue": 0.99
}
]
},
"updateTime": 1545741270000
}
```

> Or (account violation triggered)

```javascript
{
"indicators":{
"ACCOUNT":[
{
"indicator":"TMV", // Too many violations under multiple symbols trigger account violation
"value":10,
"triggerValue":1,
"plannedRecoverTime":1644919865000,
"isLocked":true
}
]
},
"updateTime":1644913304748
}
```
