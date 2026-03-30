# Composite Index Symbol Information

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query composite index symbol information
- **http_method**: GET
- **path**: `/fapi/v1/indexInfo`
- **http_request**: GET `/fapi/v1/indexInfo`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Request Parameter Notes

- Only for composite index symbols

### Response Examples

```javascript
[
{
"symbol": "DEFIUSDT",
"time": 1589437530011, // Current time
"component": "baseAsset", //Component asset
"baseAssetList":[
{
"baseAsset":"BAL",
"quoteAsset": "USDT",
"weightInQuantity":"1.04406228",
"weightInPercentage":"0.02783900"
},
{
"baseAsset":"BAND",
"quoteAsset": "USDT",
"weightInQuantity":"3.53782729",
"weightInPercentage":"0.03935200"
}
]
}
]
```

## API Description

Query composite index symbol information

## HTTP Request

GET `/fapi/v1/indexInfo`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

> Only for composite index symbols

## Response Example

```javascript
[
{
"symbol": "DEFIUSDT",
"time": 1589437530011, // Current time
"component": "baseAsset", //Component asset
"baseAssetList":[
{
"baseAsset":"BAL",
"quoteAsset": "USDT",
"weightInQuantity":"1.04406228",
"weightInPercentage":"0.02783900"
},
{
"baseAsset":"BAND",
"quoteAsset": "USDT",
"weightInQuantity":"3.53782729",
"weightInPercentage":"0.03935200"
}
]
}
]
```
