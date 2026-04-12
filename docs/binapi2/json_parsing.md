# JSON Parsing Behaviour

How binapi2 deserializes JSON responses from the Binance USD-M Futures API.

All JSON parsing goes through a single entry point:
`detail::decode_response<T>()` in `include/binapi2/fapi/client.hpp`.
It uses the [glaze](https://github.com/stephenberry/glaze) library with
custom options defined in `detail::json_read_opts`.

## Parse options

```cpp
// include/binapi2/fapi/client.hpp
inline constexpr glz::opts json_read_opts{
    .error_on_unknown_keys = false,
    .error_on_missing_keys = true,
};
```

| Option | Value | Effect |
|---|---|---|
| `error_on_unknown_keys` | `false` | Extra JSON fields not present in the C++ struct are silently skipped. |
| `error_on_missing_keys` | `true` | Every non-optional C++ field must have a corresponding key in the JSON. |

## Field categories

### Required fields

A struct member declared as a plain (non-`std::optional`) type is **required**.
If the JSON object does not contain the corresponding key, parsing fails with
`error_code::json`.

```cpp
// include/binapi2/fapi/types/common.hpp
struct server_time_response_t
{
    timestamp_ms_t serverTime{};   // required — JSON must contain "serverTime"
};
```

### Optional fields

A struct member declared as `std::optional<T>` is **optional**.
If the JSON object does not contain the key, the member stays `std::nullopt`.
No error is raised.

```cpp
// include/binapi2/fapi/types/trade.hpp  (excerpt)
struct order_response_t
{
    std::uint64_t orderId{};                   // required
    std::optional<working_type_t> workingType{}; // optional — may be absent
    std::optional<bool> priceProtect{};          // optional
    ...
};
```

### Unknown fields

JSON keys that do not correspond to any struct member are **silently ignored**.
This is intentional: Binance regularly adds new response fields without prior
notice. Rejecting unknown keys would cause the client to break on every such
addition.

## Decimal parsing

`decimal_t` is a string-backed 128-bit fixed-point type. Its glaze deserializer
accepts **both** JSON strings and JSON numbers, because Binance inconsistently
uses both on the wire:

```json
{
    "price": "70000.50",          // string-encoded (most common)
    "maintMarginRatio": 0.004,    // number (leverage-bracket, commission)
    "cum": 0.0                    // number
}
```

When the token starts with `"`, glaze parses it as a string. Otherwise it
extracts the raw digits of the JSON number verbatim to preserve exact
precision (no intermediate `double` conversion). See
`include/binapi2/fapi/types/detail/decimal.hpp` and `DecimalJson.*` tests in
`tests/binapi2/fapi/decimal_test.cpp`.

## Enum parsing

Enum values are deserialized from their wire-format strings via
`glz::meta<E>` specializations in `include/binapi2/fapi/types/enums.hpp`.

| Scenario | Behaviour | Rationale |
|---|---|---|
| Known string (e.g. `"BUY"`) | Parsed into the corresponding enumerator. | Normal path. |
| Unknown string (e.g. `"NEW_SIDE"`) | Parse error (`error_code::json`). | A new enum value means the API type definition has changed; the library must be updated. |

## Enum serialization

`to_string()` overloads in `include/binapi2/fapi/types/enums.hpp` convert
enumerators to their wire-format strings. Every overload has explicit `case`
labels for all valid values and **throws `std::invalid_argument`** if the
underlying integer does not match any known enumerator (i.e. the value is
corrupted).

```cpp
// include/binapi2/fapi/types/enums.hpp  (pattern used by all to_string overloads)
[[nodiscard]] inline std::string
to_string(order_side_t value)
{
    switch (value) {
        case order_side_t::buy:  return "BUY";
        case order_side_t::sell: return "SELL";
    }
    throw std::invalid_argument("invalid order_side_t: " + std::to_string(static_cast<int>(value)));
}
```

## Error propagation

`decode_response<T>()` wraps all parse failures into `error`:

| Stage | `error_code` | Details |
|---|---|---|
| HTTP status 4xx/5xx with Binance error body | `binance` | `binance_code` and `message` populated from the error document. |
| HTTP status 4xx/5xx without parseable error | `http_status` | Raw body available in `payload`. |
| JSON parse failure (missing key, bad type, bad enum, malformed JSON) | `json` | `message` contains the glaze-formatted error with position info. |

Source: `include/binapi2/fapi/client.hpp`, `detail::decode_response<T>()`.

## Test coverage

Tests in `tests/binapi2/fapi/`:

| Test suite | File | What it covers |
|---|---|---|
| `ResponseParse.*` | `response_parse_test.cpp` | Fixture-based round-trip parsing of all response types. |
| `ExtraFields.*` | `response_parse_test.cpp` | JSON with unknown keys parses successfully (7 tests). |
| `MissingOptional.*` | `response_parse_test.cpp` | Absent `std::optional` fields default to `nullopt` (2 tests). |
| `MissingRequired.*` | `response_parse_test.cpp` | Absent required fields produce `error_code::json` (5 tests). |
| `Enums.*` | `enums_test.cpp` | Wire-format round-trip for all enum values. |
| `EnumsCorrupted.*` | `enums_test.cpp` | `to_string()` throws on out-of-range values (26 tests). |
