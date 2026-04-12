# 6. Core types and primitives

This document covers the low-level building blocks used throughout binapi2:
the `decimal_t` fixed-point type, the `symbol_t` / `pair_t` strong types,
`timestamp_ms_t`, the enum set, the `result<T>` monad, and the supporting
glaze serialization. Every request and response struct is built from these
primitives — understanding them lets you write correct, portable code.

## `decimal_t` — exact fixed-point money

Cryptocurrency prices and quantities cannot be stored as `double` without
losing precision. `0.1 + 0.2` is not `0.3` in IEEE 754; rounding errors
compound across arithmetic and eventually produce off-by-one-satoshi
mistakes that get orders rejected by the exchange.

**`binapi2::fapi::types::decimal_t`** is a string-backed 128-bit fixed-point
decimal. It holds the exact digits as they appeared in JSON, and supports
addition, subtraction, multiplication, and division with controlled
precision.

```cpp
#include <binapi2/fapi/types/detail/decimal.hpp>

using binapi2::fapi::types::decimal_t;

decimal_t price{"60000.50"};        // exact
decimal_t qty{"0.001"};             // exact
decimal_t notional = price * qty;   // 60.00050 — exact
```

### Construction

```cpp
decimal_t a;                         // default: "0"
decimal_t b{"123.456"};              // from string literal
decimal_t c = decimal_t{"0.00000001"}; // smallest satoshi
decimal_t d = decimal_t(std::string{"42"}); // from std::string
```

**Invalid input** (non-numeric characters) throws `std::invalid_argument`.
Library code never constructs `decimal_t` from untrusted strings without
validation — typically it comes from JSON where glaze has already verified
the format.

### Arithmetic

```cpp
decimal_t a{"10.5"}, b{"3.2"};

decimal_t sum  = a + b;              // 13.7 — exact
decimal_t diff = a - b;              // 7.3  — exact
decimal_t prod = a * b;              // 33.60 — exact

// Division can lose precision — returns a decimal_result_t
auto q = div(a, b);                  // q.value, q.is_exact()
if (q.is_exact()) {
    decimal_t quotient = q.value;
}
```

Multiplication preserves the exact product up to 128-bit precision.
Division is not always exact (e.g. `1/3`) and returns
`decimal_result_t { decimal_t value; bool is_exact(); }` so callers can
decide whether to accept the rounded result.

### Comparison

All standard operators are supported (`==`, `!=`, `<`, `<=`, `>`, `>=`).
Comparisons are on numeric value, so `decimal_t{"10"} == decimal_t{"10.00"}`.

```cpp
if (price >= decimal_t{"60000"}) { /* ... */ }
if (qty.is_zero()) { /* ... */ }
if (qty.is_positive()) { /* ... */ }
```

### String conversion

```cpp
std::string s = d.to_string();       // canonical representation

// fmt / spdlog integration
spdlog::info("price: {}", price);    // uses fmt::formatter<decimal_t>
```

### JSON parsing

`decimal_t` deserializes from **both** quoted strings and unquoted JSON
numbers. Binance inconsistently uses both wire formats:

```json
{
  "price": "60000.50",           // quoted string — most common
  "maintMarginRatio": 0.004      // unquoted number — leverage brackets, commissions
}
```

Both produce the same `decimal_t` value. Details in
[../json_parsing.md](../json_parsing.md).

### Why not arbitrary precision?

128-bit is enough for any Binance price or quantity with room to spare
(prices up to `2^63 ≈ 9.2×10^18`, with 18 digits of fractional precision).
Arbitrary precision would require heap allocation per arithmetic op, hurting
the hot path.

## `symbol_t` and `pair_t` — trading symbols

### `symbol_t`

The exchange-level identifier for a tradeable instrument:
- Perpetual: `BTCUSDT`, `ETHUSDT`, `BNBUSDT`, ...
- Delivery: `BTCUSDT_250627`, `ETHUSDT_250926`, ...

```cpp
#include <binapi2/fapi/types/detail/symbol.hpp>

using binapi2::fapi::types::symbol_t;

symbol_t s1{"BTCUSDT"};          // uppercase
symbol_t s2{"btcusdt"};          // also valid — constructor uppercases
assert(s1 == s2);                // true: "BTCUSDT" == "BTCUSDT"

std::string_view v = s1.view();  // "BTCUSDT"
```

