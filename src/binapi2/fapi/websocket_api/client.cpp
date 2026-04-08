// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements the WebSocket API client, which uses a persistent WebSocket
/// connection for JSON-RPC-style request/response communication. Each request
/// is serialized as a wire_request envelope (id + method + params), sent as a
/// text frame, and the response is read synchronously from the same connection.
/// Authentication is handled by injecting apiKey, timestamp, recvWindow, and an
/// HMAC-SHA256 signature into the params of signed requests. Explicit template
/// instantiations at the bottom ensure that all supported request types are
/// compiled into the shared library.

#include <binapi2/fapi/websocket_api/client.hpp>

#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>

#include <glaze/glaze.hpp>

#include <boost/asio/post.hpp>

namespace binapi2::fapi::websocket_api {

namespace {

// Generic JSON-RPC envelope for outgoing WebSocket API requests. The id field
// is a monotonically increasing counter that allows correlating responses, and
// method identifies the Binance endpoint (e.g. "v2/ticker.book").
template<typename Params>
struct wire_request
{
    std::string id{};
    std::string method{};
    Params params{};
};

// Separate wire type for session.logon because its params type
// (session_logon_request) has a different structure than the generic
// signed_request base used by other endpoints.
struct session_logon_wire_request
{
    std::string id{};
    std::string method{};
    types::session_logon_request params{};
};

} // namespace

} // namespace binapi2::fapi::websocket_api

template<typename T>
struct glz::meta<binapi2::fapi::websocket_api::wire_request<T>>
{
    using U = binapi2::fapi::websocket_api::wire_request<T>;
    static constexpr auto value = object("id", &U::id, "method", &U::method, "params", &U::params);
};

// glz::meta specializations must be defined in the global/glz namespace and
// outside the binapi2 namespace, so the namespace is temporarily closed above
// and reopened below. This is a glaze requirement for custom serialization.
template<>
struct glz::meta<binapi2::fapi::websocket_api::session_logon_wire_request>
{
    using T = binapi2::fapi::websocket_api::session_logon_wire_request;
    static constexpr auto value = object("id", &T::id, "method", &T::method, "params", &T::params);
};

