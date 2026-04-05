








update README.md

investiage the project and cover all topics
- quck start, build
- binapi versions
- dependences
- documentation, api reference, data types and implementation status
- scripts for grabbing binance public REST and WebSocket api reference and convertion to JSON and Markdown
- architecture (refer to docs/DESIGN.md)
- testing: unit tests, integration tests
- examples, HOWTOs





--------------------------------------------------------------------------------


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







