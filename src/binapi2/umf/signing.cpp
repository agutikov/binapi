#include <binapi2/umf/signing.hpp>

#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <array>
#include <cctype>
#include <cstdio>

namespace binapi2::umf {

namespace {

std::string to_hex(const unsigned char *data, std::size_t size) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        out.push_back(hex[(data[i] >> 4) & 0x0f]);
        out.push_back(hex[data[i] & 0x0f]);
    }
    return out;
}

} // namespace

std::string hmac_sha256_hex(const std::string &key, const std::string &data) {
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_size{};
    HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char *>(data.data()), data.size(), digest.data(), &digest_size);
    return to_hex(digest.data(), digest_size);
}

std::string percent_encode(std::string_view value) {
    std::string out;
    for (const unsigned char ch : value) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            out.push_back(static_cast<char>(ch));
        } else {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", ch);
            out.append(buf);
        }
    }
    return out;
}

std::string build_query_string(const query_map &query) {
    std::string out;
    for (const auto &[key, value] : query) {
        if (!out.empty()) {
            out.push_back('&');
        }
        out += percent_encode(key);
        out.push_back('=');
        out += percent_encode(value);
    }
    return out;
}

void inject_auth_query(query_map &query, std::uint64_t recv_window, std::uint64_t timestamp_ms) {
    query["recvWindow"] = std::to_string(recv_window);
    query["timestamp"] = std::to_string(timestamp_ms);
}

void sign_query(query_map &query, const std::string &secret_key) {
    const auto canonical = build_query_string(query);
    query["signature"] = hmac_sha256_hex(secret_key, canonical);
}

} // namespace binapi2::umf
