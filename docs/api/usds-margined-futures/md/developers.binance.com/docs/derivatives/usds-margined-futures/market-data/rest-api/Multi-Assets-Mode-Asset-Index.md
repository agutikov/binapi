# Multi-Assets Mode Asset Index

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: asset index for Multi-Assets mode
- **http_method**: GET
- **path**: `/fapi/v1/assetIndex`
- **http_request**: GET `/fapi/v1/assetIndex`
- **request_weight**: 1 for a single symbol; 10 when the symbol parameter is omitted

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO | Asset pair |

### Response Examples

```json
{
  "symbol": "ADAUSD",
  "time": 1635740268004,
  "index": "1.92957370",
  "bidBuffer": "0.10000000",
  "askBuffer": "0.10000000",
  "bidRate": "1.73661633",
  "askRate": "2.12253107",
  "autoExchangeBidBuffer": "0.05000000",
  "autoExchangeAskBuffer": "0.05000000",
  "autoExchangeBidRate": "1.83309501",
  "autoExchangeAskRate": "2.02605238"
}
```

```json
[
  {
    "symbol": "ADAUSD",
    "time": 1635740268004,
    "index": "1.92957370",
    "bidBuffer": "0.10000000",
    "askBuffer": "0.10000000",
    "bidRate": "1.73661633",
    "askRate": "2.12253107",
    "autoExchangeBidBuffer": "0.05000000",
    "autoExchangeAskBuffer": "0.05000000",
    "autoExchangeBidRate": "1.83309501",
    "autoExchangeAskRate": "2.02605238"
  }
]
```

## API Description

asset index for Multi-Assets mode

## HTTP Request

GET `/fapi/v1/assetIndex`

## Request Weight

1 for a single symbol; 10 when the symbol parameter is omitted

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO | Asset pair |

## Response Example

> Response:

```json
{
  "symbol": "ADAUSD",
  "time": 1635740268004,
  "index": "1.92957370",
  "bidBuffer": "0.10000000",
  "askBuffer": "0.10000000",
  "bidRate": "1.73661633",
  "askRate": "2.12253107",
  "autoExchangeBidBuffer": "0.05000000",
  "autoExchangeAskBuffer": "0.05000000",
  "autoExchangeBidRate": "1.83309501",
  "autoExchangeAskRate": "2.02605238"
}
```

> Or(without symbol)

```json
[
  {
    "symbol": "ADAUSD",
    "time": 1635740268004,
    "index": "1.92957370",
    "bidBuffer": "0.10000000",
    "askBuffer": "0.10000000",
    "bidRate": "1.73661633",
    "askRate": "2.12253107",
    "autoExchangeBidBuffer": "0.05000000",
    "autoExchangeAskBuffer": "0.05000000",
    "autoExchangeBidRate": "1.83309501",
    "autoExchangeAskRate": "2.02605238"
  }
]
```
