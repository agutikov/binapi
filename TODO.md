



examples/binapi2/fapi/demo-cli - add short cmdline options flags




scripts that run all binapi2-fapi-demo-cli requests with testnet
    - REST
    - WebSocket API
    - WebSocket streams
save requests, responses and logs into separate file for every command


run and collect real responses for postman mock


binapi2 connect to testnet

test all methods











--------------------------------------------------------------------------------





binapi2-fapi-demo-cli load keys from storage

add support of both:
- libsecret
- systemd-creds


SecretProvider
 ├── LibSecretProvider
 ├── SystemdCredsProvider
 └── TestSecretProvider (for tests only)


implement this in a separate library

add required interface for SecretProvider in binapi2 that works with both implementation




--------------------------------------------------------------------------------



binapi2 connect to real account

record something


--------------------------------------------------------------------------------







