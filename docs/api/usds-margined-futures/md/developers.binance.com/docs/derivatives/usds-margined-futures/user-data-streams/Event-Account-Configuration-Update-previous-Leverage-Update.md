# Event: Account Configuration Update previous Leverage Update

- **protocol**: user_data_stream
- **category**: user-data-streams
- **meta_description**: Event Description

## API Summary

- **description**: When the account configuration is changed, the event type will be pushed as `ACCOUNT_CONFIG_UPDATE` When the leverage of a trade pair changes, the payload will contain the object `ac` to represent the account configuration of the trade pair, where `s` represents the specific trade pair and `l` represents the leverage When the user Multi-Assets margin mode changes the payload will contain the object `ai` representing the user account configuration, where `j` represents the user Multi-Assets margin mode
- **url_path**: `/private`
- **event_name**: `ACCOUNT_CONFIG_UPDATE`

### Response Examples

```javascript
{
"e":"ACCOUNT_CONFIG_UPDATE", // Event Type
"E":1611646737479, // Event Time
"T":1611646737476, // Transaction Time
"ac":{
"s":"BTCUSDT", // symbol
"l":25 // leverage

}
}
```

```javascript
{
"e":"ACCOUNT_CONFIG_UPDATE", // Event Type
"E":1611646737479, // Event Time
"T":1611646737476, // Transaction Time
"ai":{ // User's Account Configuration
"j":true // Multi-Assets Mode
}
}
```

## Event Description

When the account configuration is changed, the event type will be pushed as `ACCOUNT_CONFIG_UPDATE` When the leverage of a trade pair changes, the payload will contain the object `ac` to represent the account configuration of the trade pair, where `s` represents the specific trade pair and `l` represents the leverage When the user Multi-Assets margin mode changes the payload will contain the object `ai` representing the user account configuration, where `j` represents the user Multi-Assets margin mode

## URL PATH

`/private`

## Event Name

`ACCOUNT_CONFIG_UPDATE`

## Response Example

> Payload:

```javascript
{
"e":"ACCOUNT_CONFIG_UPDATE", // Event Type
"E":1611646737479, // Event Time
"T":1611646737476, // Transaction Time
"ac":{
"s":"BTCUSDT", // symbol
"l":25 // leverage

}
}
```

> Or

```javascript
{
"e":"ACCOUNT_CONFIG_UPDATE", // Event Type
"E":1611646737479, // Event Time
"T":1611646737476, // Transaction Time
"ai":{ // User's Account Configuration
"j":true // Multi-Assets Mode
}
}
```
