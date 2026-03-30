#include <binapi2/umf/client.hpp>

#include <binapi2/umf/rest/generated_endpoints.hpp>
#include <binapi2/umf/rest/account.hpp>
#include <binapi2/umf/rest/market_data.hpp>
#include <binapi2/umf/rest/trade.hpp>
#include <binapi2/umf/rest/user_data_streams.hpp>

namespace binapi2::umf::rest {

account_service::account_service(binapi2::umf::client &owner) noexcept
    : owner_(owner) {}

result<types::account_information> account_service::account_information() {
    return owner_.execute<types::account_information>(
        account_information_endpoint.method, std::string{account_information_endpoint.path}, {}, account_information_endpoint.signed_request);
}

result<std::vector<types::futures_account_balance>> account_service::balances() {
    return owner_.execute<std::vector<types::futures_account_balance>>(
        account_balances_endpoint.method, std::string{account_balances_endpoint.path}, {}, account_balances_endpoint.signed_request);
}

result<std::vector<types::position_risk>> account_service::position_risk(const types::position_risk_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<std::vector<types::position_risk>>(
        position_risk_endpoint.method, std::string{position_risk_endpoint.path}, query, position_risk_endpoint.signed_request);
}

market_data_service::market_data_service(binapi2::umf::client &owner) noexcept
    : owner_(owner) {}

result<types::empty_response> market_data_service::ping() {
    return owner_.execute<types::empty_response>(ping_endpoint.method, std::string{ping_endpoint.path}, {}, ping_endpoint.signed_request);
}

result<types::server_time_response> market_data_service::server_time() {
    return owner_.execute<types::server_time_response>(server_time_endpoint.method, std::string{server_time_endpoint.path}, {}, server_time_endpoint.signed_request);
}

result<types::exchange_info_response> market_data_service::exchange_info(const types::exchange_info_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::exchange_info_response>(exchange_info_endpoint.method, std::string{exchange_info_endpoint.path}, query, exchange_info_endpoint.signed_request);
}

result<types::order_book_response> market_data_service::order_book(const types::order_book_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<types::order_book_response>(order_book_endpoint.method, std::string{order_book_endpoint.path}, query, order_book_endpoint.signed_request);
}

result<std::vector<types::recent_trade>> market_data_service::recent_trades(const types::recent_trades_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::recent_trade>>(recent_trades_endpoint.method, std::string{recent_trades_endpoint.path}, query, recent_trades_endpoint.signed_request);
}

result<std::vector<types::aggregate_trade>> market_data_service::aggregate_trades(const types::aggregate_trades_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.fromId) {
        query["fromId"] = std::to_string(*request.fromId);
    }
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::aggregate_trade>>(
        aggregate_trades_endpoint.method, std::string{aggregate_trades_endpoint.path}, query, aggregate_trades_endpoint.signed_request);
}

result<std::vector<types::kline>> market_data_service::klines(const types::kline_request &request) {
    query_map query{{"symbol", request.symbol}, {"interval", to_string(request.interval)}};
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(klines_endpoint.method, std::string{klines_endpoint.path}, query, klines_endpoint.signed_request);
}

result<std::vector<types::kline>> market_data_service::continuous_klines(const types::continuous_kline_request &request) {
    query_map query{{"pair", request.pair}, {"contractType", to_string(request.contractType)}, {"interval", to_string(request.interval)}};
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(
        continuous_klines_endpoint.method, std::string{continuous_klines_endpoint.path}, query, continuous_klines_endpoint.signed_request);
}

result<std::vector<types::kline>> market_data_service::index_price_klines(const types::index_price_kline_request &request) {
    query_map query{{"pair", request.pair}, {"interval", to_string(request.interval)}};
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(
        index_price_klines_endpoint.method, std::string{index_price_klines_endpoint.path}, query, index_price_klines_endpoint.signed_request);
}

result<std::vector<types::kline>> market_data_service::mark_price_klines(const types::kline_request &request) {
    query_map query{{"symbol", request.symbol}, {"interval", to_string(request.interval)}};
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(
        mark_price_klines_endpoint.method, std::string{mark_price_klines_endpoint.path}, query, mark_price_klines_endpoint.signed_request);
}

result<std::vector<types::kline>> market_data_service::premium_index_klines(const types::kline_request &request) {
    query_map query{{"symbol", request.symbol}, {"interval", to_string(request.interval)}};
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    return owner_.execute<std::vector<types::kline>>(
        premium_index_klines_endpoint.method, std::string{premium_index_klines_endpoint.path}, query, premium_index_klines_endpoint.signed_request);
}

result<types::book_ticker> market_data_service::book_ticker(const types::book_ticker_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::book_ticker>(book_ticker_endpoint.method, std::string{book_ticker_endpoint.path}, query, book_ticker_endpoint.signed_request);
}

result<std::vector<types::book_ticker>> market_data_service::book_tickers() {
    return owner_.execute<std::vector<types::book_ticker>>(book_ticker_endpoint.method, std::string{book_ticker_endpoint.path}, {}, book_ticker_endpoint.signed_request);
}

result<types::price_ticker> market_data_service::price_ticker(const types::price_ticker_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::price_ticker>(price_ticker_endpoint.method, std::string{price_ticker_endpoint.path}, query, price_ticker_endpoint.signed_request);
}

