# Query Insurance Fund Balance Snapshot

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query Insurance Fund Balance Snapshot
- **http_method**: GET
- **path**: `/fapi/v1/insuranceBalance`
- **http_request**: GET `/fapi/v1/insuranceBalance`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

### Response Examples

```json
{
  "symbols": [
    "BNBUSDT",
    "BTCUSDT",
    "BTCUSDT_250627",
    "BTCUSDT_250926",
    "ETHBTC",
    "ETHUSDT",
    "ETHUSDT_250627",
    "ETHUSDT_250926"
  ],
  "assets": [
    {
      "asset": "USDC",
      "marginBalance": "299999998.6497832",
      "updateTime": 1745366402000
    },
    {
      "asset": "USDT",
      "marginBalance": "793930579.315848",
      "updateTime": 1745366402000
    },
    {
      "asset": "BTC",
      "marginBalance": "61.73143554",
      "updateTime": 1745366402000
    },
    {
      "asset": "BNFCR",
      "marginBalance": "633223.99396922",
      "updateTime": 1745366402000
    }
  ]
}
```

```json
[
  {
    "symbols": [
      "ADAUSDT",
      "BCHUSDT",
      "DOTUSDT",
      "EOSUSDT",
      "ETCUSDT",
      "LINKUSDT",
      "LTCUSDT",
      "TRXUSDT",
      "XLMUSDT",
      "XMRUSDT",
      "XRPUSDT"
    ],
    "assets": [
      {
        "asset": "USDT",
        "marginBalance": "314151411.06482935",
        "updateTime": 1745366402000
      }
    ]
  },
  {
    "symbols": [
      "ACTUSDT",
      "MUBARAKUSDT",
      "OMUSDT",
      "TSTUSDT"
    ],
    "assets": [
      {
        "asset": "USDT",
        "marginBalance": "5166686.84431694",
        "updateTime": 1745366402000
      }
    ]
  }
]
```

## API Description

Query Insurance Fund Balance Snapshot

## HTTP Request

GET `/fapi/v1/insuranceBalance`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | NO |  |

## Response Example

pass symbol

```json
{
  "symbols": [
    "BNBUSDT",
    "BTCUSDT",
    "BTCUSDT_250627",
    "BTCUSDT_250926",
    "ETHBTC",
    "ETHUSDT",
    "ETHUSDT_250627",
    "ETHUSDT_250926"
  ],
  "assets": [
    {
      "asset": "USDC",
      "marginBalance": "299999998.6497832",
      "updateTime": 1745366402000
    },
    {
      "asset": "USDT",
      "marginBalance": "793930579.315848",
      "updateTime": 1745366402000
    },
    {
      "asset": "BTC",
      "marginBalance": "61.73143554",
      "updateTime": 1745366402000
    },
    {
      "asset": "BNFCR",
      "marginBalance": "633223.99396922",
      "updateTime": 1745366402000
    }
  ]
}
```

> or not pass symbol

```json
[
  {
    "symbols": [
      "ADAUSDT",
      "BCHUSDT",
      "DOTUSDT",
      "EOSUSDT",
      "ETCUSDT",
      "LINKUSDT",
      "LTCUSDT",
      "TRXUSDT",
      "XLMUSDT",
      "XMRUSDT",
      "XRPUSDT"
    ],
    "assets": [
      {
        "asset": "USDT",
        "marginBalance": "314151411.06482935",
        "updateTime": 1745366402000
      }
    ]
  },
  {
    "symbols": [
      "ACTUSDT",
      "MUBARAKUSDT",
      "OMUSDT",
      "TSTUSDT"
    ],
    "assets": [
      {
        "asset": "USDT",
        "marginBalance": "5166686.84431694",
        "updateTime": 1745366402000
      }
    ]
  }
]
```
