# Event: GRID_UPDATE

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: Event Description

## API Summary

- **description**: `GRID_UPDATE` update when a sub order of a grid is filled or partially filled. Strategy Status
- **url_path**: `/private`
- **event_name**: `GRID_UPDATE`

### Response Examples

```javascript
{
"e": "GRID_UPDATE", // Event Type
"T": 1669262908216, // Transaction Time
"E": 1669262908218, // Event Time
"gu": {
"si": 176057039, // Strategy ID
"st": "GRID", // Strategy Type
"ss": "WORKING", // Strategy Status
"s": "BTCUSDT", // Symbol
"r": "-0.00300716", // Realized PNL
"up": "16720", // Unmatched Average Price
"uq": "-0.001", // Unmatched Qty
"uf": "-0.00300716", // Unmatched Fee
"mp": "0.0", // Matched PNL
"ut": 1669262908197 // Update Time
}
}
```

## Event Description

`GRID_UPDATE` update when a sub order of a grid is filled or partially filled. Strategy Status

- NEW
- WORKING
- CANCELLED
- EXPIRED

## URL PATH

`/private`

## Event Name

`GRID_UPDATE`

## Response Example

```javascript
{
"e": "GRID_UPDATE", // Event Type
"T": 1669262908216, // Transaction Time
"E": 1669262908218, // Event Time
"gu": {
"si": 176057039, // Strategy ID
"st": "GRID", // Strategy Type
"ss": "WORKING", // Strategy Status
"s": "BTCUSDT", // Symbol
"r": "-0.00300716", // Realized PNL
"up": "16720", // Unmatched Average Price
"uq": "-0.001", // Unmatched Qty
"uf": "-0.00300716", // Unmatched Fee
"mp": "0.0", // Matched PNL
"ut": 1669262908197 // Update Time
}
}
```
