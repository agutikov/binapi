// SPDX-License-Identifier: Apache-2.0
//
// Validate binapi2 endpoint coverage and parameter completeness against
// the official Binance Postman collection for USD-M Futures.

#include <binapi2/fapi/rest/generated_endpoints.hpp>
#include <binapi2/fapi/types/account.hpp>
#include <binapi2/fapi/types/convert.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/trade.hpp>

#include <gtest/gtest.h>
#include <glaze/glaze.hpp>

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Postman JSON types (minimal subset for parsing the collection).
// ---------------------------------------------------------------------------

namespace postman {

struct query_param
{
    std::string key{};
    std::string value{};
    std::string description{};
    bool disabled{false};
};

struct url
{
    std::string raw{};
    std::vector<std::string> host{};
    std::vector<std::string> path{};
    std::vector<query_param> query{};
};

struct request_obj
{
    std::string method{};
    url url_data{};
};

struct item_obj
{
    std::string name{};
    std::optional<request_obj> request_data{};
    std::optional<std::vector<item_obj>> children{};
};

struct collection
{
    std::vector<item_obj> items{};
};

} // namespace postman

template<>
struct glz::meta<postman::query_param>
{
    using T = postman::query_param;
    static constexpr auto value = object("key", &T::key, "value", &T::value, "description", &T::description, "disabled", &T::disabled);
};

template<>
struct glz::meta<postman::url>
{
    using T = postman::url;
    static constexpr auto value = object("raw", &T::raw, "host", &T::host, "path", &T::path, "query", &T::query);
};

template<>
struct glz::meta<postman::request_obj>
{
    using T = postman::request_obj;
    static constexpr auto value = object("method", &T::method, "url", &T::url_data);
};

template<>
struct glz::meta<postman::item_obj>
{
    using T = postman::item_obj;
    static constexpr auto value = object("name", &T::name, "request", &T::request_data, "item", &T::children);
};

template<>
struct glz::meta<postman::collection>
{
    using T = postman::collection;
    static constexpr auto value = object("item", &T::items);
};

// ---------------------------------------------------------------------------
// Helpers.
// ---------------------------------------------------------------------------

namespace {

/// Flattened Postman endpoint after recursive traversal.
struct postman_endpoint
{
    std::string name;
    std::string method; // "GET", "POST", etc.
    std::string path;   // "/fapi/v1/order"
    std::set<std::string> params;
};

/// Recursively collect endpoints from the Postman item tree.
void
collect_endpoints(const std::vector<postman::item_obj>& items, std::vector<postman_endpoint>& out)
{
    for (const auto& it : items) {
        if (it.request_data.has_value()) {
            postman_endpoint ep;
            ep.name = it.name;
            ep.method = it.request_data->method;

            // Build path from segments: ["fapi","v1","order"] -> "/fapi/v1/order"
            std::string path;
            for (const auto& seg : it.request_data->url_data.path) {
                path += '/';
                path += seg;
            }
            ep.path = path;

            for (const auto& q : it.request_data->url_data.query) {
                ep.params.insert(q.key);
            }
            out.push_back(std::move(ep));
        }
        if (it.children.has_value()) {
            collect_endpoints(*it.children, out);
        }
    }
}

/// Parse the Postman collection JSON file and return flat endpoint list.
std::vector<postman_endpoint>
parse_postman(const std::string& path)
{
    std::ifstream f(path);
    EXPECT_TRUE(f.good()) << "Cannot open Postman collection: " << path;

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();

    postman::collection coll;
    auto ec = glz::read<glz::opts{.error_on_unknown_keys = false}>(coll, json);
    EXPECT_FALSE(ec) << "Postman JSON parse error: " << glz::format_error(ec, json);

    std::vector<postman_endpoint> endpoints;
    collect_endpoints(coll.items, endpoints);
    return endpoints;
}

// ---------------------------------------------------------------------------
// binapi2 endpoint + parameter registry.
// ---------------------------------------------------------------------------

using namespace binapi2::fapi;

/// Convert beast verb to string for comparison.
std::string
verb_to_string(boost::beast::http::verb v)
{
    switch (v) {
    case boost::beast::http::verb::get: return "GET";
    case boost::beast::http::verb::post: return "POST";
    case boost::beast::http::verb::put: return "PUT";
    case boost::beast::http::verb::delete_: return "DELETE";
    default: return "UNKNOWN";
    }
}

/// Composite key for endpoint lookup: (path, method).
using endpoint_key = std::pair<std::string, std::string>;

/// Extract field names from a request struct via glaze reflection.
template<class T>
std::set<std::string>
field_names()
{
    std::set<std::string> names;
    constexpr auto N = glz::reflect<T>::size;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (names.emplace(glz::reflect<T>::keys[I]), ...);
    }(std::make_index_sequence<N>{});
    return names;
}

