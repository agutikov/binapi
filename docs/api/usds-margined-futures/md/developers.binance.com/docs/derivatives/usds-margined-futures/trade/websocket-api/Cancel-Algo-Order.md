# Cancel Algo Order (TRADE)

- **protocol**: websocket_api
- **category**: websocket-api
- **meta_description**: API Description

## API Summary

- **description**: Cancel an active algo order.
- **method**: `algoOrder.cancel`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoId | LONG | NO |  |
| clientAlgoId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Either `algoId` or `clientAlgoId` must be sent.

### Request Examples

```json
{
  "id": "5633b6a2-90a9-4192-83e7-925c90b6a2fd",
  "method": "algoOrder.cancel",
  "params": {
    "apiKey": "HsOehcfih8ZRxnhjp2XjGXhsOBd6msAhKz9joQaWwZ7arcJTlD2hGOGQj1lGdTjR",
    "algoId": 283194212,
    "clientAlgoId": "DolwRKnQNjoc1E9Bbh03ER",
    "timestamp": 1703439070722,
    "signature": "b09c49815b4e3f1f6098cd9fbe26a933a9af79803deaaaae03c29f719c08a8a8"
  }
}
```

### Response Examples

```json
{
  "id": "unique-cancel-request-id-5678",
  "status": 200,
  "result": {
    "algoId": 2000000002162519,
    "clientAlgoId": "rDMG8WSde6LkyMNtk6s825",
    "code": "200",
    "msg": "success"
  },
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 6
    }
  ]
}
```

## API Description

Cancel an active algo order.

## Method

`algoOrder.cancel`

## Request

```json
{
  "id": "5633b6a2-90a9-4192-83e7-925c90b6a2fd",
  "method": "algoOrder.cancel",
  "params": {
    "apiKey": "HsOehcfih8ZRxnhjp2XjGXhsOBd6msAhKz9joQaWwZ7arcJTlD2hGOGQj1lGdTjR",
    "algoId": 283194212,
    "clientAlgoId": "DolwRKnQNjoc1E9Bbh03ER",
    "timestamp": 1703439070722,
    "signature": "b09c49815b4e3f1f6098cd9fbe26a933a9af79803deaaaae03c29f719c08a8a8"
  }
}
```

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| algoId | LONG | NO |  |
| clientAlgoId | STRING | NO |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Either `algoId` or `clientAlgoId` must be sent.

## Response Example

```json
{
  "id": "unique-cancel-request-id-5678",
  "status": 200,
  "result": {
    "algoId": 2000000002162519,
    "clientAlgoId": "rDMG8WSde6LkyMNtk6s825",
    "code": "200",
    "msg": "success"
  },
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 6
    }
  ]
}
```
