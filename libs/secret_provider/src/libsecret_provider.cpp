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
    // Note: we deliberately do **not** wrap the libsecret call in a
    // `co_await boost::asio::post(exec, boost::cobalt::use_op)` "yield"
    // here. That pattern, on cobalt 1.89, corrupts the asio executor's
    // per-thread allocator pool — a hash table allocated by glib inside
    // `secret_password_lookup_sync` ends up colliding with a slot that
    // asio still considers cached, so the next executor function
    // allocation writes through a stale pointer. Verified with valgrind:
    //
    //   "Invalid write of size 1" in `thread_info_base::allocate`,
    //   freed-by stack: `g_hash_table_unref → secret_password_lookup_sync`.
    //
    // We're already on the cobalt executor thread when this coroutine
    // runs, so the post-and-yield was redundant. Just call the blocking
    // libsecret API directly.

    std::string key_str(key);
    GError* gerror = nullptr;
    gchar* password = secret_password_lookup_sync(
        &schema, nullptr, &gerror,
        "service", service_name_.c_str(),
        "key", key_str.c_str(),
        nullptr);

    if (gerror) {
        std::string err = gerror->message;
        g_error_free(gerror);
        co_return std::unexpected(std::move(err));
    }
    if (!password) {
        co_return std::unexpected("secret not found: " + key_str);
    }
    std::string result(password);
    secret_password_free(password);
    co_return result;
}

} // namespace secret_provider
