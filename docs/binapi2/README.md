# binapi2 documentation

Documentation index for the binapi2 USD-M Futures client library.

## Architecture and design

| Document | Description |
|----------|-------------|
| [DESIGN.md](DESIGN.md) | Architecture overview, request dispatch, async model, service classes, configuration, error handling |
| [async_architecture.md](async_architecture.md) | Async-first design: API types, execution variants, source layout |
| [async_io.md](async_io.md) | Async console, logging, and file recording patterns |
| [threading_and_io.md](threading_and_io.md) | Executor ownership, coroutine environments (`cobalt::main`, `io_thread`, manual `io_context`) |
| [streams.md](streams.md) | WebSocket stream components: market, combined, dynamic, user, local order book |

## Data and parsing

| Document | Description |
|----------|-------------|
| [data_types.md](data_types.md) | Catalog of all C++ request/response types with Binance API doc references |
| [json_parsing.md](json_parsing.md) | Glaze deserializer behaviour: required/optional fields, decimal parsing, enums, errors |
| [json_parsing_variant.md](json_parsing_variant.md) | Compile-time variant dispatch design notes for user data stream events |

## Coverage and verification

| Document | Description |
|----------|-------------|
| [implementation_status.md](implementation_status.md) | Per-endpoint coverage matrix (Binance docs → source files) |
| [demo_cli_all_apis.md](demo_cli_all_apis.md) | Demo CLI command reference (135 commands) + known testnet issues |

## Integration

| Document | Description |
|----------|-------------|
| [secret_provider.md](secret_provider.md) | Credential loading via libsecret / systemd-creds / test providers |

## Benchmark results

| File | Description |
|------|-------------|
| [rest_benchmark.txt](rest_benchmark.txt) | REST endpoint latency/throughput against mock server |
| [stream_parse_benchmark.txt](stream_parse_benchmark.txt) | Stream JSON parsing throughput |
