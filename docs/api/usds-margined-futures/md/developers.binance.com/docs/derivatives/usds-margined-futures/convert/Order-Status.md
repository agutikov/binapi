# Order status(USER_DATA)

- **protocol**: reference
- **category**: convert
- **meta_description**: API Description

## API Summary

- **description**: Query order status by order ID.
- **http_method**: GET
- **path**: `/fapi/v1/convert/orderStatus`
- **http_request**: GET `/fapi/v1/convert/orderStatus`
- **request_weight**: 50(IP)

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| orderId | STRING | NO | Either orderId or quoteId is required |
| quoteId | STRING | NO | Either orderId or quoteId is required |

### Response Examples

```json
{
  "orderId": 933256278426274426,
  "orderStatus": "SUCCESS",
  "fromAsset": "BTC",
  "fromAmount": "0.00054414",
  "toAsset": "USDT",
  "toAmount": "20",
  "ratio": "36755",
  "inverseRatio": "0.00002721",
  "createTime": 1623381330472
}
```

## API Description

Query order status by order ID.

## HTTP Request

GET `/fapi/v1/convert/orderStatus`

## Request Weight

50(IP)

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| orderId | STRING | NO | Either orderId or quoteId is required |
| quoteId | STRING | NO | Either orderId or quoteId is required |

## Response Example

```json
{
  "orderId": 933256278426274426,
  "orderStatus": "SUCCESS",
  "fromAsset": "BTC",
  "fromAmount": "0.00054414",
  "toAsset": "USDT",
  "toAmount": "20",
  "ratio": "36755",
  "inverseRatio": "0.00002721",
  "createTime": 1623381330472
}
```
