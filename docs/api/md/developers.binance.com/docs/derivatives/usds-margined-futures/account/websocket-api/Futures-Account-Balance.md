# Futures Account Balance(USER_DATA)

- **protocol**: websocket_api
- **category**: websocket-api
- **meta_description**: API Description

## API Summary

- **description**: Query account balance info
- **method**: `account.balance`
- **request_weight**: 5

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Examples

```json
{
  "id": "605a6d20-6588-4cb9-afa0-b0ab087507ba",
  "method": "account.balance",
  "params": {
    "apiKey": "xTaDyrmvA9XT2oBHHjy39zyPzKCvMdtH3b9q4xadkAg2dNSJXQGCxzui26L823W2",
    "timestamp": 1702561978458,
    "signature": "208bb94a26f99aa122b1319490ca9cb2798fccc81d9b6449521a26268d53217a"
  }
}
```

### Response Examples

```javascript
{
"id": "605a6d20-6588-4cb9-afa0-b0ab087507ba",
"status": 200,
"result": [
{
"accountAlias": "SgsR", // unique account code
"asset": "USDT", // asset name
"balance": "122607.35137903", // wallet balance
"crossWalletBalance": "23.72469206", // crossed wallet balance
"crossUnPnl": "0.00000000" // unrealized profit of crossed positions
"availableBalance": "23.72469206", // available balance
"maxWithdrawAmount": "23.72469206", // maximum amount for transfer out
"marginAvailable": true, // whether the asset can be used as margin in Multi-Assets mode
"updateTime": 1617939110373
}
],
"rateLimits": [
{
"rateLimitType": "REQUEST_WEIGHT",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 2400,
"count": 20
}
]
}
```

## API Description

Query account balance info

## Method

`account.balance`

## Request

```json
{
  "id": "605a6d20-6588-4cb9-afa0-b0ab087507ba",
  "method": "account.balance",
  "params": {
    "apiKey": "xTaDyrmvA9XT2oBHHjy39zyPzKCvMdtH3b9q4xadkAg2dNSJXQGCxzui26L823W2",
    "timestamp": 1702561978458,
    "signature": "208bb94a26f99aa122b1319490ca9cb2798fccc81d9b6449521a26268d53217a"
  }
}
```

## Request Weight

5

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

## Response Example

```javascript
{
"id": "605a6d20-6588-4cb9-afa0-b0ab087507ba",
"status": 200,
"result": [
{
"accountAlias": "SgsR", // unique account code
"asset": "USDT", // asset name
"balance": "122607.35137903", // wallet balance
"crossWalletBalance": "23.72469206", // crossed wallet balance
"crossUnPnl": "0.00000000" // unrealized profit of crossed positions
"availableBalance": "23.72469206", // available balance
"maxWithdrawAmount": "23.72469206", // maximum amount for transfer out
"marginAvailable": true, // whether the asset can be used as margin in Multi-Assets mode
"updateTime": 1617939110373
}
],
"rateLimits": [
{
"rateLimitType": "REQUEST_WEIGHT",
"interval": "MINUTE",
"intervalNum": 1,
"limit": 2400,
"count": 20
}
]
}
```
