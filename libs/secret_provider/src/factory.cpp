// SPDX-License-Identifier: Apache-2.0

#include <secret_provider/factory.hpp>

#include <secret_provider/env_provider.hpp>
#include <secret_provider/systemd_creds_provider.hpp>
#include <secret_provider/test_provider.hpp>

#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
#include <secret_provider/libsecret_provider.hpp>
#endif

namespace secret_provider {

std::unique_ptr<secret_provider> create(std::string_view name)
{
    if (name == "test") {
        return std::make_unique<test_provider>();
    }

    if (name == "env") {
        return std::make_unique<env_provider>();
    }

#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
    if (name == "libsecret") {
        return std::make_unique<libsecret_provider>("binapi2");
    }
    if (name.starts_with("libsecret:")) {
        return std::make_unique<libsecret_provider>("binapi2");
    }
#endif

    if (name.starts_with("systemd-creds:")) {
        auto dir = std::string(name.substr(14));
        return std::make_unique<systemd_creds_provider>(dir);
    }

    return nullptr;
}

std::vector<std::string> available()
{
    std::vector<std::string> names;
    names.emplace_back("env");
    names.emplace_back("test");
    names.emplace_back("systemd-creds:<dir>");
#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
    names.emplace_back("libsecret");
#endif
    return names;
}

} // namespace secret_provider
