# Contract Info Stream

- **protocol**: websocket_stream
- **category**: websocket-market-streams
- **meta_description**: Stream Description

## API Summary

- **description**: ContractInfo stream pushes when contract info updates(listing/settlement/contract bracket update). `bks` field only shows up when bracket gets updated.
- **url_path**: `/market`
- **stream_name**: `!contractInfo`
- **update_speed**: Real-time

### Response Examples

```javascript
{
"e":"contractInfo", // Event Type
"E":1669356423908, // Event Time
"s":"IOTAUSDT", // Symbol
"ps":"IOTAUSDT", // Pair
"ct":"PERPETUAL", // Contract type
"dt":4133404800000, // Delivery date time
"ot":1569398400000, // onboard date time
"cs":"TRADING", // Contract status
"bks":[
{
"bs":1, // Notional bracket
"bnf":0, // Floor notional of this bracket
"bnc":5000, // Cap notional of this bracket
"mmr":0.01, // Maintenance ratio for this bracket
"cf":0, // Auxiliary number for quick calculation
"mi":21, // Min leverage for this bracket
"ma":50 // Max leverage for this bracket
},
{
"bs":2,
"bnf":5000,
"bnc":25000,
"mmr":0.025,
"cf":75,
"mi":11,
"ma":20
}
]
}
```

## Stream Description

ContractInfo stream pushes when contract info updates(listing/settlement/contract bracket update). `bks` field only shows up when bracket gets updated.

## URL PATH

`/market`

## Stream Name

`!contractInfo`

## Update Speed

Real-time

## Response Example

```javascript
{
"e":"contractInfo", // Event Type
"E":1669356423908, // Event Time
"s":"IOTAUSDT", // Symbol
"ps":"IOTAUSDT", // Pair
"ct":"PERPETUAL", // Contract type
"dt":4133404800000, // Delivery date time
"ot":1569398400000, // onboard date time
"cs":"TRADING", // Contract status
"bks":[
{
"bs":1, // Notional bracket
"bnf":0, // Floor notional of this bracket
"bnc":5000, // Cap notional of this bracket
"mmr":0.01, // Maintenance ratio for this bracket
"cf":0, // Auxiliary number for quick calculation
"mi":21, // Min leverage for this bracket
"ma":50 // Max leverage for this bracket
},
{
"bs":2,
"bnf":5000,
"bnc":25000,
"mmr":0.025,
"cf":75,
"mi":11,
"ma":20
}
]
}
```
