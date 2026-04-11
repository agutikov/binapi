# Secret Provider Library

## Overview

`libs/secret_provider/` is a generic credential provider library for loading
API keys and secrets from secure storage backends. It is independent of binapi2
and can be reused in any C++23 project.

Namespace: `secret_provider::`

## Interface

```cpp
class secret_provider {
public:
    virtual ~secret_provider() = default;

    [[nodiscard]] virtual boost::cobalt::task<std::expected<std::string, std::string>>
    async_get_secret(std::string_view key) = 0;
};
```

Returns `std::expected<string, string>` — value on success, error message on failure.
Async via `cobalt::task` to support potentially slow backends (D-Bus, subprocess).

## Implementations

| Provider | Backend | Header | Build guard |
|----------|---------|--------|-------------|
| `test_provider` | In-memory map | `test_provider.hpp` | Always |
| `systemd_creds_provider` | `systemd-creds decrypt` subprocess | `systemd_creds_provider.hpp` | Always |
| `libsecret_provider` | Secret Service D-Bus (GNOME Keyring, KDE Wallet) | `libsecret_provider.hpp` | `SECRET_PROVIDER_HAS_LIBSECRET` |

### test_provider

```cpp
test_provider provider;
provider.set("api_key", "my-secret");
auto val = co_await provider.async_get_secret("api_key"); // -> "my-secret"
```

### systemd_creds_provider

Decrypts systemd credential files via `systemd-creds decrypt <file> -`.

```cpp
systemd_creds_provider provider("/etc/credstore");
auto val = co_await provider.async_get_secret("binance-api-key");
```

Create credentials:
```bash
echo -n "your-api-key" | systemd-creds encrypt - /etc/credstore/binance-api-key
```

### libsecret_provider

Looks up secrets from the system keyring via the Secret Service D-Bus protocol.

```cpp
libsecret_provider provider("binapi2");
auto val = co_await provider.async_get_secret("binance/api_key");
```

Store secrets with `secret-tool`:
```bash
secret-tool store --label "binapi2 API key" service binapi2 key binance/api_key
secret-tool store --label "binapi2 secret" service binapi2 key binance/secret_key
```

Schema attributes: `{ "service": <service_name>, "key": <key> }`.

## binapi2 Integration

`include/binapi2/fapi/secret_config.hpp` provides a helper to load both API
credentials into a config:

```cpp
#include <binapi2/fapi/secret_config.hpp>
#include <secret_provider/libsecret_provider.hpp>

secret_provider::libsecret_provider provider("binapi2");
auto r = co_await binapi2::fapi::async_load_credentials(
    cfg, provider, "binance/api_key", "binance/secret_key");

if (!r) {
    // Fall back to environment variables
    if (const char* k = std::getenv("BINANCE_API_KEY")) cfg.api_key = k;
    if (const char* s = std::getenv("BINANCE_SECRET_KEY")) cfg.secret_key = s;
}
```

## Build

libsecret is auto-detected via `pkg-config`. When found, `SECRET_PROVIDER_HAS_LIBSECRET`
is defined and `libsecret_provider` is compiled. systemd-creds provider is always
available (requires `/usr/bin/systemd-creds` at runtime).

```cmake
# Automatically included from top-level CMakeLists.txt:
add_subdirectory(libs/secret_provider)
```

## Source Reference

| File | Purpose |
|------|---------|
| `libs/secret_provider/include/secret_provider/secret_provider.hpp` | Abstract base class |
| `libs/secret_provider/include/secret_provider/test_provider.hpp` | In-memory test provider |
| `libs/secret_provider/include/secret_provider/systemd_creds_provider.hpp` | systemd-creds header |
| `libs/secret_provider/include/secret_provider/libsecret_provider.hpp` | libsecret header |
| `libs/secret_provider/src/systemd_creds_provider.cpp` | subprocess implementation |
| `libs/secret_provider/src/libsecret_provider.cpp` | D-Bus implementation |
| `include/binapi2/fapi/secret_config.hpp` | binapi2 config integration helper |
