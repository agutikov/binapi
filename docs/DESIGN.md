# binapi2 Design Documentation

C++ client library for **Binance USD-M Futures API**. Built on C++23, Boost.Beast/ASIO, OpenSSL, and Glaze JSON.

---

## Architecture Overview

```plantuml
@startuml
skinparam componentStyle rectangle
skinparam packageStyle frame

package "binapi2::fapi" {

  [client] as CLI

  package "REST Services" {
    [service (base)] as SVC
    [market_data_service] as MDS
    [account_service] as ACS
    [trade_service] as TRS
    [convert_service] as CVS
    [user_data_stream_service] as UDS
  }

  package "WebSocket Streams" {
    [market_streams] as MS
    [user_streams] as US
    [local_order_book] as LOB
  }

  package "WebSocket API" {
    [websocket_api::client] as WSA
  }

  package "Transport" {
    [http_client] as HTTP
    [websocket_client] as WS
  }

  package "Infrastructure" {
    [config] as CFG
    [signing] as SIGN
    [query (to_query_map)] as QRY
    [endpoint_traits] as ET
    [result<T> / error] as RES
    [types] as TYP
  }
}

cloud "Binance" {
  [REST API\nfapi.binance.com] as BREST
  [WebSocket Streams\nfstream.binance.com] as BSTREAM
  [WebSocket API\nws-fapi.binance.com] as BWSAPI
}

MDS --|> SVC
ACS --|> SVC
TRS --|> SVC
CVS --|> SVC

CLI --> MDS
CLI --> ACS
CLI --> TRS
CLI --> CVS
CLI --> UDS

SVC --> CLI : execute(request)
SVC --> ET : endpoint_traits<Request>
SVC --> QRY : to_query_map(request)

CLI --> HTTP
WSA --> WS

MS --> WS
US --> WS
LOB --> MS
LOB --> CLI

HTTP --> SIGN
WSA --> SIGN
HTTP --> BREST
WS --> BSTREAM
WS --> BWSAPI
@enduml
```

---

## Generic Request Dispatch

The core design pattern: **request types carry all the information needed to dispatch an API call**.

### REST API

```plantuml
@startuml
skinparam classAttributeIconSize 0

class "rest::service" as SVC {
  #owner_ : client&
  --
  +execute<Request>(request) : result<response_type>
  +async_execute<Request>(request, callback) : void
}

class "rest::endpoint_traits<Request>" as ET <<specialization>> {
  +{static} response_type : type alias
  +{static} endpoint : endpoint_metadata&
}

class "endpoint_metadata" as EM {
  +name : string_view
  +method : http::verb
  +path : string_view
  +security : security_type
  +signed_request : bool
}

class "to_query_map<T>" as QM <<function>> {
  Reflects struct fields via glz::reflect<T>
  Skips nullopt optionals
  Stringifies enums via to_string()
  Returns query_map
}

class "market_data_service" as MDS {
  Inherits execute/async_execute
  --
  using ping_request = types::ping_request
  using exchange_info_request = ...
  ... (type aliases)
  --
  klines(kline_request) : named method
  .. (shared request type methods)
}

class "client" as CLI {
  +execute<Request>(request) : result<response_type>
  +async_execute<Request>(request, callback) : void
  --
  +execute<Response>(method, path, query, signed)
  +async_execute<Response>(method, path, query, signed, callback)
}

MDS --|> SVC
SVC --> CLI : delegates
SVC ..> ET : resolves endpoint
SVC ..> QM : serializes request
ET --> EM
@enduml
```

**How it works:**

1. `service::execute(request)` looks up `endpoint_traits<Request>` at compile time to get the endpoint metadata and response type
2. `to_query_map(request)` uses `glz::reflect<T>` to serialize the request struct fields into a `query_map`, skipping `std::optional` nullopts and converting enums via `to_string()`
3. `client::execute<Response>()` handles signing, query string encoding, HTTP transport, and JSON response deserialization

