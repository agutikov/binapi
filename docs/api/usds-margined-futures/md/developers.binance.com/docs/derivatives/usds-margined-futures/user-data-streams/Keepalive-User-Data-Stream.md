# Keepalive User Data Stream (USER_STREAM)

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: API Description

## API Summary

- **description**: Keepalive a user data stream to prevent a time out. User data streams will close after 60 minutes. It's recommended to send a ping about every 60 minutes.
- **http_method**: PUT
- **path**: `/fapi/v1/listenKey`
- **http_request**: PUT `/fapi/v1/listenKey`
- **request_weight**: 1

### Request Parameter Notes

- None

### Response Examples

```javascript
{
"listenKey": "3HBntNTepshgEdjIwSUIBgB9keLyOCg5qv3n6bYAtktG8ejcaW5HXz9Vx1JgIieg" //the listenkey which got extended
}
```

## API Description

Keepalive a user data stream to prevent a time out. User data streams will close after 60 minutes. It's recommended to send a ping about every 60 minutes.

## HTTP Request

PUT `/fapi/v1/listenKey`

## Request Weight

1

## Request Parameters

None

## Response Example

```javascript
{
"listenKey": "3HBntNTepshgEdjIwSUIBgB9keLyOCg5qv3n6bYAtktG8ejcaW5HXz9Vx1JgIieg" //the listenkey which got extended
}
```
