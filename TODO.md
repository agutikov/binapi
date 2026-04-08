





How we suppose to work with streams?
Callbacks?
Income data buffers, queues?
How to start/stop (subscribe/unsubscribe)?


--------------------------------------------------------------------------------



connection pooling


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


finish testing secure endpoints, streams and ws api with testnet





--------------------------------------------------------------------------------



binapi2 connect to real account

record something


--------------------------------------------------------------------------------







