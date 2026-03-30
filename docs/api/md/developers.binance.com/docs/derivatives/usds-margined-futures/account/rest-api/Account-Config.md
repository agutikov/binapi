# Futures Account Configuration(USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Query account configuration
- **http_method**: GET
- **path**: `/fapi/v1/accountConfig`
- **http_request**: GET `/fapi/v1/accountConfig`
- **request_weight**: 5

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Response Examples

```javascript
{
"feeTier": 0, // account commission tier
"canTrade": true, // if can trade
"canDeposit": true, // if can transfer in asset
"canWithdraw": true, // if can transfer out asset
"dualSidePosition": true,
"updateTime": 0, // reserved property, please ignore
"multiAssetsMargin": false,
"tradeGroupId": -1
}
```

## API Description

Query account configuration

## HTTP Request

GET `/fapi/v1/accountConfig`

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
"feeTier": 0, // account commission tier
"canTrade": true, // if can trade
"canDeposit": true, // if can transfer in asset
"canWithdraw": true, // if can transfer out asset
"dualSidePosition": true,
"updateTime": 0, // reserved property, please ignore
"multiAssetsMargin": false,
"tradeGroupId": -1
}
```
