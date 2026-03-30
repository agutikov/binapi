# Get Futures Trade Download Link by Id(USER_DATA)

- **protocol**: rest
- **category**: rest-api
- **meta_description**: API Description

## API Summary

- **description**: Get futures trade download link by Id
- **http_method**: GET
- **path**: `/fapi/v1/trade/asyn/id`
- **http_request**: GET `/fapi/v1/trade/asyn/id`
- **request_weight**: 10

### Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| downloadId | STRING | YES | get by download id api |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

### Request Parameter Notes

- Download link expiration: 24h

### Response Examples

```javascript
{
"downloadId":"545923594199212032",
"status":"completed", // Enum：completed，processing
"url":"www.binance.com", // The link is mapped to download id
"notified":true, // ignore
"expirationTimestamp":1645009771000, // The link would expire after this timestamp
"isExpired":null,
}
```

```javascript
{
"downloadId":"545923594199212032",
"status":"processing",
"url":"",
"notified":false,
"expirationTimestamp":-1
"isExpired":null,

}
```

## API Description

Get futures trade download link by Id

## HTTP Request

GET `/fapi/v1/trade/asyn/id`

## Request Weight

10

## Request Parameters

| Name | Type | Mandatory | Description |
| --- | --- | --- | --- |
| downloadId | STRING | YES | get by download id api |
| recvWindow | LONG | NO |  |
| timestamp | LONG | YES |  |

> Download link expiration: 24h

## Response Example

> Response:

```javascript
{
"downloadId":"545923594199212032",
"status":"completed", // Enum：completed，processing
"url":"www.binance.com", // The link is mapped to download id
"notified":true, // ignore
"expirationTimestamp":1645009771000, // The link would expire after this timestamp
"isExpired":null,
}
```

> OR (Response when server is processing)

```javascript
{
"downloadId":"545923594199212032",
"status":"processing",
"url":"",
"notified":false,
"expirationTimestamp":-1
"isExpired":null,

}
```
