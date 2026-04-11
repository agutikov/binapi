// SPDX-License-Identifier: Apache-2.0

#include <secret_provider/libsecret_provider.hpp>

#include <boost/asio/post.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/this_coro.hpp>

#include <libsecret/secret.h>

namespace secret_provider {

// Schema for storing/retrieving secrets.
// Attributes: "service" (application name) + "key" (secret identifier).
static const SecretSchema schema = {
    "secret_provider.generic",
    SECRET_SCHEMA_DONT_MATCH_NAME,
    {
        {"service", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {nullptr, SecretSchemaAttributeType(0)},
    }};

libsecret_provider::libsecret_provider(std::string service_name) :
    service_name_(std::move(service_name))
{
}

boost::cobalt::task<std::expected<std::string, std::string>>
libsecret_provider::async_get_secret(std::string_view key)
{
    auto exec = co_await boost::cobalt::this_coro::executor;

    std::string result;
    std::string error;
    std::string key_str(key);

    auto do_lookup = [&] {
        GError* gerror = nullptr;
        gchar* password = secret_password_lookup_sync(
            &schema, nullptr, &gerror,
            "service", service_name_.c_str(),
            "key", key_str.c_str(),
            nullptr);

        if (gerror) {
            error = gerror->message;
            g_error_free(gerror);
            return;
        }
        if (!password) {
            error = "secret not found: " + key_str;
            return;
        }
        result = password;
        secret_password_free(password);
    };

    // Post blocking D-Bus call to the executor
    co_await boost::asio::post(exec, boost::cobalt::use_op);
    do_lookup();

    if (!error.empty())
        co_return std::unexpected(std::move(error));
    co_return std::move(result);
}

} // namespace secret_provider