result<std::vector<types::price_ticker>> market_data_service::price_tickers() {
    return owner_.execute<std::vector<types::price_ticker>>(price_ticker_endpoint.method, std::string{price_ticker_endpoint.path}, {}, price_ticker_endpoint.signed_request);
}

result<types::ticker_24hr> market_data_service::ticker_24hr(const types::ticker_24hr_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::ticker_24hr>(ticker_24hr_endpoint.method, std::string{ticker_24hr_endpoint.path}, query, ticker_24hr_endpoint.signed_request);
}

result<std::vector<types::ticker_24hr>> market_data_service::ticker_24hrs() {
    return owner_.execute<std::vector<types::ticker_24hr>>(ticker_24hr_endpoint.method, std::string{ticker_24hr_endpoint.path}, {}, ticker_24hr_endpoint.signed_request);
}

result<types::mark_price> market_data_service::mark_price(const types::mark_price_request &request) {
    query_map query;
    if (request.symbol) {
        query["symbol"] = *request.symbol;
    }
    return owner_.execute<types::mark_price>(mark_price_endpoint.method, std::string{mark_price_endpoint.path}, query, mark_price_endpoint.signed_request);
}

result<std::vector<types::mark_price>> market_data_service::mark_prices() {
    return owner_.execute<std::vector<types::mark_price>>(mark_price_endpoint.method, std::string{mark_price_endpoint.path}, {}, mark_price_endpoint.signed_request);
}

result<types::open_interest> market_data_service::open_interest(const types::open_interest_request &request) {
    return owner_.execute<types::open_interest>(
        open_interest_endpoint.method, std::string{open_interest_endpoint.path}, {{"symbol", request.symbol}}, open_interest_endpoint.signed_request);
}

result<std::vector<types::recent_trade>> market_data_service::historical_trades(const types::historical_trades_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    if (request.fromId) {
        query["fromId"] = std::to_string(*request.fromId);
    }
    return owner_.execute<std::vector<types::recent_trade>>(
        historical_trades_endpoint.method, std::string{historical_trades_endpoint.path}, query, historical_trades_endpoint.signed_request);
}

trade_service::trade_service(binapi2::umf::client &owner) noexcept
    : owner_(owner) {}

result<types::order_response> trade_service::new_order(const types::new_order_request &request) {
    query_map query{{"symbol", request.symbol}, {"side", to_string(request.side)}, {"type", to_string(request.type)}, {"quantity", request.quantity}};
    if (request.timeInForce) query["timeInForce"] = to_string(*request.timeInForce);
    if (request.price) query["price"] = *request.price;
    if (request.newClientOrderId) query["newClientOrderId"] = *request.newClientOrderId;
    if (request.stopPrice) query["stopPrice"] = *request.stopPrice;
    return owner_.execute<types::order_response>(new_order_endpoint.method, std::string{new_order_endpoint.path}, query, new_order_endpoint.signed_request);
}

result<types::order_response> trade_service::modify_order(const types::modify_order_request &request) {
    query_map query{
        {"symbol", request.symbol},
        {"side", to_string(request.side)},
        {"quantity", request.quantity},
        {"price", request.price},
    };
    if (request.orderId) query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId) query["origClientOrderId"] = *request.origClientOrderId;
    if (request.priceMatch) query["priceMatch"] = *request.priceMatch;
    return owner_.execute<types::order_response>(modify_order_endpoint.method, std::string{modify_order_endpoint.path}, query, modify_order_endpoint.signed_request);
}

result<types::order_response> trade_service::cancel_order(const types::cancel_order_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.orderId) query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId) query["origClientOrderId"] = *request.origClientOrderId;
    return owner_.execute<types::order_response>(cancel_order_endpoint.method, std::string{cancel_order_endpoint.path}, query, cancel_order_endpoint.signed_request);
}

result<types::order_response> trade_service::query_order(const types::query_order_request &request) {
    query_map query{{"symbol", request.symbol}};
    if (request.orderId) query["orderId"] = std::to_string(*request.orderId);
    if (request.origClientOrderId) query["origClientOrderId"] = *request.origClientOrderId;
    return owner_.execute<types::order_response>(query_order_endpoint.method, std::string{query_order_endpoint.path}, query, query_order_endpoint.signed_request);
}

user_data_stream_service::user_data_stream_service(binapi2::umf::client &owner) noexcept
    : owner_(owner) {}

result<types::listen_key_response> user_data_stream_service::start() {
    return owner_.execute<types::listen_key_response>(start_listen_key_endpoint.method, std::string{start_listen_key_endpoint.path}, {}, start_listen_key_endpoint.signed_request);
}

result<types::listen_key_response> user_data_stream_service::keepalive() {
    return owner_.execute<types::listen_key_response>(keepalive_listen_key_endpoint.method, std::string{keepalive_listen_key_endpoint.path}, {}, keepalive_listen_key_endpoint.signed_request);
}

result<types::empty_response> user_data_stream_service::close() {
    return owner_.execute<types::empty_response>(close_listen_key_endpoint.method, std::string{close_listen_key_endpoint.path}, {}, close_listen_key_endpoint.signed_request);
}

} // namespace binapi2::umf::rest
