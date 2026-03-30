# Event: Conditional_Order_Trigger_Reject

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: Event Description

## API Summary

- **description**: `CONDITIONAL_ORDER_TRIGGER_REJECT` update when a triggered TP/SL order got rejected.
- **url_path**: `/private`
- **event_name**: `CONDITIONAL_ORDER_TRIGGER_REJECT`

### Response Examples

```javascript
{
"e":"CONDITIONAL_ORDER_TRIGGER_REJECT", // Event Type
"E":1685517224945, // Event Time
"T":1685517224955, // me message send Time
"or":{
"s":"ETHUSDT", // Symbol
"i":155618472834, // orderId
"r":"Due to the order could not be filled immediately, the FOK order has been rejected. The order will not be recorded in the order history", // reject reason
}
}
```

## Event Description

`CONDITIONAL_ORDER_TRIGGER_REJECT` update when a triggered TP/SL order got rejected.

## URL PATH

`/private`

## Event Name

`CONDITIONAL_ORDER_TRIGGER_REJECT`

## Response Example

```javascript
{
"e":"CONDITIONAL_ORDER_TRIGGER_REJECT", // Event Type
"E":1685517224945, // Event Time
"T":1685517224955, // me message send Time
"or":{
"s":"ETHUSDT", // Symbol
"i":155618472834, // orderId
"r":"Due to the order could not be filled immediately, the FOK order has been rejected. The order will not be recorded in the order history", // reject reason
}
}
```
