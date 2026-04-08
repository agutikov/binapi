







Write a documents about


How we suppose to work with streams?
Callbacks, local order book, what else?
Income data buffers, queues?
    - Queue size limits?
    - overflow policy (old, new, fail)?
    - access to queued data?
How to start/stop (subscribe/unsubscribe)?
Connection reuse after unsubscribe?
Multiple parallel streams?
    - different connections
    - one connection


--------------------------------------------------------------------------------

Can we decouple io_thread from client? Does it make sense?
Would it then help to make more generic library code?

What is fapi::client?
Seems fapi::client has too many different responsibilities:
    - container for services, streams and wsapi client
    - transport
    - io context / io_thread owner
    - executor


I don't think client::run_sync does have sense while io_thread already has one.

How to clearly separate execution model defined while constructing:
    - co_main
    - io_thread
    - std::async
    - ... what else?
And call model:
    - true async - co_await
    - with callback
    - semi-async/hybrid - return future
        - executed in different thread
        - caller thread can wait on future object
    - sync - return final resulting data

In all cases the core implementation is the same and is true async,
but with different:
    - origin of the execution context
    - caller
    - communication with caller


To answer those question we first need to analyse our API structure and requirements:


What API types we have?
- REST - request -> response
    - if authentication required - auth tokens in every request
    - within a dedicated connection
    - in a shared connection (connection pooling)
- WS API - also request -> response, but inside WebSocket connection instead of HTTP connection
    - require connect (same as REST) but then logon, once per session unlike REST
    - expected to share connection for multiple requests
- Streams - connect and then only receive
    - one TCP connection per stream? (is it true?)


What execution variants we have of each of API variants?

- sync
- async
- callback
- loop

make a table of all combinations with comments


Do we need to support all of them?
Or maybe don't need easy sync calls at all?
Or maybe need only production-ready setup:
    - REST:
        - single thread
        - single shared connection
        - maybe one spare connection?
        - or one priority connection?
    - WSAPI:
        - the same as for REST
    - Streams:
        - parse in the same thread or buffer and transfer to parser thread?
    - dedicated threads per API?
    - or thread per pipeline stage: serialization, network, parsing?




--------------------------------------------------------------------------------




cmd_stream_book_ticker

it's manual implementation over async transport
where is true async stream API?

maybe asio::concurrent_channel ?




--------------------------------------------------------------------------------

Throughput benchmarks w/o WS transport.
    - record some samples from testnet
    - write a benchmark that repeats those samples


--------------------------------------------------------------------------------


REST benchmark with postman mock

--------------------------------------------------------------------------------



connection pooling for REST

is it possible to pool connections for streams or ws_api?

update benchmark


--------------------------------------------------------------------------------


thread pool

update both benchmarks


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


all tests from demo-cli, including local order book



--------------------------------------------------------------------------------



binapi2 connect to real account

record something


--------------------------------------------------------------------------------



review what would need to be done to move on Folly or Seastar



