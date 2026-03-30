# Event: Trade Lite Update

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: Event Description

## API Summary

- **description**: Fast trade stream reduces data latency compared original `ORDER_TRADE_UPDATE` stream. However, it only pushes TRADE Execution Type, and fewer data fields.
- **url_path**: `/private`
- **event_name**: `TRADE_LITE`

### Response Examples

```javascript
{
"e":"TRADE_LITE", // Event Type
"E":1721895408092, // Event Time
"T":1721895408214, // Transaction Time
"s":"BTCUSDT", // Symbol
"q":"0.001", // Original Quantity
"p":"0", // Original Price
"m":false, // Is this trade the maker side?
"c":"z8hcUoOsqEdKMeKPSABslD", // Client Order Id
// special client order id:
// starts with "autoclose-": liquidation order
// "adl_autoclose": ADL auto close order
// "settlement_autoclose-": settlement order for delisting or delivery
"S":"BUY", // Side
"L":"64089.20", // Last Filled Price
"l":"0.040", // Order Last Filled Quantity
"t":109100866, // Trade Id
"i":8886774, // Order Id
}
```

## Event Description

Fast trade stream reduces data latency compared original `ORDER_TRADE_UPDATE` stream. However, it only pushes TRADE Execution Type, and fewer data fields.

## URL PATH

`/private`

## Event Name

`TRADE_LITE`

## Response Example

```javascript
{
"e":"TRADE_LITE", // Event Type
"E":1721895408092, // Event Time
"T":1721895408214, // Transaction Time
"s":"BTCUSDT", // Symbol
"q":"0.001", // Original Quantity
"p":"0", // Original Price
"m":false, // Is this trade the maker side?
"c":"z8hcUoOsqEdKMeKPSABslD", // Client Order Id
// special client order id:
// starts with "autoclose-": liquidation order
// "adl_autoclose": ADL auto close order
// "settlement_autoclose-": settlement order for delisting or delivery
"S":"BUY", // Side
"L":"64089.20", // Last Filled Price
"l":"0.040", // Order Last Filled Quantity
"t":109100866, // Trade Id
"i":8886774, // Order Id
}
```