namespace binapi2::fapi::websocket_api {

namespace {

// Deserializes a raw JSON WebSocket frame into a typed RPC response. Handles
// two failure modes: JSON parse errors (malformed data) and Binance-level
// errors (valid JSON but the response contains an error object with a code
// and message).
template<typename Response>
result<types::websocket_api_response<Response>>
decode_rpc_response(const std::string& raw)
{
    types::websocket_api_response<Response> response{};
    if (auto ec = glz::read_json(response, raw)) {
        return result<types::websocket_api_response<Response>>::failure(
            { error_code::json, 0, 0, glz::format_error(ec, raw), raw });
    }

    if (response.error) {
        return result<types::websocket_api_response<Response>>::failure(
            { error_code::binance, response.status, response.error->code, response.error->msg, raw });
    }

    return result<types::websocket_api_response<Response>>::success(std::move(response));
}

template<typename Params>
result<std::string>
encode_rpc_request(const std::string& id, std::string_view method, const Params& params)
{
    wire_request<Params> request{ id, std::string{ method }, params };
    auto payload = glz::write_json(request);
    if (!payload) {
        return result<std::string>::failure({ error_code::json, 0, 0, glz::format_error(payload.error(), ""), {} });
    }
    return result<std::string>::success(std::move(*payload));
}

} // namespace

client::client(boost::asio::io_context& io_context, config cfg) :
    io_context_(io_context), cfg_(std::move(cfg)), transport_(io_context_, cfg_)
{
}

std::string
client::next_id()
{
    return std::to_string(++id_counter_);
}

types::websocket_api_signed_request
client::make_signed_request_base()
{
    query_map auth_query;
    inject_auth_query(auth_query, cfg_.recv_window, current_timestamp_ms());
    auth_query["apiKey"] = cfg_.api_key;

    const auto canonical = build_query_string(auth_query);
    return {
        .apiKey = cfg_.api_key,
        .timestamp = std::stoull(auth_query["timestamp"]),
        .recvWindow = std::stoull(auth_query["recvWindow"]),
        .signature = hmac_sha256_hex(cfg_.secret_key, canonical),
    };
}

template<typename Response, typename Params>
result<types::websocket_api_response<Response>>
client::send_rpc(std::string_view method, const Params& params)
{
    auto payload = encode_rpc_request(next_id(), method, params);
    if (!payload) {
        return result<types::websocket_api_response<Response>>::failure(payload.err);
    }

    auto write_result = transport_.write_text(*payload);
    if (!write_result) {
        return result<types::websocket_api_response<Response>>::failure(write_result.err);
    }

    auto raw = transport_.read_text();
    if (!raw) {
        return result<types::websocket_api_response<Response>>::failure(raw.err);
    }

    return decode_rpc_response<Response>(*raw);
}

// Conditionally injects authentication fields into a request. Uses
// if-constexpr to check at compile time whether the request type inherits
// from websocket_api_signed_request; unsigned request types pass through
// unmodified.
template<class Request>
Request
client::inject_auth(const Request& request)
{
    if constexpr (std::is_base_of_v<types::websocket_api_signed_request, Request>) {
        auto auth = make_signed_request_base();
        Request authed = request;
        authed.apiKey = auth.apiKey;
        authed.timestamp = auth.timestamp;
        authed.recvWindow = auth.recvWindow;
        authed.signature = auth.signature;
        return authed;
    }
    else {
        return request;
    }
}

// --- Generic execute ---

// Trait-driven dispatch: endpoint_traits<Request> maps each request type to
// its method string and response type at compile time, so a single execute
// template handles all WebSocket API endpoints uniformly.
template<class Request>
    requires has_ws_endpoint_traits<Request>
auto
client::execute(const Request& request)
    -> result<types::websocket_api_response<typename endpoint_traits<Request>::response_type_t>>
{
    using traits = endpoint_traits<Request>;
    return send_rpc<typename traits::response_type_t>(traits::method, inject_auth(request));
}

template<class Request>
    requires has_ws_endpoint_traits<Request>
auto
client::async_execute(const Request& request)
    -> boost::cobalt::task<result<types::websocket_api_response<typename endpoint_traits<Request>::response_type_t>>>
{
    co_return execute(request);
}

// Explicit instantiations for all trait-enabled request types.
template auto client::execute(const types::websocket_api_book_ticker_request&)
    -> result<types::websocket_api_response<types::book_ticker>>;
template auto client::execute(const types::websocket_api_price_ticker_request&)
    -> result<types::websocket_api_response<types::price_ticker>>;
template auto client::execute(const types::websocket_api_order_place_request&)
    -> result<types::websocket_api_response<types::order_response>>;
template auto client::execute(const types::websocket_api_order_query_request&)
    -> result<types::websocket_api_response<types::order_response>>;
template auto client::execute(const types::websocket_api_order_cancel_request&)
    -> result<types::websocket_api_response<types::order_response>>;
template auto client::execute(const types::websocket_api_order_modify_request&)
    -> result<types::websocket_api_response<types::order_response>>;
template auto client::execute(const types::websocket_api_position_request&)
    -> result<types::websocket_api_response<std::vector<types::position_risk>>>;
template auto client::execute(const types::websocket_api_algo_order_place_request&)
    -> result<types::websocket_api_response<types::algo_order_response>>;
template auto client::execute(const types::websocket_api_algo_order_cancel_request&)
    -> result<types::websocket_api_response<types::code_msg_response>>;

template auto client::async_execute(const types::websocket_api_book_ticker_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::book_ticker>>>;
template auto client::async_execute(const types::websocket_api_price_ticker_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::price_ticker>>>;
template auto client::async_execute(const types::websocket_api_order_place_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::order_response>>>;
template auto client::async_execute(const types::websocket_api_order_query_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::order_response>>>;
template auto client::async_execute(const types::websocket_api_order_cancel_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::order_response>>>;
template auto client::async_execute(const types::websocket_api_order_modify_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::order_response>>>;
template auto client::async_execute(const types::websocket_api_position_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<std::vector<types::position_risk>>>>;
template auto client::async_execute(const types::websocket_api_algo_order_place_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::algo_order_response>>>;
template auto client::async_execute(const types::websocket_api_algo_order_cancel_request&)
    -> boost::cobalt::task<result<types::websocket_api_response<types::code_msg_response>>>;

// --- Connection ---

result<void>
client::connect()
{
    return transport_.connect(cfg_.websocket_api_host, cfg_.websocket_api_port, cfg_.websocket_api_target);
}

boost::cobalt::task<result<void>>
client::async_connect()
{
    co_return co_await transport_.async_connect(cfg_.websocket_api_host, cfg_.websocket_api_port, cfg_.websocket_api_target);
}

result<void>
client::close()
{
    return transport_.close();
}

boost::cobalt::task<result<void>>
client::async_close()
{
    co_return co_await transport_.async_close();
}

// --- Session logon (unique auth flow) ---

// session.logon has a unique auth flow: it builds the signature from a
// query map containing apiKey, recvWindow, and timestamp (in sorted order),
// rather than embedding auth fields into a generic signed_request struct.
// This is because the logon request predates the generic auth pattern and
// the server expects the params in a specific shape.
result<types::websocket_api_response<types::session_logon_result>>
client::session_logon()
{
    query_map auth_query;
    inject_auth_query(auth_query, cfg_.recv_window, current_timestamp_ms());
    auth_query["apiKey"] = cfg_.api_key;

    const auto canonical = build_query_string(auth_query);
    const auto signature = hmac_sha256_hex(cfg_.secret_key, canonical);

    types::session_logon_request params{
        .apiKey = cfg_.api_key,
        .timestamp = std::stoull(auth_query["timestamp"]),
        .recvWindow = std::stoull(auth_query["recvWindow"]),
        .signature = signature,
    };

    return send_rpc<types::session_logon_result>(session_logon_method, params);
}

boost::cobalt::task<result<types::websocket_api_response<types::session_logon_result>>>
client::async_session_logon()
{
    co_return session_logon();
}

// --- Parameterless signed endpoints ---

result<types::websocket_api_response<types::account_information>>
client::account_status()
{
    return send_rpc<types::account_information>(account_status_method, make_signed_request_base());
}

boost::cobalt::task<result<types::websocket_api_response<types::account_information>>>
client::async_account_status()
{
    co_return account_status();
}

result<types::websocket_api_response<types::account_information>>
client::account_status_v2()
{
    return send_rpc<types::account_information>(account_status_v2_method, make_signed_request_base());
}

boost::cobalt::task<result<types::websocket_api_response<types::account_information>>>
client::async_account_status_v2()
{
    co_return account_status_v2();
}

result<types::websocket_api_response<std::vector<types::futures_account_balance>>>
client::account_balance()
{
    return send_rpc<std::vector<types::futures_account_balance>>(account_balance_method, make_signed_request_base());
}

boost::cobalt::task<result<types::websocket_api_response<std::vector<types::futures_account_balance>>>>
client::async_account_balance()
{
    co_return account_balance();
}

// --- Shared: position v2 ---

result<types::websocket_api_response<std::vector<types::position_risk>>>
client::account_position_v2(const types::websocket_api_position_request& request)
{
    return send_rpc<std::vector<types::position_risk>>(account_position_v2_method, inject_auth(request));
}

boost::cobalt::task<result<types::websocket_api_response<std::vector<types::position_risk>>>>
client::async_account_position_v2(const types::websocket_api_position_request& request)
{
    co_return account_position_v2(request);
}

// --- Shared: user data stream ---

result<types::websocket_api_response<types::websocket_api_listen_key_result>>
client::user_data_stream_start()
{
    types::websocket_api_user_data_stream_request params{ .apiKey = cfg_.api_key };
    return send_rpc<types::websocket_api_listen_key_result>(user_data_stream_start_method, params);
}

boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>>
client::async_user_data_stream_start()
{
    co_return user_data_stream_start();
}

result<types::websocket_api_response<types::websocket_api_listen_key_result>>
client::user_data_stream_ping()
{
    types::websocket_api_user_data_stream_request params{ .apiKey = cfg_.api_key };
    return send_rpc<types::websocket_api_listen_key_result>(user_data_stream_ping_method, params);
}

boost::cobalt::task<result<types::websocket_api_response<types::websocket_api_listen_key_result>>>
client::async_user_data_stream_ping()
{
    co_return user_data_stream_ping();
}

result<types::websocket_api_response<types::empty_response_t>>
client::user_data_stream_stop()
{
    types::websocket_api_user_data_stream_request params{ .apiKey = cfg_.api_key };
    return send_rpc<types::empty_response_t>(user_data_stream_stop_method, params);
}

boost::cobalt::task<result<types::websocket_api_response<types::empty_response_t>>>
client::async_user_data_stream_stop()
{
    co_return user_data_stream_stop();
}

} // namespace binapi2::fapi::websocket_api
