# Basis

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query future basis
- **http_method**: GET
- **path**: `/futures/data/basis`
- **http_request**: GET `/futures/data/basis`
- **request_weight**: 0

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| pair | STRING | YES | BTCUSDT |
| contractType | ENUM | YES | CURRENT_QUARTER, NEXT_QUARTER, PERPETUAL |
| period | ENUM | YES | "5m","15m","30m","1h","2h","4h","6h","12h","1d" |
| limit | LONG | YES | Default 30,Max 500 |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |

### Request Parameter Notes

- If startTime and endTime are not sent, the most recent data is returned. Only the data of the latest 30 days is available.

### Response Examples

```json
[
  {
    "indexPrice": "34400.15945055",
    "contractType": "PERPETUAL",
    "basisRate": "0.0004",
    "futuresPrice": "34414.10",
    "annualizedBasisRate": "",
    "basis": "13.94054945",
    "pair": "BTCUSDT",
    "timestamp": 1698742800000
  }
]
```

## API Description

Query future basis

## HTTP Request

GET `/futures/data/basis`

## Request Weight

0

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| pair | STRING | YES | BTCUSDT |
| contractType | ENUM | YES | CURRENT_QUARTER, NEXT_QUARTER, PERPETUAL |
| period | ENUM | YES | "5m","15m","30m","1h","2h","4h","6h","12h","1d" |
| limit | LONG | YES | Default 30,Max 500 |
| startTime | LONG | NO |  |
| endTime | LONG | NO |  |

> If startTime and endTime are not sent, the most recent data is returned. Only the data of the latest 30 days is available.

## Response Example

```json
[
  {
    "indexPrice": "34400.15945055",
    "contractType": "PERPETUAL",
    "basisRate": "0.0004",
    "futuresPrice": "34414.10",
    "annualizedBasisRate": "",
    "basis": "13.94054945",
    "pair": "BTCUSDT",
    "timestamp": 1698742800000
  }
]
```
