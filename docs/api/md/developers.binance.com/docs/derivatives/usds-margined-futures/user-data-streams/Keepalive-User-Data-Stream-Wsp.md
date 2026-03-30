# Keepalive User Data Stream (USER_STREAM)

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: API Description

## API Summary

- **description**: Keepalive a user data stream to prevent a time out. User data streams will close after 60 minutes. It's recommended to send a ping about every 60 minutes.
- **method**: `userDataStream.ping`
- **request_weight**: 1

### Request Parameter Notes

- None

### Request Examples

```json
{
  "id": "815d5fce-0880-4287-a567-80badf004c74",
  "method": "userDataStream.ping",
  "params": {
    "apiKey": "vmPUZE6mv9SD5VNHk9HlWFsOr9aLE2zvsw0MuIgwCIPy8atIco14y7Ju91duEh8A"
  }
}
```

### Response Examples

```json
{
  "id": "815d5fce-0880-4287-a567-80badf004c74",
  "status": 200,
  "result": {
    "listenKey": "3HBntNTepshgEdjIwSUIBgB9keLyOCg5qv3n6bYAtktG8ejcaW5HXz9Vx1JgIieg"
  },
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 2
    }
  ]
}
```

## API Description

Keepalive a user data stream to prevent a time out. User data streams will close after 60 minutes. It's recommended to send a ping about every 60 minutes.

## Method

`userDataStream.ping`

## Request

```json
{
  "id": "815d5fce-0880-4287-a567-80badf004c74",
  "method": "userDataStream.ping",
  "params": {
    "apiKey": "vmPUZE6mv9SD5VNHk9HlWFsOr9aLE2zvsw0MuIgwCIPy8atIco14y7Ju91duEh8A"
  }
}
```

## Request Weight

1

## Request Parameters

None

## Response Example

```json
{
  "id": "815d5fce-0880-4287-a567-80badf004c74",
  "status": 200,
  "result": {
    "listenKey": "3HBntNTepshgEdjIwSUIBgB9keLyOCg5qv3n6bYAtktG8ejcaW5HXz9Vx1JgIieg"
  },
  "rateLimits": [
    {
      "rateLimitType": "REQUEST_WEIGHT",
      "interval": "MINUTE",
      "intervalNum": 1,
      "limit": 2400,
      "count": 2
    }
  ]
}
```
