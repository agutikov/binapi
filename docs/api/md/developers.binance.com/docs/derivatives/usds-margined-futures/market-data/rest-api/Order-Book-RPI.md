# RPI Order Book

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query symbol orderbook with RPI orders
- **http_method**: GET
- **path**: `/fapi/v1/rpiDepth`
- **http_request**: GET `/fapi/v1/rpiDepth`
- **request_weight**: Adjusted based on the limit:

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 1000; Valid limits:[1000] |

### Response Examples

```javascript
{
"lastUpdateId": 1027024,
"E": 1589436922972, // Message output time
"T": 1589436922959, // Transaction time
"bids": [
[
"4.00000000", // PRICE
"431.00000000" // QTY
]
],
"asks": [
[
"4.00000200",
"12.00000000"
]
]
}
```

## API Description

Query symbol orderbook with RPI orders

## HTTP Request

GET `/fapi/v1/rpiDepth`

Note:

> RPI(Retail Price Improvement) orders are included and aggreated in the response message. Crossed price levels are hidden and invisible.

## Request Weight

Adjusted based on the limit:

| Limit | Weight |
| --- | --- |
| 1000 | 20 |

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 1000; Valid limits:[1000] |

## Response Example

```javascript
{
"lastUpdateId": 1027024,
"E": 1589436922972, // Message output time
"T": 1589436922959, // Transaction time
"bids": [
[
"4.00000000", // PRICE
"431.00000000" // QTY
]
],
"asks": [
[
"4.00000200",
"12.00000000"
]
]
}
```
