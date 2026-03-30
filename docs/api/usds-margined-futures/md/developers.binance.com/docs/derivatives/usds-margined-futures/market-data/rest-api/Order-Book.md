# Order Book

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query symbol orderbook
- **http_method**: GET
- **path**: `/fapi/v1/depth`
- **http_request**: GET `/fapi/v1/depth`
- **request_weight**: Adjusted based on the limit:

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 500; Valid limits:[5, 10, 20, 50, 100, 500, 1000] |

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

Query symbol orderbook

## HTTP Request

GET `/fapi/v1/depth`

Note:

> Retail Price Improvement(RPI) orders are not visible and excluded in the response message.

## Request Weight

Adjusted based on the limit:

| Limit | Weight |
| --- | --- |
| 5, 10, 20, 50 | 2 |
| 100 | 5 |
| 500 | 10 |
| 1000 | 20 |

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| limit | INT | NO | Default 500; Valid limits:[5, 10, 20, 50, 100, 500, 1000] |

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