**Usage:**
```cpp
// Via service (grouped by domain):
auto result = client.market_data.execute(exchange_info_request{});
auto result = client.trade.execute(new_order_request{.symbol = "BTCUSDT", ...});

// Or directly on client:
auto result = client.execute(types::cancel_order_request{.symbol = "BTCUSDT", .orderId = 123});

// Async:
client.trade.async_execute(new_order_request{...}, [](auto result) { ... });
```

### WebSocket API

```plantuml
@startuml
skinparam classAttributeIconSize 0

class "websocket_api::client" as WSC {
  +execute<Request>(request) : result<ws_response<response_type>>
  +async_execute<Request>(request, callback) : void
  --
  -inject_auth(request) : Request
  -send_rpc<Response>(method, params) : result<ws_response<Response>>
  -make_signed_request_base() : signed_request
}

class "ws::endpoint_traits<Request>" as WET <<specialization>> {
  +{static} response_type : type alias
  +{static} method : string_view (RPC method name)
}

note right of WSC
  Auth injection is automatic:
  if Request inherits from
  websocket_api_signed_request,
  auth fields are populated.
end note

WSC ..> WET : resolves method + response
@enduml
```

**How it works:**

1. `ws::client::execute(request)` looks up `ws::endpoint_traits<Request>` to get the RPC method name and response type
2. If the request type inherits from `websocket_api_signed_request`, auth fields (apiKey, timestamp, recvWindow, signature) are injected automatically
3. The request is serialized into a JSON-RPC envelope `{id, method, params}` and sent over the WebSocket connection

**Usage:**
```cpp
ws_client.execute(websocket_api_order_place_request{.symbol = "BTCUSDT", .side = "BUY", ...});
ws_client.execute(websocket_api_book_ticker_request{.symbol = "BTCUSDT"});
```

---

## Request Flow

```plantuml
@startuml
actor User
participant "service" as SVC
participant "endpoint_traits" as ET
participant "to_query_map" as QM
participant "client" as C
participant "signing" as S
participant "http_client" as H
participant "Binance REST" as B

User -> SVC: execute(kline_request{symbol, interval, ...})
SVC -> ET: endpoint_traits<kline_request>::endpoint
ET --> SVC: {GET, "/fapi/v1/klines", unsigned}
SVC -> QM: to_query_map(request)
QM -> QM: reflect fields via glz::reflect<T>
QM -> QM: skip nullopt, stringify enums
QM --> SVC: query_map{"symbol":"BTCUSDT","interval":"1h",...}
SVC -> C: execute<vector<kline>>(GET, path, query, false)
C -> S: inject_auth_query + sign_query (if signed)
C -> H: request(GET, path?query, "", api_key)
H -> B: HTTPS GET /fapi/v1/klines?symbol=BTCUSDT&interval=1h
B --> H: JSON response
H --> C: http_response{status, body}
C -> C: decode_response<vector<kline>>(response)
C --> User: result<vector<kline>>
@enduml
```

---

## Stream Lifecycle

```plantuml
@startuml
actor User
participant "market_streams" as MS
participant "websocket_client" as WS
participant "Binance WSS" as B

User -> MS: connect_aggregate_trade({symbol:"BTCUSDT"})
MS -> WS: connect("fstream.binance.com", "/ws/btcusdt@aggTrade")
WS -> B: WSS handshake
B --> WS: connected
WS --> MS: result<void> success
MS --> User: result<void>

User -> MS: read_aggregate_trade_loop(handler)
loop until handler returns false
  B --> WS: JSON message
  WS --> MS: raw text
  MS -> MS: deserialize to aggregate_trade_stream_event
  MS -> User: handler(event) -> bool
end
@enduml
```

---

## Local Order Book Sync

```plantuml
@startuml
participant "local_order_book" as LOB
participant "market_streams" as MS
participant "client (REST)" as REST
participant "Binance" as B

LOB -> MS: connect_diff_book_depth({symbol})
MS -> B: WSS subscribe depth stream

LOB -> LOB: buffer incoming depth events

LOB -> REST: execute(order_book_request{symbol, limit})
REST -> B: GET /fapi/v1/depth
B --> REST: snapshot (lastUpdateId = S)
REST --> LOB: order_book_response

LOB -> LOB: discard buffered events where u <= S
LOB -> LOB: apply remaining buffered events
LOB -> LOB: synced = true

loop continuous
  B --> MS: depth diff event
  MS --> LOB: apply_event(event)
  LOB -> LOB: update bids/asks maps
  LOB -> LOB: notify snapshot_callback
end
@enduml
```

