# Trading Schedule

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Trading session schedules for the underlying assets of TradFi Perps are provided for a one-week period starting from the day prior to the query time, covering both the U.S. equity and commodity markets. Equity market session types include "PRE_MARKET", "REGULAR", "AFTER_MARKET", "OVERNIGHT", and "NO_TRADING", while commodity market session types include "REGULAR" and "NO_TRADING".
- **http_method**: GET
- **path**: `/fapi/v1/tradingSchedule`
- **http_request**: GET `/fapi/v1/tradingSchedule`
- **request_weight**: 5

### Request Parameter Notes

- NONE

### Response Examples

```json
{
  "updateTime": 1761286643918,
  "marketSchedules": {
    "EQUITY": {
      "sessions": [
        {
          "startTime": 1761177600000,
          "endTime": 1761206400000,
          "type": "OVERNIGHT"
        },
        {
          "startTime": 1761206400000,
          "endTime": 1761226200000,
          "type": "PRE_MARKET"
        }
      ]
    },
    "COMMODITY": {
      "sessions": [
        {
          "startTime": 1761724800000,
          "endTime": 1761744600000,
          "type": "NO_TRADING"
        },
        {
          "startTime": 1761744600000,
          "endTime": 1761768000000,
          "type": "REGULAR"
        }
      ]
    }
  }
}
```

## API Description

Trading session schedules for the underlying assets of TradFi Perps are provided for a one-week period starting from the day prior to the query time, covering both the U.S. equity and commodity markets. Equity market session types include "PRE_MARKET", "REGULAR", "AFTER_MARKET", "OVERNIGHT", and "NO_TRADING", while commodity market session types include "REGULAR" and "NO_TRADING".

## HTTP Request

GET `/fapi/v1/tradingSchedule`

## Request Weight

5

## Request Parameters

NONE

## Response Example

```json
{
  "updateTime": 1761286643918,
  "marketSchedules": {
    "EQUITY": {
      "sessions": [
        {
          "startTime": 1761177600000,
          "endTime": 1761206400000,
          "type": "OVERNIGHT"
        },
        {
          "startTime": 1761206400000,
          "endTime": 1761226200000,
          "type": "PRE_MARKET"
        }
      ]
    },
    "COMMODITY": {
      "sessions": [
        {
          "startTime": 1761724800000,
          "endTime": 1761744600000,
          "type": "NO_TRADING"
        },
        {
          "startTime": 1761744600000,
          "endTime": 1761768000000,
          "type": "REGULAR"
        }
      ]
    }
  }
}
```
