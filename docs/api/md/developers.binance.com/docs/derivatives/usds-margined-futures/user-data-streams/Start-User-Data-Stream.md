# Start User Data Stream (USER_STREAM)

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: API Description

## API Summary

- **description**: Start a new user data stream. The stream will close after 60 minutes unless a keepalive is sent. If the account has an active `listenKey`, that `listenKey` will be returned and its validity will be extended for 60 minutes.
- **http_method**: POST
- **path**: `/fapi/v1/listenKey`
- **http_request**: POST `/fapi/v1/listenKey`
- **request_weight**: 1

### Request Parameter Notes

- None

### Response Examples

```json
{
  "listenKey": "pqia91ma19a5s61cv6a81va65sdf19v8a65a1a5s61cv6a81va65sdf19v8a65a1"
}
```

## API Description

Start a new user data stream. The stream will close after 60 minutes unless a keepalive is sent. If the account has an active `listenKey`, that `listenKey` will be returned and its validity will be extended for 60 minutes.

## HTTP Request

POST `/fapi/v1/listenKey`

## Request Weight

1

## Request Parameters

None

## Response Example

```json
{
  "listenKey": "pqia91ma19a5s61cv6a81va65sdf19v8a65a1a5s61cv6a81va65sdf19v8a65a1"
}
```
