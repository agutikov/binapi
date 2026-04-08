








for all enum types - specify value of enum items

what happen if enum parser gets unknown string?
if serialization gets unknown enum value?



tests for enum_set




convert vectors of enums into enum_set



add special type for symbol - make a simple wrapper for std::string
this is not enum, but also not a random string
maybe type would be useful in future



update docs/data_types.md
update docs/DESIGN.md





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