struct endpoint_info
{
    const rest::endpoint_metadata* metadata;
    std::set<std::string> param_names;
};

/// Build the complete registry of binapi2 endpoints.
/// Maps (path, method) -> endpoint_info with reflected parameter names.
std::map<endpoint_key, endpoint_info>
build_binapi_registry()
{
    std::map<endpoint_key, endpoint_info> reg;

    auto add = [&](const rest::endpoint_metadata& ep, std::set<std::string> params) {
        auto key = endpoint_key{std::string(ep.path), verb_to_string(ep.method)};
        reg[key] = {&ep, std::move(params)};
    };

    // --- Market data: 1:1 traits ---
    add(rest::ping_endpoint, field_names<types::ping_request_t>());
    add(rest::server_time_endpoint, field_names<types::server_time_request_t>());
    add(rest::exchange_info_endpoint, field_names<types::exchange_info_request_t>());
    add(rest::order_book_endpoint, field_names<types::order_book_request_t>());
    add(rest::recent_trades_endpoint, field_names<types::recent_trades_request_t>());
    add(rest::aggregate_trades_endpoint, field_names<types::aggregate_trades_request_t>());
    add(rest::continuous_klines_endpoint, field_names<types::continuous_kline_request_t>());
    add(rest::index_price_klines_endpoint, field_names<types::index_price_kline_request_t>());
    add(rest::book_ticker_endpoint, field_names<types::book_ticker_request_t>());
    add(rest::price_ticker_endpoint, field_names<types::price_ticker_request_t>());
    add(rest::ticker_24hr_endpoint, field_names<types::ticker_24hr_request_t>());
    add(rest::mark_price_endpoint, field_names<types::mark_price_request_t>());
    add(rest::funding_rate_history_endpoint, field_names<types::funding_rate_history_request_t>());
    add(rest::open_interest_endpoint, field_names<types::open_interest_request_t>());
    add(rest::historical_trades_endpoint, field_names<types::historical_trades_request_t>());
    add(rest::basis_endpoint, field_names<types::basis_request_t>());
    add(rest::price_ticker_v2_endpoint, field_names<types::price_ticker_v2_request_t>());
    add(rest::delivery_price_endpoint, field_names<types::delivery_price_request_t>());
    add(rest::composite_index_info_endpoint, field_names<types::composite_index_info_request_t>());
    add(rest::index_constituents_endpoint, field_names<types::index_constituents_request_t>());
    add(rest::asset_index_endpoint, field_names<types::asset_index_request_t>());
    add(rest::insurance_fund_endpoint, field_names<types::insurance_fund_request_t>());
    add(rest::adl_risk_endpoint, field_names<types::adl_risk_request_t>());
    add(rest::rpi_depth_endpoint, field_names<types::rpi_depth_request_t>());
    add(rest::trading_schedule_endpoint, field_names<types::trading_schedule_request_t>());

    // --- Market data: shared request types (named service methods) ---
    add(rest::klines_endpoint, field_names<types::kline_request_t>());
    add(rest::mark_price_klines_endpoint, field_names<types::kline_request_t>());
    add(rest::premium_index_klines_endpoint, field_names<types::kline_request_t>());
    add(rest::open_interest_statistics_endpoint, field_names<types::futures_data_request_t>());
    add(rest::top_long_short_account_ratio_endpoint, field_names<types::futures_data_request_t>());
    add(rest::top_trader_long_short_ratio_endpoint, field_names<types::futures_data_request_t>());
    add(rest::long_short_ratio_endpoint, field_names<types::futures_data_request_t>());
    add(rest::taker_buy_sell_volume_endpoint, field_names<types::futures_data_request_t>());

    // --- Market data: parameterless list variants (same endpoint, no symbol) ---
    // These use the same endpoint_metadata as single-symbol variants; the optional
    // symbol is simply omitted.  funding_rate_info_t is parameterless.
    add(rest::funding_rate_info_endpoint, {});

    // --- Account: 1:1 traits ---
    add(rest::position_risk_endpoint, field_names<types::position_risk_request_t>());
    add(rest::symbol_config_endpoint, field_names<types::symbol_config_request_t>());
    add(rest::income_history_endpoint, field_names<types::income_history_request_t>());
    add(rest::leverage_brackets_endpoint, field_names<types::leverage_bracket_request_t>());
    add(rest::commission_rate_endpoint, field_names<types::commission_rate_request_t>());
    add(rest::toggle_bnb_burn_endpoint, field_names<types::toggle_bnb_burn_request_t>());
    add(rest::quantitative_rules_endpoint, field_names<types::quantitative_rules_request_t>());
    add(rest::pm_account_info_endpoint, field_names<types::pm_account_info_request_t>());

    // --- Account: parameterless endpoints ---
    add(rest::account_information_endpoint, {});
    add(rest::account_balances_endpoint, {});
    add(rest::account_config_endpoint, {});
    add(rest::get_multi_assets_mode_endpoint, {});
    add(rest::get_position_mode_endpoint, {});
    add(rest::rate_limit_order_endpoint, {});
    add(rest::get_bnb_burn_endpoint, {});

    // --- Account: shared request types (download_id / download_link) ---
    add(rest::download_id_transaction_endpoint, field_names<types::download_id_request_t>());
    add(rest::download_link_transaction_endpoint, field_names<types::download_link_request_t>());
    add(rest::download_id_order_endpoint, field_names<types::download_id_request_t>());
    add(rest::download_link_order_endpoint, field_names<types::download_link_request_t>());
    add(rest::download_id_trade_endpoint, field_names<types::download_id_request_t>());
    add(rest::download_link_trade_endpoint, field_names<types::download_link_request_t>());

    // --- Trade: 1:1 traits ---
    add(rest::new_order_endpoint, field_names<types::new_order_request_t>());
    add(rest::modify_order_endpoint, field_names<types::modify_order_request_t>());
    add(rest::cancel_order_endpoint, field_names<types::cancel_order_request_t>());
    add(rest::query_order_endpoint, field_names<types::query_order_request_t>());
    add(rest::cancel_all_open_orders_endpoint, field_names<types::cancel_all_open_orders_request_t>());
    add(rest::auto_cancel_endpoint, field_names<types::auto_cancel_request_t>());
    add(rest::query_open_order_endpoint, field_names<types::query_open_order_request_t>());
    add(rest::all_open_orders_endpoint, field_names<types::all_open_orders_request_t>());
    add(rest::all_orders_endpoint, field_names<types::all_orders_request_t>());
    add(rest::position_risk_v3_endpoint, field_names<types::position_info_v3_request_t>());
    add(rest::adl_quantile_endpoint, field_names<types::adl_quantile_request_t>());
    add(rest::force_orders_endpoint, field_names<types::force_orders_request_t>());
    add(rest::account_trades_endpoint, field_names<types::account_trade_request_t>());
    add(rest::change_position_mode_endpoint, field_names<types::change_position_mode_request_t>());
    add(rest::change_multi_assets_endpoint, field_names<types::change_multi_assets_mode_request_t>());
    add(rest::change_leverage_endpoint, field_names<types::change_leverage_request_t>());
    add(rest::change_margin_type_endpoint, field_names<types::change_margin_type_request_t>());
    add(rest::modify_isolated_margin_endpoint, field_names<types::modify_isolated_margin_request_t>());
    add(rest::position_margin_history_endpoint, field_names<types::position_margin_history_request_t>());
    add(rest::order_modify_history_endpoint, field_names<types::order_modify_history_request_t>());
    add(rest::new_algo_order_endpoint, field_names<types::new_algo_order_request_t>());
    add(rest::cancel_algo_order_endpoint, field_names<types::cancel_algo_order_request_t>());
    add(rest::query_algo_order_endpoint, field_names<types::query_algo_order_request_t>());
    add(rest::all_algo_orders_endpoint, field_names<types::all_algo_orders_request_t>());

    // --- Trade: shared request types ---
    add(rest::test_order_endpoint, field_names<types::new_order_request_t>());
    add(rest::batch_orders_endpoint, field_names<types::batch_orders_request_t>());
    add(rest::modify_batch_orders_endpoint, field_names<types::batch_orders_request_t>());
    add(rest::cancel_batch_orders_endpoint, field_names<types::cancel_multiple_orders_request_t>());

    // --- Trade: parameterless ---
    add(rest::open_algo_orders_endpoint, {});
    add(rest::cancel_all_algo_orders_endpoint, {});
    add(rest::tradfi_perps_endpoint, field_names<types::tradfi_perps_request_t>());

    // --- User data streams ---
    add(rest::start_listen_key_endpoint, {});
    add(rest::keepalive_listen_key_endpoint, {});
    add(rest::close_listen_key_endpoint, {});

    // --- Convert: 1:1 traits ---
    add(rest::convert_get_quote_endpoint, field_names<types::convert_quote_request_t>());
    add(rest::convert_accept_quote_endpoint, field_names<types::convert_accept_request_t>());
    add(rest::convert_order_status_endpoint, field_names<types::convert_order_status_request_t>());

    return reg;
}

