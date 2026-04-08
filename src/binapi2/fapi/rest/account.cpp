// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file Implements account REST endpoints. Most are parameterless signed
/// queries. The download_id / download_link methods follow a shared two-step
/// pattern: first request a download_id (with a date range), then poll for
/// the download_link using that ID. This pattern is repeated for transactions,
/// orders, and trades, each hitting a different endpoint but using the same
/// request/response types.

#include <binapi2/fapi/rest/account.hpp>

#include <binapi2/fapi/client.hpp>
#include <binapi2/fapi/rest/generated_endpoints.hpp>

#include <boost/cobalt/task.hpp>

namespace binapi2::fapi::rest {

result<types::account_information_t>
account_service::account_information_t()
{
    return owner_.execute<types::account_information_t>(account_information_endpoint.method,
                                                      std::string{ account_information_endpoint.path },
                                                      {},
                                                      account_information_endpoint.signed_request);
}

boost::cobalt::task<result<types::account_information_t>>
account_service::async_account_information()
{
    co_return co_await owner_.async_execute<types::account_information_t>(account_information_endpoint.method,
                                                      std::string{ account_information_endpoint.path },
                                                      {},
                                                      account_information_endpoint.signed_request);
}

result<std::vector<types::futures_account_balance_t>>
account_service::balances()
{
    return owner_.execute<std::vector<types::futures_account_balance_t>>(account_balances_endpoint.method,
                                                                       std::string{ account_balances_endpoint.path },
                                                                       {},
                                                                       account_balances_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::futures_account_balance_t>>>
account_service::async_balances()
{
    co_return co_await owner_.async_execute<std::vector<types::futures_account_balance_t>>(account_balances_endpoint.method,
                                                                       std::string{ account_balances_endpoint.path },
                                                                       {},
                                                                       account_balances_endpoint.signed_request);
}

result<types::account_config_response_t>
account_service::account_config()
{
    return owner_.execute<types::account_config_response_t>(account_config_endpoint.method,
                                                          std::string{ account_config_endpoint.path },
                                                          {},
                                                          account_config_endpoint.signed_request);
}

boost::cobalt::task<result<types::account_config_response_t>>
account_service::async_account_config()
{
    co_return co_await owner_.async_execute<types::account_config_response_t>(account_config_endpoint.method,
                                                          std::string{ account_config_endpoint.path },
                                                          {},
                                                          account_config_endpoint.signed_request);
}

result<types::multi_assets_mode_response_t>
account_service::get_multi_assets_mode()
{
    return owner_.execute<types::multi_assets_mode_response_t>(get_multi_assets_mode_endpoint.method,
                                                             std::string{ get_multi_assets_mode_endpoint.path },
                                                             {},
                                                             get_multi_assets_mode_endpoint.signed_request);
}

boost::cobalt::task<result<types::multi_assets_mode_response_t>>
account_service::async_get_multi_assets_mode()
{
    co_return co_await owner_.async_execute<types::multi_assets_mode_response_t>(get_multi_assets_mode_endpoint.method,
                                                             std::string{ get_multi_assets_mode_endpoint.path },
                                                             {},
                                                             get_multi_assets_mode_endpoint.signed_request);
}

result<types::position_mode_response_t>
account_service::get_position_mode()
{
    return owner_.execute<types::position_mode_response_t>(get_position_mode_endpoint.method,
                                                         std::string{ get_position_mode_endpoint.path },
                                                         {},
                                                         get_position_mode_endpoint.signed_request);
}

boost::cobalt::task<result<types::position_mode_response_t>>
account_service::async_get_position_mode()
{
    co_return co_await owner_.async_execute<types::position_mode_response_t>(get_position_mode_endpoint.method,
                                                         std::string{ get_position_mode_endpoint.path },
                                                         {},
                                                         get_position_mode_endpoint.signed_request);
}

result<std::vector<types::rate_limit_t>>
account_service::rate_limit_order()
{
    return owner_.execute<std::vector<types::rate_limit_t>>(rate_limit_order_endpoint.method,
                                                          std::string{ rate_limit_order_endpoint.path },
                                                          {},
                                                          rate_limit_order_endpoint.signed_request);
}

boost::cobalt::task<result<std::vector<types::rate_limit_t>>>
account_service::async_rate_limit_order()
{
    co_return co_await owner_.async_execute<std::vector<types::rate_limit_t>>(rate_limit_order_endpoint.method,
                                                          std::string{ rate_limit_order_endpoint.path },
                                                          {},
                                                          rate_limit_order_endpoint.signed_request);
}

result<types::bnb_burn_status_response_t>
account_service::get_bnb_burn()
{
    return owner_.execute<types::bnb_burn_status_response_t>(get_bnb_burn_endpoint.method,
                                                           std::string{ get_bnb_burn_endpoint.path },
                                                           {},
                                                           get_bnb_burn_endpoint.signed_request);
}

boost::cobalt::task<result<types::bnb_burn_status_response_t>>
account_service::async_get_bnb_burn()
{
    co_return co_await owner_.async_execute<types::bnb_burn_status_response_t>(get_bnb_burn_endpoint.method,
                                                           std::string{ get_bnb_burn_endpoint.path },
                                                           {},
                                                           get_bnb_burn_endpoint.signed_request);
}

result<types::download_id_response_t>
account_service::download_id_transaction(const types::download_id_request_t& request)
{
    return owner_.execute<types::download_id_response_t>(download_id_transaction_endpoint.method,
                                                       std::string{ download_id_transaction_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_transaction_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_id_response_t>>
account_service::async_download_id_transaction(const types::download_id_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_id_response_t>(download_id_transaction_endpoint.method,
                                                       std::string{ download_id_transaction_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_transaction_endpoint.signed_request);
}

result<types::download_link_response_t>
account_service::download_link_transaction(const types::download_link_request_t& request)
{
    return owner_.execute<types::download_link_response_t>(download_link_transaction_endpoint.method,
                                                         std::string{ download_link_transaction_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_transaction_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_link_response_t>>
account_service::async_download_link_transaction(const types::download_link_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_link_response_t>(download_link_transaction_endpoint.method,
                                                         std::string{ download_link_transaction_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_transaction_endpoint.signed_request);
}

result<types::download_id_response_t>
account_service::download_id_order(const types::download_id_request_t& request)
{
    return owner_.execute<types::download_id_response_t>(download_id_order_endpoint.method,
                                                       std::string{ download_id_order_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_order_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_id_response_t>>
account_service::async_download_id_order(const types::download_id_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_id_response_t>(download_id_order_endpoint.method,
                                                       std::string{ download_id_order_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_order_endpoint.signed_request);
}

result<types::download_link_response_t>
account_service::download_link_order(const types::download_link_request_t& request)
{
    return owner_.execute<types::download_link_response_t>(download_link_order_endpoint.method,
                                                         std::string{ download_link_order_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_order_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_link_response_t>>
account_service::async_download_link_order(const types::download_link_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_link_response_t>(download_link_order_endpoint.method,
                                                         std::string{ download_link_order_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_order_endpoint.signed_request);
}

result<types::download_id_response_t>
account_service::download_id_trade(const types::download_id_request_t& request)
{
    return owner_.execute<types::download_id_response_t>(download_id_trade_endpoint.method,
                                                       std::string{ download_id_trade_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_trade_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_id_response_t>>
account_service::async_download_id_trade(const types::download_id_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_id_response_t>(download_id_trade_endpoint.method,
                                                       std::string{ download_id_trade_endpoint.path },
                                                       to_query_map(request),
                                                       download_id_trade_endpoint.signed_request);
}

result<types::download_link_response_t>
account_service::download_link_trade(const types::download_link_request_t& request)
{
    return owner_.execute<types::download_link_response_t>(download_link_trade_endpoint.method,
                                                         std::string{ download_link_trade_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_trade_endpoint.signed_request);
}

boost::cobalt::task<result<types::download_link_response_t>>
account_service::async_download_link_trade(const types::download_link_request_t& request)
{
    co_return co_await owner_.async_execute<types::download_link_response_t>(download_link_trade_endpoint.method,
                                                         std::string{ download_link_trade_endpoint.path },
                                                         to_query_map(request),
                                                         download_link_trade_endpoint.signed_request);
}

} // namespace binapi2::fapi::rest
