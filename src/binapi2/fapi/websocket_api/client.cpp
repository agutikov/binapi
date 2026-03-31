// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include <binapi2/fapi/websocket_api/client.hpp>

#include <binapi2/fapi/signing.hpp>
#include <binapi2/fapi/time.hpp>
#include <binapi2/fapi/websocket_api/generated_methods.hpp>

#include <glaze/glaze.hpp>

#include <boost/asio/post.hpp>

namespace binapi2::fapi::websocket_api {

namespace {

template<typename Params>
struct wire_request
{
    std::string id{};
    std::string method{};
    Params params{};
};

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

template<>
struct glz::meta<binapi2::fapi::websocket_api::session_logon_wire_request>
{
    using T = binapi2::fapi::websocket_api::session_logon_wire_request;
    static constexpr auto value = object("id", &T::id, "method", &T::method, "params", &T::params);
};

namespace binapi2::fapi::websocket_api {

namespace {

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

types::websocket_api_signed_request
make_signed_request_base(const config& cfg)
{
    query_map auth_query;
    inject_auth_query(auth_query, cfg.recv_window, current_timestamp_ms());
    auth_query["apiKey"] = cfg.api_key;

    const auto canonical = build_query_string(auth_query);
    return {
        .apiKey = cfg.api_key,
        .timestamp = std::stoull(auth_query["timestamp"]),
        .recvWindow = std::stoull(auth_query["recvWindow"]),
        .signature = hmac_sha256_hex(cfg.secret_key, canonical),
    };
}

template<typename Response, typename Params>
result<types::websocket_api_response<Response>>
send_rpc(transport::websocket_client& transport, const std::string& id, std::string_view method, const Params& params)
{
    auto payload = encode_rpc_request(id, method, params);
    if (!payload) {
        return result<types::websocket_api_response<Response>>::failure(payload.err);
    }

    auto write_result = transport.write_text(*payload);
    if (!write_result) {
        return result<types::websocket_api_response<Response>>::failure(write_result.err);
    }

    auto raw = transport.read_text();
    if (!raw) {
        return result<types::websocket_api_response<Response>>::failure(raw.err);
    }

    return decode_rpc_response<Response>(*raw);
}

} // namespace

client::client(boost::asio::io_context& io_context, config cfg) :
    io_context_(io_context), cfg_(std::move(cfg)), transport_(io_context_, cfg_)
{
}

result<void>
client::connect()
{
    return transport_.connect(cfg_.websocket_api_host, cfg_.websocket_api_port, cfg_.websocket_api_target);
}

void
client::connect(callback_type<void> callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(connect()); });
}

std::string
client::next_id()
{
    return std::to_string(++id_counter_);
}

result<types::websocket_api_response<types::session_logon_result>>
client::session_logon()
{
    query_map auth_query;
    inject_auth_query(auth_query, cfg_.recv_window, current_timestamp_ms());
    auth_query["apiKey"] = cfg_.api_key;

    const auto canonical = build_query_string(auth_query);
    const auto signature = hmac_sha256_hex(cfg_.secret_key, canonical);

    session_logon_wire_request request{
        .id = next_id(),
        .method = std::string{session_logon_method},
        .params = {
            .apiKey = cfg_.api_key,
            .timestamp = std::stoull(auth_query["timestamp"]),
            .recvWindow = std::stoull(auth_query["recvWindow"]),
            .signature = signature,
        },
    };

    return send_rpc<types::session_logon_result>(transport_, request.id, session_logon_method, request.params);
}

void
client::session_logon(callback_type<types::websocket_api_response<types::session_logon_result>> callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(session_logon()); });
}

result<types::websocket_api_response<types::account_information>>
client::account_status()
{
    return send_rpc<types::account_information>(transport_, next_id(), account_status_method, make_signed_request_base(cfg_));
}

void
client::account_status(callback_type<types::websocket_api_response<types::account_information>> callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(account_status()); });
}

