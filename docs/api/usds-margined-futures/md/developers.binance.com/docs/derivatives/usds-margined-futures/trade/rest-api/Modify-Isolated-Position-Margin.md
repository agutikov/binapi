# Modify Isolated Position Margin(TRADE)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Modify Isolated Position Margin
- **http_method**: POST
- **path**: `/fapi/v1/positionMargin`
- **http_request**: POST `/fapi/v1/positionMargin`
- **request_weight**: 1

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| positionSide | ENUM | NO | Default `BOTH` for One-way Mode ; `LONG` or `SHORT` for Hedge Mode. It must be sent with Hedge Mode. |
| amount | DECIMAL | YES |  |
| type | INT | YES | 1: Add position margin，2: Reduce position margin |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Only for isolated symbol

### Response Examples

```json
{
  "amount": 100.0,
  "code": 200,
  "msg": "Successfully modify position margin.",
  "type": 1
}
```

## API Description

Modify Isolated Position Margin

## HTTP Request

POST `/fapi/v1/positionMargin`

## Request Weight

1

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| symbol | STRING | YES |  |
| positionSide | ENUM | NO | Default `BOTH` for One-way Mode ; `LONG` or `SHORT` for Hedge Mode. It must be sent with Hedge Mode. |
| amount | DECIMAL | YES |  |
| type | INT | YES | 1: Add position margin，2: Reduce position margin |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Only for isolated symbol

## Response Example

```json
{
  "amount": 100.0,
  "code": 200,
  "msg": "Successfully modify position margin.",
  "type": 1
}
```
