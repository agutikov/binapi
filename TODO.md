








decimal mult and div
- options:
    - with boost::multiprecision
    - manual with int256 and overflow detection
- always return 3 result parts
    - to detect overflow after mult
    - result:
        - actual result of precision (10^-18), max value limit 10^20
        - error - small reminder of type 10^-(18+36)
        - overflow - big error of type 10^()
        - special types for reminders and overflow

--------------------------------------------------------------------------------


Unit tests

Analyze what can be tested in binapi2 with unit tests?

Comprehensive and intensive testing of decimal type
cover all corner cases, especially for multiplication and division
add limits to the type



add googletest
add cmake option BINAPI2_WITH_TESTS




--------------------------------------------------------------------------------

examples

refactor

binapi2-fapi-demo client tool
- 2 verbosity levels
    - print jsons
    - print http
- all readonly requests and streams
- some other requests
    - rest
    - wsapi
- local order book demo



--------------------------------------------------------------------------------


binapi2 validation against postman


--------------------------------------------------------------------------------


binapi2 connect to testnet

--------------------------------------------------------------------------------


??? Key storage

binapi2 connect to real account

record something


--------------------------------------------------------------------------------







