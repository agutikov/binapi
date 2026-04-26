// SPDX-License-Identifier: Apache-2.0

#include <secret_provider/factory.hpp>

#include <secret_provider/dir_provider.hpp>
#include <secret_provider/env_provider.hpp>
#include <secret_provider/systemd_creds_provider.hpp>
#include <secret_provider/test_provider.hpp>

#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
#include <secret_provider/libsecret_provider.hpp>
#endif

#include <mutex>
#include <unordered_map>

namespace secret_provider {

namespace {

struct registry
{
    std::mutex mu;
    std::unordered_map<std::string, factory_fn> entries;
};

registry& reg()
{
    static registry r;
    return r;
}

std::unique_ptr<secret_provider> dispatch_registered(std::string_view name)
{
    std::lock_guard lock(reg().mu);
    if (auto it = reg().entries.find(std::string(name)); it != reg().entries.end()) {
        return it->second({});
    }
    if (auto colon = name.find(':'); colon != std::string_view::npos) {
        auto prefix = std::string(name.substr(0, colon));
        if (auto it = reg().entries.find(prefix); it != reg().entries.end()) {
            return it->second(name.substr(colon + 1));
        }
    }
    return nullptr;
}

} // namespace

std::unique_ptr<secret_provider> create(std::string_view name)
{
    if (name == "test") {
        return std::make_unique<test_provider>();
    }

    if (name == "env") {
        return std::make_unique<env_provider>();
    }

#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
    if (name == "libsecret" || name.starts_with("libsecret:")) {
        return std::make_unique<libsecret_provider>("binapi2");
    }
#endif

    if (name.starts_with("systemd-creds:")) {
        return std::make_unique<systemd_creds_provider>(std::string(name.substr(14)));
    }

    if (name.starts_with("dir:")) {
        return std::make_unique<dir_provider>(std::string(name.substr(4)));
    }

    return dispatch_registered(name);
}

std::vector<std::string> available()
{
    std::vector<std::string> names;
    names.emplace_back("env");
    names.emplace_back("test");
    names.emplace_back("systemd-creds:<dir>");
    names.emplace_back("dir:<dir>");
#if defined(SECRET_PROVIDER_HAS_LIBSECRET)
    names.emplace_back("libsecret");
#endif

    std::lock_guard lock(reg().mu);
    for (const auto& [prefix, _] : reg().entries) {
        names.push_back(prefix);
    }
    return names;
}

void register_provider(std::string prefix, factory_fn fn)
{
    std::lock_guard lock(reg().mu);
    reg().entries[std::move(prefix)] = std::move(fn);
}

} // namespace secret_provider
