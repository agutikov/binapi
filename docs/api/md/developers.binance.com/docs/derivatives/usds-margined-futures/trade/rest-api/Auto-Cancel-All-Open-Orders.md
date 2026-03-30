# Auto-Cancel All Open Orders (TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Cancel all open orders of the specified symbol at the end of the specified countdown. The endpoint should be called repeatedly as heartbeats so that the existing countdown time can be canceled and replaced by a new one.
- **http_method**: POST
- **path**: `/fapi/v1/countdownCancelAll`
- **http_request**: POST `/fapi/v1/countdownCancelAll`

### Response Examples

```json
{
  "symbol": "BTCUSDT",
  "countdownTime": "100000"
}
```

## API Description

Cancel all open orders of the specified symbol at the end of the specified countdown. The endpoint should be called repeatedly as heartbeats so that the existing countdown time can be canceled and replaced by a new one.

> Example usage: Call this endpoint at 30s intervals with an countdownTime of 120000 (120s). If this endpoint is not called within 120 seconds, all your orders of the specified symbol will be automatically canceled. If this endpoint is called with an countdownTime of 0, the countdown timer will be stopped.

The system will check all countdowns approximately every 10 milliseconds, so please note that sufficient redundancy should be considered when using this function. We do not recommend setting the countdown time to be too precise or too small.

## HTTP Request

POST `/fapi/v1/countdownCancelAll`

Weight: 10

Parameters:

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| countdownTime | LONG | YES | countdown time, 1000 for 1 second. 0 to cancel the timer |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```json
{
  "symbol": "BTCUSDT",
  "countdownTime": "100000"
}
```