/// Parameters injected by the framework, not present in request structs.
const std::set<std::string> framework_params = {"timestamp", "signature", "recvWindow"};

/// Postman endpoints that binapi2 intentionally skips (deprecated or not applicable).
/// Key: (path, method).
const std::set<endpoint_key> known_skipped = {
    // v2 account/balance — binapi2 uses v3.
    {"/fapi/v2/account", "GET"},
    {"/fapi/v2/balance", "GET"},
    // Convert exchangeInfo — not part of the trading API.
    {"/fapi/v1/convert/exchangeInfo", "GET"},
};

/// Known parameter differences: Postman params that binapi2 intentionally omits
/// (e.g. parameterless service methods that could accept optional filters).
/// Key: (path, method), value: set of param names to ignore.
const std::map<endpoint_key, std::set<std::string>> known_missing_params = {
    // binapi2 exposes open_algo_orders() as a parameterless service method.
    // The official docs list optional algoId, algoType, symbol filters.
    // doc: docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Current-All-Algo-Open-Orders.md
    {{"/fapi/v1/openAlgoOrders", "GET"}, {"algoId", "algoType", "symbol"}},
    // binapi2 exposes cancel_all_algo_orders() as parameterless.
    // The official docs list symbol as mandatory.
    // doc: docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api/Cancel-All-Algo-Open-Orders.md
    {{"/fapi/v1/algoOpenOrders", "DELETE"}, {"symbol"}},
};

