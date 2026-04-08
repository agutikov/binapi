







add _t to all types in include/binapi2/fapi/types



for all enum types - specify value of enum items


tests for enum_set



what happen if enum parser gets unknown string?



convert vectors of enums into enum_set



add special type for symbol - make a simple wrapper for std::string
this is not enum, but also not a random string
maybe type would be useful in future









binapi2 connect to testnet

test all methods











script that runs all requests with testnet




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







