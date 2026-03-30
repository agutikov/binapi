# Classic Portfolio Margin Account Information (USER_DATA)

- **protocol**: reference
- **category**: usds-margined-futures
- **meta_description**: API Description

## API Summary

- **description**: Get Classic Portfolio Margin current account information.
- **http_method**: GET
- **path**: `/fapi/v1/pmAccountInfo`
- **http_request**: GET `/fapi/v1/pmAccountInfo`
- **request_weight**: 5

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| asset | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- maxWithdrawAmount is for asset transfer out to the spot wallet.

### Response Examples

```javascript
{
"maxWithdrawAmountUSD": "1627523.32459208", // Classic Portfolio margin maximum virtual amount for transfer out in USD
"asset": "BTC", // asset name
"maxWithdrawAmount": "27.43689636", // maximum amount for transfer out
}
```

## API Description

Get Classic Portfolio Margin current account information.

## HTTP Request

GET `/fapi/v1/pmAccountInfo`

## Request Weight

5

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| asset | STRING | YES |  |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> maxWithdrawAmount is for asset transfer out to the spot wallet.

## Response Example

```javascript
{
"maxWithdrawAmountUSD": "1627523.32459208", // Classic Portfolio margin maximum virtual amount for transfer out in USD
"asset": "BTC", // asset name
"maxWithdrawAmount": "27.43689636", // maximum amount for transfer out
}
```