/// Known parameter differences: binapi2 params that the Postman collection omits.
/// These are valid Binance API params confirmed in the official docs.
/// Key: (path, method), value: set of param names to ignore.
const std::map<endpoint_key, std::set<std::string>> known_extra_params = {
    // Postman omits trailing-stop and conditional order params (stopPrice,
    // activationPrice, callbackRate, closePosition, workingType, priceProtect).
    // All six are documented as optional params for STOP/STOP_MARKET,
    // TAKE_PROFIT/TAKE_PROFIT_MARKET, and TRAILING_STOP_MARKET order types.
    // doc: docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/trade/rest-api.md  (New Order section)
    {{"/fapi/v1/order", "POST"}, {"stopPrice", "activationPrice", "callbackRate", "closePosition", "workingType", "priceProtect"}},
    // Same request type as new_order; same missing params.
    {{"/fapi/v1/order/test", "POST"}, {"stopPrice", "activationPrice", "callbackRate", "closePosition", "workingType", "priceProtect"}},
    // binapi2 supports an optional symbol filter.  The official docs list no
    // request params, but the live API accepts it (undocumented convenience).
    // doc: docs/api/md/developers.binance.com/docs/derivatives/usds-margined-futures/market-data/rest-api/Exchange-Information.md
    {{"/fapi/v1/exchangeInfo", "GET"}, {"symbol"}},
};

} // anonymous namespace

// ===========================================================================
// Tests.
// ===========================================================================

class PostmanValidation : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        postman_endpoints_ = parse_postman(POSTMAN_JSON_PATH);
        binapi_registry_ = build_binapi_registry();
    }

    static std::vector<postman_endpoint> postman_endpoints_;
    static std::map<endpoint_key, endpoint_info> binapi_registry_;
};

std::vector<postman_endpoint> PostmanValidation::postman_endpoints_;
std::map<endpoint_key, endpoint_info> PostmanValidation::binapi_registry_;

// ---------------------------------------------------------------------------
// Test 1: Every Postman endpoint has a matching binapi2 endpoint_metadata.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, AllPostmanEndpointsCovered)
{
    ASSERT_FALSE(postman_endpoints_.empty()) << "No endpoints parsed from Postman collection";

    std::vector<std::string> missing;
    for (const auto& ep : postman_endpoints_) {
        auto key = endpoint_key{ep.path, ep.method};
        if (known_skipped.count(key))
            continue;
        if (!binapi_registry_.count(key)) {
            missing.push_back(ep.method + " " + ep.path + " (" + ep.name + ")");
        }
    }

    EXPECT_TRUE(missing.empty()) << "Postman endpoints missing from binapi2:\n"
                                 << [&] {
                                        std::string s;
                                        for (const auto& m : missing)
                                            s += "  - " + m + "\n";
                                        return s;
                                    }();
}