result<types::websocket_api_response<std::vector<types::futures_account_balance>>>
client::account_balance()
{
    return send_rpc<std::vector<types::futures_account_balance>>(
        transport_, next_id(), account_balance_method, make_signed_request_base(cfg_));
}

void
client::account_balance(callback_type<types::websocket_api_response<std::vector<types::futures_account_balance>>> callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(account_balance()); });
}

result<types::websocket_api_response<types::order_response>>
client::new_order(const types::new_order_request& request)
{
    auto auth = make_signed_request_base(cfg_);
    types::websocket_api_order_place_request params{};
    params.apiKey = auth.apiKey;
    params.timestamp = auth.timestamp;
    params.recvWindow = auth.recvWindow;
    params.signature = auth.signature;
    params.symbol = request.symbol;
    params.side = to_string(request.side);
    params.type = to_string(request.type);
    params.timeInForce = request.timeInForce ? std::optional<std::string>{ to_string(*request.timeInForce) } : std::nullopt;
    params.quantity = request.quantity;
    params.price = request.price;
    params.newClientOrderId = request.newClientOrderId;
    params.stopPrice = request.stopPrice;
    return send_rpc<types::order_response>(transport_, next_id(), order_place_method, params);
}

void
client::new_order(const types::new_order_request& request,
                  callback_type<types::websocket_api_response<types::order_response>> callback)
{
    boost::asio::post(io_context_, [this, request, callback = std::move(callback)]() mutable { callback(new_order(request)); });
}

result<types::websocket_api_response<types::order_response>>
client::query_order(const types::query_order_request& request)
{
    auto auth = make_signed_request_base(cfg_);
    types::websocket_api_order_query_request params{};
    params.apiKey = auth.apiKey;
    params.timestamp = auth.timestamp;
    params.recvWindow = auth.recvWindow;
    params.signature = auth.signature;
    params.symbol = request.symbol;
    params.orderId = request.orderId;
    params.origClientOrderId = request.origClientOrderId;
    return send_rpc<types::order_response>(transport_, next_id(), order_status_method, params);
}

void
client::query_order(const types::query_order_request& request,
                    callback_type<types::websocket_api_response<types::order_response>> callback)
{
    boost::asio::post(io_context_,
                      [this, request, callback = std::move(callback)]() mutable { callback(query_order(request)); });
}

result<types::websocket_api_response<types::order_response>>
client::cancel_order(const types::cancel_order_request& request)
{
    auto auth = make_signed_request_base(cfg_);
    types::websocket_api_order_cancel_request params{};
    params.apiKey = auth.apiKey;
    params.timestamp = auth.timestamp;
    params.recvWindow = auth.recvWindow;
    params.signature = auth.signature;
    params.symbol = request.symbol;
    params.orderId = request.orderId;
    params.origClientOrderId = request.origClientOrderId;
    return send_rpc<types::order_response>(transport_, next_id(), order_cancel_method, params);
}

void
client::cancel_order(const types::cancel_order_request& request,
                     callback_type<types::websocket_api_response<types::order_response>> callback)
{
    boost::asio::post(io_context_,
                      [this, request, callback = std::move(callback)]() mutable { callback(cancel_order(request)); });
}

result<types::websocket_api_response<types::book_ticker>>
client::book_ticker(const types::book_ticker_request& request)
{
    return send_rpc<types::book_ticker>(
        transport_, next_id(), ticker_book_method, types::websocket_api_book_ticker_request{ request.symbol });
}

void
client::book_ticker(const types::book_ticker_request& request,
                    callback_type<types::websocket_api_response<types::book_ticker>> callback)
{
    boost::asio::post(io_context_,
                      [this, request, callback = std::move(callback)]() mutable { callback(book_ticker(request)); });
}

result<void>
client::close()
{
    return transport_.close();
}

void
client::close(callback_type<void> callback)
{
    boost::asio::post(io_context_, [this, callback = std::move(callback)]() mutable { callback(close()); });
}

} // namespace binapi2::fapi::websocket_api