**Uppercase normalization in the constructor** means the user can type
symbols in any case from the command line or config file, and the library
handles the conversion. The Binance REST API happens to accept any case,
but stream topics require lowercase — `stream_traits` always lowercases
explicitly before building the WS URL, so both REST and stream paths work
regardless of the input case.

### `pair_t`

The base/quote asset combination without the expiry suffix:
- Perpetual: `BTCUSDT` (same as symbol)
- Delivery: `BTCUSDT` (symbol `BTCUSDT_250627` has pair `BTCUSDT`)

Used by endpoints that aggregate across all contracts for a pair, e.g.
`continuous_kline_request_t`, `basis_request_t`, `delivery_price_request_t`.

```cpp
pair_t p{"BTCUSDT"};             // same ergonomics as symbol_t
```

Both `symbol_t` and `pair_t` are strong types — they won't implicitly
convert to or from each other, catching accidental misuse at compile time:

```cpp
pair_t p{"BTCUSDT"};
// continuous_kline_request_t needs pair_t — this works:
continuous_kline_request_t req{.pair = p};
// klines_request_t needs symbol_t — passing pair_t would be a compile error
```

### Why strong types?

A bare `std::string` symbol is indistinguishable from any other string:
order IDs, asset names, error messages. Strong types catch mistakes like
passing an asset name where a symbol is expected.

## `timestamp_ms_t` — Unix timestamps

```cpp
#include <binapi2/fapi/types/detail/timestamp.hpp>

using binapi2::fapi::types::timestamp_ms_t;

timestamp_ms_t t{1776013500000};    // ms since epoch
std::uint64_t ms = t.value();

// Arithmetic (ms units)
timestamp_ms_t later = t + 60'000;  // 60 seconds later
```

All Binance timestamps are milliseconds since Unix epoch. `timestamp_ms_t`
is a thin wrapper over `std::uint64_t` for type safety. JSON parses as a
number (Binance never quotes timestamps).

To convert to `std::chrono::system_clock::time_point`, call `to_time_point()`.
To format as ISO 8601, use `fmt::format("{}", t)` — the formatter is
included.

## Enums

All Binance string constants are exposed as `enum class` types with
`to_string()` overloads that produce the canonical wire format:

```cpp
using namespace binapi2::fapi::types;

order_side_t s = order_side_t::buy;
std::string wire = to_string(s);    // "BUY"

order_type_t t = order_type_t::limit;
std::string wire2 = to_string(t);   // "LIMIT"
```

Glaze serializes them as strings via `glz::meta<E>` specialization. JSON
parsing uses the string directly — a response with `"side": "BUY"` parses
into `order_side_t::buy`.

### Complete enum list