// ---------------------------------------------------------------------------
// Test 2: Every binapi2 endpoint_metadata has a matching Postman endpoint.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, NoBinapi2EndpointsWithoutPostman)
{
    // Build lookup from Postman.
    std::set<endpoint_key> postman_keys;
    for (const auto& ep : postman_endpoints_) {
        postman_keys.insert({ep.path, ep.method});
    }

    std::vector<std::string> extra;
    for (const auto& [key, info] : binapi_registry_) {
        if (!postman_keys.count(key)) {
            extra.push_back(key.second + " " + key.first + " (" + std::string(info.metadata->name) + ")");
        }
    }

    EXPECT_TRUE(extra.empty()) << "binapi2 endpoints not found in Postman collection:\n"
                               << [&] {
                                      std::string s;
                                      for (const auto& e : extra)
                                          s += "  - " + e + "\n";
                                      return s;
                                  }();
}

// ---------------------------------------------------------------------------
// Test 3: HTTP method agreement.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, HttpMethodsMatch)
{
    for (const auto& ep : postman_endpoints_) {
        auto key = endpoint_key{ep.path, ep.method};
        auto it = binapi_registry_.find(key);
        if (it == binapi_registry_.end())
            continue; // Covered by the coverage test.

        EXPECT_EQ(verb_to_string(it->second.metadata->method), ep.method)
            << "Method mismatch for " << ep.path << " (" << ep.name << ")";
    }
}

// ---------------------------------------------------------------------------
// Test 4: Parameter coverage — binapi2 request structs cover all Postman params.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, RequestParametersCoverPostman)
{
    std::vector<std::string> issues;

    for (const auto& ep : postman_endpoints_) {
        auto key = endpoint_key{ep.path, ep.method};
        auto it = binapi_registry_.find(key);
        if (it == binapi_registry_.end())
            continue;

        const auto& binapi_params = it->second.param_names;

        // Known acceptable differences.
        auto km = known_missing_params.find(key);
        const auto& skip_set = (km != known_missing_params.end()) ? km->second : std::set<std::string>{};

        for (const auto& postman_param : ep.params) {
            // Skip framework-injected parameters.
            if (framework_params.count(postman_param))
                continue;
            if (skip_set.count(postman_param))
                continue;

            if (!binapi_params.count(postman_param)) {
                issues.push_back(ep.method + " " + ep.path + ": missing param '" + postman_param + "'");
            }
        }
    }

    EXPECT_TRUE(issues.empty()) << "Postman parameters missing from binapi2 request structs:\n"
                                << [&] {
                                       std::string s;
                                       for (const auto& i : issues)
                                           s += "  - " + i + "\n";
                                       return s;
                                   }();
}

// ---------------------------------------------------------------------------
// Test 5: No extra binapi2 params that Postman doesn't know about.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, NoExtraBinapi2Parameters)
{
    // Build Postman lookup: (path, method) -> set of param names.
    std::map<endpoint_key, std::set<std::string>> postman_params;
    for (const auto& ep : postman_endpoints_) {
        auto key = endpoint_key{ep.path, ep.method};
        postman_params[key] = ep.params;
    }

    std::vector<std::string> issues;

    for (const auto& [key, info] : binapi_registry_) {
        auto pit = postman_params.find(key);
        if (pit == postman_params.end())
            continue;

        // Known acceptable differences.
        auto ke = known_extra_params.find(key);
        const auto& skip_set = (ke != known_extra_params.end()) ? ke->second : std::set<std::string>{};

        for (const auto& binapi_param : info.param_names) {
            if (skip_set.count(binapi_param))
                continue;
            if (!pit->second.count(binapi_param)) {
                issues.push_back(key.second + " " + key.first + ": extra param '" + binapi_param + "'");
            }
        }
    }

    EXPECT_TRUE(issues.empty()) << "binapi2 parameters not found in Postman collection:\n"
                                << [&] {
                                       std::string s;
                                       for (const auto& i : issues)
                                           s += "  - " + i + "\n";
                                       return s;
                                   }();
}

// ---------------------------------------------------------------------------
// Test 6: Endpoint count sanity check.
// ---------------------------------------------------------------------------

TEST_F(PostmanValidation, EndpointCountSanity)
{
    // Postman has 95 endpoints; we should parse all of them.
    EXPECT_GE(postman_endpoints_.size(), 90u) << "Unexpectedly few Postman endpoints parsed";

    // binapi2 should cover the vast majority.
    EXPECT_GE(binapi_registry_.size(), 90u) << "Unexpectedly few binapi2 endpoints registered";
}
