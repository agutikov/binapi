





How we suppose to work with streams?
Callbacks?
Income data buffers, queues?
    - Queue size limits?
    - overflow policy (old, new, fail)?
    - access to queued data?
How to start/stop (subscribe/unsubscribe)?
Multiple parallel streams?
Throughput benchmarks.


--------------------------------------------------------------------------------



connection pooling
What is it in our implementation context?
Do we need it and in what cases?


thread pool
Is it possible to use with cobalt?
How?
What benefits?


Mapping between:
- threads
- io contexts
- io objects (sockets, files)
- cobalt contexts (Does it exist?)
- binapi2 components
    - client
    - service
    - stream
    - handler
    - ... what else?

Is it possible to have?
- one io context in multiple threads
- one thread with multiple io contexts


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