| Enum | Values | Used in |
|------|--------|---------|
| `security_type_t` | `none`, `market_data`, `user_stream`, `user_data`, `trade` | Internal (endpoint metadata) |
| `order_side_t` | `buy`, `sell` | Orders |
| `order_type_t` | `limit`, `market`, `stop`, `stop_market`, `take_profit`, `take_profit_market`, `trailing_stop_market` | Orders |
| `time_in_force_t` | `gtc`, `ioc`, `fok`, `gtx`, `gtd` | Orders |
| `order_status_t` | `new_order`, `partially_filled`, `filled`, `canceled`, `rejected`, `expired`, `new_adl`, `new_insurance`, `expired_in_match` | Order state machine |
| `position_side_t` | `both`, `long_`, `short_` | Hedge mode orders |
| `margin_type_t` | `isolated`, `crossed` | Account config |
| `working_type_t` | `mark_price`, `contract_price` | Stop trigger source |
| `response_type_t` | `ack`, `result` | Order placement response verbosity |
| `contract_type_t` | `perpetual`, `current_month`, `next_month`, `current_quarter`, `next_quarter`, `perpetual_delivering`, `tradifi_perpetual` | Delivery contract classification |
| `contract_status_t` | `pending`, `pre_trading`, `trading`, `post_trading`, `end_of_day`, `halt`, `auction_match`, `break_`, `settling`, `close`, `pending_listing`, `pre_delivering`, `delivering`, `delivered` | Symbol lifecycle state |
| `stp_mode_t` | `none`, `expire_taker`, `expire_maker`, `expire_both` | Self-trade prevention |
| `price_match_t` | `none`, `opponent`, `opponent_5`, `opponent_10`, `opponent_20`, `queue`, `queue_5`, `queue_10`, `queue_20` | Automatic price following |
| `income_type_t` | 17 values including `transfer`, `realized_pnl`, `funding_fee`, `commission`, `insurance_clear`, ... | Income history |
| `kline_interval_t` | `_1m`, `_3m`, `_5m`, `_15m`, `_30m`, `_1h`, `_2h`, `_4h`, `_6h`, `_8h`, `_12h`, `_1d`, `_3d`, `_1w`, `_1M` | Klines, analytics |
| `futures_data_period_t` | period intervals for analytics | Long/short ratios, OI stats |
| `reason_type_t` | `deposit`, `withdraw`, `order`, `funding_fee`, ... | Account update event |
| `execution_type_t` | `new_order`, `canceled`, `calculated`, `expired`, `trade`, `amendment` | Order trade update |
| `algo_type_t` | `vp`, `twap` | Algo orders |
| `algo_status_t` | algo lifecycle states | Algo order update |
| `strategy_type_t`, `strategy_status_t` | grid/strategy management | Grid/strategy events |

Reading any JSON with a string that doesn't match a known enumerator is a
**parse error** (`error_code::json`) — intentional, because a new value
means Binance added something the library needs to support.

### `to_string()` and exceptions

```cpp
try {
    auto s = to_string(static_cast<order_side_t>(99));
} catch (const std::invalid_argument& e) {
    // "invalid order_side_t: 99"
}
```

`to_string()` throws `std::invalid_argument` on out-of-range values (e.g.
a corrupted enum loaded from a file). The library itself never corrupts
enums — this is a safety net for user code.

## `enum_set<E>` — bitset of enum flags

Some fields (notably `symbol_info_t::orderTypes` and
`exchange_info_response_t` filter support sets) represent a subset of enum
values. `enum_set<E>` is a compile-time sized bitset with JSON array
serialization.

```cpp
using order_types_set = enum_set<order_type_t>;

order_types_set supported;
supported.insert(order_type_t::limit);
supported.insert(order_type_t::market);
supported.insert(order_type_t::stop);

if (supported.contains(order_type_t::limit)) { /* ... */ }

// JSON: ["LIMIT","MARKET","STOP"]
```

Operations: `insert`, `erase`, `contains`, `size`, iteration. 32 test
cases in `tests/binapi2/fapi/enum_set_test.cpp`.

## `result<T>` — error monad

Every fallible library operation returns `result<T>`:

```cpp
template<typename T>
struct result
{
    T value;
    error err;                       // error details on failure

    operator bool() const noexcept;  // true on success
    T& operator*();
    T* operator->();

    static result<T> success(T);
    static result<T> failure(error);
};
```

The idiomatic usage:

```cpp
auto r = co_await (*rest)->market_data.async_execute(ping_request_t{});
if (!r) {
    spdlog::error("{}", r.err.message);
    co_return 1;
}
// use *r or r->field
```

### `error` structure

```cpp
struct error
{
    error_code code;                // enum: none, invalid_argument, transport, http_status, json, binance, websocket, internal
    int http_status;                // HTTP status if code == http_status or binance
    int binance_code;               // Binance error code if code == binance
    std::string message;            // human-readable
    std::string payload;            // raw response body on parse failures
};
```

