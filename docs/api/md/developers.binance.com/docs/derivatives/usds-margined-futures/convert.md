# List All Convert Pairs

- **protocol**: reference
- **category**: usds-margined-futures
- **meta_description**: API Description

## API Summary

- **description**: Query for all convertible token pairs and the tokens’ respective upper/lower limits
- **http_method**: GET
- **path**: `/fapi/v1/convert/exchangeInfo`
- **http_request**: GET `/fapi/v1/convert/exchangeInfo`
- **request_weight**: 20(IP)

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| fromAsset | STRING | EITHER OR BOTH | User spends coin |
| toAsset | STRING | EITHER OR BOTH | User receives coin |

### Request Parameter Notes

- User needs to supply either or both of the input parameter If not defined for both fromAsset and toAsset, only partial token pairs will be returned Asset BNFCR is only available to convert for MICA region users.

### Response Examples

```json
[
  {
    "fromAsset": "BTC",
    "toAsset": "USDT",
    "fromAssetMinAmount": "0.0004",
    "fromAssetMaxAmount": "50",
    "toAssetMinAmount": "20",
    "toAssetMaxAmount": "2500000"
  }
]
```

## API Description

Query for all convertible token pairs and the tokens’ respective upper/lower limits

## HTTP Request

GET `/fapi/v1/convert/exchangeInfo`

## Request Weight

20(IP)

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| fromAsset | STRING | EITHER OR BOTH | User spends coin |
| toAsset | STRING | EITHER OR BOTH | User receives coin |

> User needs to supply either or both of the input parameter If not defined for both fromAsset and toAsset, only partial token pairs will be returned Asset BNFCR is only available to convert for MICA region users.

## Response Example

```json
[
  {
    "fromAsset": "BTC",
    "toAsset": "USDT",
    "fromAssetMinAmount": "0.0004",
    "fromAssetMaxAmount": "50",
    "toAssetMinAmount": "20",
    "toAssetMaxAmount": "2500000"
  }
]
```
