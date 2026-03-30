# Close User Data Stream (USER_STREAM)

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: API Description

## API Summary

- **description**: Close out a user data stream.
- **method**: `userDataStream.stop`
- **request_weight**: 1

### Request Parameter Notes

- None

### Request Examples

```json
{
  "id": "819e1b1b-8c06-485b-a13e-131326c69599",
  "method": "userDataStream.stop",
  "params": {
    "apiKey": "vmPUZE6mv9SD5VNHk9HlWFsOr9aLE2zvsw0MuIgwCIPy8atIco14y7Ju91duEh8A"
  }
}
```

### Response Examples

```json
{
  "id": "819e1b1b-8c06-485b-a13e-131326c69599",
  "status": 200,
  "result": {},
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

Close out a user data stream.

## Method

`userDataStream.stop`

## Request

```json
{
  "id": "819e1b1b-8c06-485b-a13e-131326c69599",
  "method": "userDataStream.stop",
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
  "id": "819e1b1b-8c06-485b-a13e-131326c69599",
  "status": 200,
  "result": {},
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