**`error_code::binance`** means Binance returned a structured error document
(e.g. `{"code":-2010,"msg":"Account has insufficient balance"}`). Check
`binance_code` against the [Binance error code list](https://binance-docs.github.io/apidocs/futures/en/#error-codes).

**`error_code::json`** means the response could not be parsed (missing key,
type mismatch, unknown enum). `payload` contains the raw body for debugging.

**`error_code::transport`** is a socket or TLS failure. `payload` may contain
the underlying `boost::system::error_code` description.

### `result<void>`

Operations that don't return a value use `result<void>`:

```cpp
// local_order_book::async_run returns result<void>
auto r = co_await book.async_run(symbol_t{"BTCUSDT"}, 1000);
if (!r) {
    spdlog::error("{}", r.err.message);
}
```

### Chaining

There is no operator chaining — binapi2 does not use Haskell-style monadic
combinators. Use `if (!r)` guards or early `co_return`:

```cpp
auto rest = co_await api.create_rest_client();
if (!rest) co_return rest.err;

auto info = co_await (*rest)->market_data.async_execute(exchange_info_request_t{});
if (!info) co_return info.err;

// ...
```

## Glaze serialization

binapi2 uses [Glaze](https://github.com/stephenberry/glaze) for
compile-time JSON serialization. Most types either:

1. Use aggregate reflection directly (no `glz::meta` needed) when the C++
   field names match the JSON keys.
2. Provide a `glz::meta<T>` specialization that maps C++ field names to
   the short Binance keys (e.g. `"s"` for `symbol`).

Stream event types almost always need custom `glz::meta` because Binance
uses terse single-letter keys.

Parse behaviour:
- `error_on_unknown_keys = false` — extra fields are ignored. Binance
  regularly adds fields; the library must tolerate them.
- `error_on_missing_keys = true` — required fields must be present.
  Missing keys produce `error_code::json`.

Full details in [../json_parsing.md](../json_parsing.md).

## `common.hpp` — shared support types

A few types used across all API surfaces:

```cpp
struct rate_limit_t
{
    std::string rateLimitType;       // REQUEST_WEIGHT, ORDERS, RAW_REQUESTS
    std::string interval;            // SECOND, MINUTE, DAY
    int intervalNum;
    int limit;
};

struct price_level_t
{
    decimal_t price;
    decimal_t quantity;
};

struct server_time_response_t
{
    timestamp_ms_t serverTime;
};

struct listen_key_response_t
{
    std::string listenKey;
};
```

`price_level_t` is a pair of `(price, quantity)` — used in order book bids
and asks, and in mark price bracketed quotes.

`rate_limit_t` is returned as part of `exchange_info_response_t` and in
every `websocket_api_response_t`, telling you the current usage of each
rate-limited resource.

## Secrets and configuration

The credential types are intentionally simple:

```cpp
struct config
{
    // Network
    std::string rest_host, rest_port, rest_base_path;
    std::string websocket_api_host, websocket_api_port, websocket_api_target;
    std::string stream_host, stream_port, stream_base_target;

    // Credentials
    std::string api_key;                      // always required for signed endpoints
    std::string secret_key;                   // HMAC path
    std::string ed25519_private_key_pem;      // Ed25519 path (default)
    sign_method_t sign_method = sign_method_t::ed25519;

    // Tuning
    std::uint64_t recv_window = 5000;
    std::string user_agent = "binapi2-fapi/0.1.0";
    bool testnet = false;

    // Optional transport logging callback
    transport_logger logger{};

    static config testnet_config();
};
```

The credential fields are plain `std::string`s — binapi2 does not own the
secret storage mechanism. Use the separate `secret_provider` library (see
[../secret_provider.md](../secret_provider.md)) to load them from libsecret,
systemd-creds, environment, or a test provider.

## Summary

| Type | Purpose | Key rules |
|------|---------|-----------|
| `decimal_t` | Exact fixed-point money | Always use for prices, quantities, notionals. No `double` in API types. |
| `symbol_t` | Trading symbol | Auto-uppercased. Stream topics automatically lowercased by `stream_traits`. |
| `pair_t` | Base/quote pair | Distinct type from `symbol_t` for compile-time safety. |
| `timestamp_ms_t` | Unix ms timestamp | Thin wrapper over `uint64_t`. |
| enums | Type-safe wire constants | `to_string()` returns wire format. Unknown JSON value is a parse error. |
| `enum_set<E>` | Bitset of enum values | For `orderTypes`, filter sets. |
| `result<T>` | Error monad | Every I/O op returns this. Check `if (!r)` before accessing. |
| `error` | Error details | `code`, `http_status`, `binance_code`, `message`, `payload`. |
| `config` | Connection + credentials | Defaults to Ed25519 signing, production endpoints. |