---

## Type System

### Request → Endpoint Mapping

Request types with a 1:1 endpoint mapping have `endpoint_traits` (REST) or `ws::endpoint_traits` (WebSocket API) specializations. These are dispatched generically via `execute(request)`.

Shared request types (used by multiple endpoints) retain named service methods:

| Shared Type | Endpoints | Service |
|---|---|---|
| `kline_request` | klines, mark_price_klines, premium_index_klines | market_data |
| `futures_data_request` | open_interest_statistics, top_long_short_account_ratio, top_trader_long_short_ratio, long_short_ratio, taker_buy_sell_volume | market_data |
| `download_id_request` | download_id_transaction, download_id_order, download_id_trade | account |
| `download_link_request` | download_link_transaction, download_link_order, download_link_trade | account |
| `batch_orders_request` | batch_orders, modify_batch_orders | trade |

### Query Serialization

`to_query_map<T>(request)` uses compile-time reflection via `glz::reflect<T>` to automatically build URL query parameters from request struct fields:

- `std::string` → passed as-is
- `std::uint64_t`, `int` → `std::to_string()`
- `bool` → `"true"` / `"false"`
- fapi enums → `types::to_string()` (e.g., `order_side::buy` → `"BUY"`)
- `std::optional<T>` where value is nullopt → skipped entirely
- Works with both `glz::meta`-annotated and plain `reflectable` aggregates

---

## Access Modes

Every method supports **synchronous** (returns `result<T>`) and **asynchronous** (takes `callback_type<T>`, invoked via `io_context`) access.

| Access Mode | Transport | Authentication | Latency | Use Case |
|---|---|---|---|---|
| REST Request | HTTPS | API key in header, HMAC-SHA256 signed query | Medium | Account queries, order placement, market data snapshots |
| WebSocket Stream | WSS | None (market) / Listen key (user) | Low | Real-time market data, account events |
| WebSocket API | WSS | HMAC-SHA256 per message | Lowest | Low-latency trading without HTTP overhead |
| Local Order Book | WSS + REST | None | Low | Synchronized local depth book |

---

## Service Classes

Services inherit from `rest::service` which provides generic `execute(request)` / `async_execute(request, callback)`. Each service pulls request types from the `types` namespace via `using` declarations.

### 1. Market Data Service (`rest::market_data_service`)

Public endpoints. No authentication required.

Generic (via `execute`): `ping_request`, `server_time_request`, `exchange_info_request`, `order_book_request`, `recent_trades_request`, `aggregate_trades_request`, `continuous_kline_request`, `index_price_kline_request`, `book_ticker_request`, `price_ticker_request`, `ticker_24hr_request`, `mark_price_request`, `funding_rate_history_request`, `open_interest_request`, `historical_trades_request`, `basis_request`, `price_ticker_v2_request`, `delivery_price_request`, `composite_index_info_request`, `index_constituents_request`, `asset_index_request`, `insurance_fund_request`, `adl_risk_request`, `rpi_depth_request`, `trading_schedule_request`

Named methods: `klines`, `mark_price_klines`, `premium_index_klines` (shared `kline_request`); `open_interest_statistics`, `top_long_short_account_ratio`, `top_trader_long_short_ratio`, `long_short_ratio`, `taker_buy_sell_volume` (shared `futures_data_request`); `book_tickers`, `price_tickers`, `price_tickers_v2`, `ticker_24hrs`, `mark_prices`, `funding_rate_info` (parameterless)

### 2. Account Service (`rest::account_service`)

Signed endpoints.

Generic: `position_risk_request`, `symbol_config_request`, `income_history_request`, `leverage_bracket_request`, `commission_rate_request`, `toggle_bnb_burn_request`, `quantitative_rules_request`, `pm_account_info_request`

