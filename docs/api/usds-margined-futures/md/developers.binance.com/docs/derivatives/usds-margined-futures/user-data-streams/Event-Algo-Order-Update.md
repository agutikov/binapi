# Event: Algo Order Update

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: Event Description

## API Summary

- **description**: When new algo order created, order status changed will push such event. event type is `ALGO_UPDATE`.
- **url_path**: `/private`
- **event_name**: `ALGO_UPDATE`

### Response Examples

```javascript
{
"e":"ALGO_UPDATE", // Event Type
"T":1750515742297, // Transaction Time
"E":1750515742303, // Event Time
"o":{
"caid":"Q5xaq5EGKgXXa0fD7fs0Ip", // Client Algo Id
"aid":2148719, // Algo Id
"at":"CONDITIONAL", // Algo Type
"o":"TAKE_PROFIT", //Order Type
"s":"BNBUSDT", //Symbol
"S":"SELL", //Side
"ps":"BOTH", //Position Side
"f":"GTC", //Time in force
"q":"0.01", //quantity
"X":"CANCELED", //Algo status
"ai":"", // order id
"ap": "0.00000", // avg fill price in matching engine, only display when order is triggered and placed in matching engine
"aq": "0.00000", // execuated quantity in matching engine, only display when order is triggered and placed in matching engine
"act": "0", // actual order type in matching engine, only display when order is triggered and placed in matching engine
"tp":"750", //Trigger price
"p":"750", //Order Price
"V":"EXPIRE_MAKER", //STP mode
"wt":"CONTRACT_PRICE", //Working type
"pm":"NONE", // Price match mode
"cp":false, //If Close-All
"pP":false, //If price protection is turned on
"R":false, // Is this reduce only
"tt":0, //Trigger time
"gtd":0, // good till time for GTD time in force
"rm": "Reduce Only reject" // algo order failed reason
}
}
```

## Event Description

When new algo order created, order status changed will push such event. event type is `ALGO_UPDATE`.

Algo Status

- `NEW`: This status indicates that the conditional order was successfully placed into the Algo Service but has not yet been triggered.
- `CANCELED`: This status signifies that the conditional order has been canceled.
- `TRIGGERING`: This status suggests that the order has met the triggering condition and has been forwarded to the matching engine.
- `TRIGGERED`: This status means that the order has been successfully placed into the matching engine.
- `FINISHED`: This status shows that the triggered conditional order has been filled or canceled in the matching engine.
- `REJECTED`: This status signifies that the conditional order has been denied by the matching engine, such as in scenarios of margin check failures.
- `EXPIRED`: This status denotes that the conditional order has been canceled by the system. An example would be when a user places a GTE_GTC Time-In-Force conditional order but then closes all positions on that symbol, resulting in system-led cancellation of the conditional order.

## URL PATH

`/private`

## Event Name

`ALGO_UPDATE`

## Response Example

```javascript
{
"e":"ALGO_UPDATE", // Event Type
"T":1750515742297, // Transaction Time
"E":1750515742303, // Event Time
"o":{
"caid":"Q5xaq5EGKgXXa0fD7fs0Ip", // Client Algo Id
"aid":2148719, // Algo Id
"at":"CONDITIONAL", // Algo Type
"o":"TAKE_PROFIT", //Order Type
"s":"BNBUSDT", //Symbol
"S":"SELL", //Side
"ps":"BOTH", //Position Side
"f":"GTC", //Time in force
"q":"0.01", //quantity
"X":"CANCELED", //Algo status
"ai":"", // order id
"ap": "0.00000", // avg fill price in matching engine, only display when order is triggered and placed in matching engine
"aq": "0.00000", // execuated quantity in matching engine, only display when order is triggered and placed in matching engine
"act": "0", // actual order type in matching engine, only display when order is triggered and placed in matching engine
"tp":"750", //Trigger price
"p":"750", //Order Price
"V":"EXPIRE_MAKER", //STP mode
"wt":"CONTRACT_PRICE", //Working type
"pm":"NONE", // Price match mode
"cp":false, //If Close-All
"pP":false, //If price protection is turned on
"R":false, // Is this reduce only
"tt":0, //Trigger time
"gtd":0, // good till time for GTD time in force
"rm": "Reduce Only reject" // algo order failed reason
}
}
```