Named methods: `account_information`, `balances`, `account_config`, `get_multi_assets_mode`, `get_position_mode`, `rate_limit_order`, `get_bnb_burn` (parameterless); `download_id_*`, `download_link_*` (shared request types)

### 3. Trade Service (`rest::trade_service`)

Signed endpoints.

Generic: `new_order_request`, `modify_order_request`, `cancel_order_request`, `query_order_request`, `cancel_all_open_orders_request`, `auto_cancel_request`, `query_open_order_request`, `all_open_orders_request`, `all_orders_request`, `position_info_v3_request`, `adl_quantile_request`, `force_orders_request`, `account_trade_request`, `change_position_mode_request`, `change_multi_assets_mode_request`, `change_leverage_request`, `change_margin_type_request`, `modify_isolated_margin_request`, `position_margin_history_request`, `order_modify_history_request`, `new_algo_order_request`, `cancel_algo_order_request`, `query_algo_order_request`, `all_algo_orders_request`

Named methods: `test_order` (alias collision), `batch_orders`, `modify_batch_orders`, `cancel_batch_orders` (special serialization); `open_algo_orders`, `cancel_all_algo_orders`, `tradfi_perps` (parameterless)

### 4. Convert Service (`rest::convert_service`)

Fully generic: `quote_request`, `accept_request`, `order_status_request`

### 5. User Data Stream Service (`rest::user_data_stream_service`)

Named methods: `start`, `keepalive`, `close`

### 6. WebSocket API (`websocket_api::client`)

Generic (via `execute`): `order_place_request`, `order_query_request`, `order_cancel_request`, `order_modify_request`, `position_request`, `book_ticker_request`, `price_ticker_request`, `algo_order_place_request`, `algo_order_cancel_request`

Named methods: `session_logon`, `account_status/v2`, `account_balance`, `account_position_v2` (shared type), `user_data_stream_start/ping/stop` (shared type), `connect`, `close`

### 7. Market Streams (`streams::market_streams`)

Real-time WebSocket market data. Connect/read loop pattern per stream type. See [implementation_status.md] for full stream list.

### 8. User Streams (`streams::user_streams`)

Real-time account events via listen key. Multiplexed event handlers for order updates, balance changes, margin calls, etc.

### 9. Local Order Book (`streams::local_order_book`)

Thread-safe synchronized local order book. Combines REST snapshots with WebSocket depth diffs.

---

## Configuration

```plantuml
@startuml
class config {
  +rest_host : string = "fapi.binance.com"
  +rest_port : string = "443"
  +rest_base_path : string = ""
  +websocket_api_host : string = "ws-fapi.binance.com"
  +websocket_api_port : string = "443"
  +websocket_api_target : string = "/ws-fapi/v1"
  +stream_host : string = "fstream.binance.com"
  +stream_port : string = "443"
  +stream_base_target : string = "/ws"
  +api_key : string
  +secret_key : string
  +recv_window : uint64 = 5000
  +user_agent : string = "binapi2-fapi/0.1.0"
  +testnet : bool = false
  --
  +{static} testnet_config() : config
}
@enduml
```

---

## Error Handling

```plantuml
@startuml
enum error_code {
  none
  invalid_argument
  transport
  http_status
  json
  binance
  websocket
  internal
}

class error {
  +code : error_code
  +http_status : int
  +binance_code : int
  +message : string
  +payload : string
}

class "result<T>" {
  +operator bool() : bool
  +operator*() : T&
  +err : error
  --
  +{static} success(T) : result<T>
  +{static} failure(error) : result<T>
}

"result<T>" --> error
error --> error_code
@enduml
```

---

## Dependencies

| Dependency | Purpose | Type |
|---|---|---|
| Boost.ASIO | Async I/O, event loop | Required |
| Boost.Beast | HTTP/WebSocket protocol | Required |
| OpenSSL | TLS (HTTPS/WSS) + HMAC-SHA256 signing | Required |
| ZLIB | Response compression | Required |
| Glaze | Compile-time JSON serialization + struct reflection | Bundled (header-only) |
| DTF | Datetime formatting | Bundled (header-only) |

**Build:** CMake 3.10+, C++23 compiler.

[implementation_status.md]: /docs/implementation_status.md
