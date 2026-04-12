// SPDX-License-Identifier: Apache-2.0
//
// Market data commands — public REST endpoints, no authentication required.

#include "cmd_market_data.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

// ---------------------------------------------------------------------------
// Existing commands (refactored to use helpers where straightforward)
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_ping(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->market_data.async_execute(types::ping_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("pong");
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_time(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    auto r = co_await (*rest)->market_data.async_execute(types::server_time_request_t{});
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("server time: {}", r->serverTime);
    if (verbosity >= 1) print_json(*r);
    co_return 0;
}

boost::cobalt::task<int> cmd_exchange_info(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::exchange_info_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_order_book(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: order-book <symbol> [limit]"); co_return 1; }

    auto rest = co_await c.create_rest_client();
    if (!rest) { spdlog::error("connect: {}", rest.err.message); co_return 1; }
    types::order_book_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);

    auto r = co_await (*rest)->market_data.async_execute(req);
    if (!r) { print_error(r.err); co_return 1; }

    spdlog::info("order book: bids={} asks={} lastUpdateId={}",
                 r->bids.size(), r->asks.size(), r->lastUpdateId);

    if (verbosity >= 1) {
        print_json(*r);
    } else {
        int n = std::min(5, static_cast<int>(r->bids.size()));
        for (int i = 0; i < n; ++i)
            out("  bid {} x {}", r->bids[static_cast<std::size_t>(i)].price,
                r->bids[static_cast<std::size_t>(i)].quantity);
        n = std::min(5, static_cast<int>(r->asks.size()));
        for (int i = 0; i < n; ++i)
            out("  ask {} x {}", r->asks[static_cast<std::size_t>(i)].price,
                r->asks[static_cast<std::size_t>(i)].quantity);
    }
    co_return 0;
}

boost::cobalt::task<int> cmd_recent_trades(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: recent-trades <symbol> [limit]"); co_return 1; }
    types::recent_trades_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_book_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: book-ticker <symbol>"); co_return 1; }
    types::book_ticker_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_book_tickers(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::book_tickers_request_t{});
}

boost::cobalt::task<int> cmd_price_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: price-ticker <symbol>"); co_return 1; }
    types::price_ticker_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_price_tickers(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::price_tickers_request_t{});
}

boost::cobalt::task<int> cmd_ticker_24hr(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: ticker-24hr <symbol>"); co_return 1; }
    types::ticker_24hr_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_mark_price(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: mark-price <symbol>"); co_return 1; }
    types::mark_price_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_mark_prices(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::mark_prices_request_t{});
}

boost::cobalt::task<int> cmd_klines(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_kline<types::klines_request_t>(c, args, "klines <symbol> <interval> [limit]");
}

boost::cobalt::task<int> cmd_funding_rate(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: funding-rate <symbol> [limit]"); co_return 1; }
    types::funding_rate_history_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_open_interest(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: open-interest <symbol>"); co_return 1; }
    types::open_interest_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

// ---------------------------------------------------------------------------
// New market data commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_aggregate_trades(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: aggregate-trades <symbol> [limit]"); co_return 1; }
    types::aggregate_trades_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_historical_trades(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: historical-trades <symbol> [limit]"); co_return 1; }
    types::historical_trades_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_continuous_kline(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_pair_kline<types::continuous_kline_request_t>(
        c, args, "continuous-kline <pair> <interval> [limit]");
}

boost::cobalt::task<int> cmd_index_price_kline(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_pair_kline<types::index_price_kline_request_t>(
        c, args, "index-price-kline <pair> <interval> [limit]");
}

boost::cobalt::task<int> cmd_mark_price_klines(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_kline<types::mark_price_klines_request_t>(
        c, args, "mark-price-klines <symbol> <interval> [limit]");
}

boost::cobalt::task<int> cmd_premium_index_klines(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_kline<types::premium_index_klines_request_t>(
        c, args, "premium-index-klines <symbol> <interval> [limit]");
}

boost::cobalt::task<int> cmd_price_ticker_v2(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: price-ticker-v2 <symbol>"); co_return 1; }
    types::price_ticker_v2_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_price_tickers_v2(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::price_tickers_v2_request_t{});
}

boost::cobalt::task<int> cmd_ticker_24hrs(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::ticker_24hrs_request_t{});
}

boost::cobalt::task<int> cmd_funding_rate_info(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::funding_rate_info_request_t{});
}

boost::cobalt::task<int> cmd_open_interest_stats(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_futures_analytics<types::open_interest_statistics_request_t>(
        c, args, "open-interest-stats <symbol> <period> [limit]");
}

boost::cobalt::task<int> cmd_top_ls_account_ratio(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_futures_analytics<types::top_long_short_account_ratio_request_t>(
        c, args, "top-ls-account-ratio <symbol> <period> [limit]");
}

boost::cobalt::task<int> cmd_top_ls_trader_ratio(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_futures_analytics<types::top_trader_long_short_ratio_request_t>(
        c, args, "top-ls-trader-ratio <symbol> <period> [limit]");
}

boost::cobalt::task<int> cmd_long_short_ratio(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_futures_analytics<types::long_short_ratio_request_t>(
        c, args, "long-short-ratio <symbol> <period> [limit]");
}

boost::cobalt::task<int> cmd_taker_volume(binapi2::futures_usdm_api& c, const args_t& args)
{
    co_return co_await cmd_futures_analytics<types::taker_buy_sell_volume_request_t>(
        c, args, "taker-volume <symbol> <period> [limit]");
}

boost::cobalt::task<int> cmd_basis(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: basis <pair> <period> [limit]"); co_return 1; }
    types::basis_request_t req;
    req.pair = args[0];
    req.period = parse_enum<types::futures_data_period_t>(args[1]);
    if (args.size() > 2) req.limit = std::stoi(args[2]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_delivery_price(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: delivery-price <pair>"); co_return 1; }
    types::delivery_price_request_t req;
    req.pair = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_composite_index_info(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::composite_index_info_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_index_constituents(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: index-constituents <symbol>"); co_return 1; }
    types::index_constituents_request_t req;
    req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_asset_index(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::asset_index_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_insurance_fund(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::insurance_fund_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_adl_risk(binapi2::futures_usdm_api& c, const args_t& args)
{
    types::adl_risk_request_t req;
    if (!args.empty()) req.symbol = args[0];
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_rpi_depth(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: rpi-depth <symbol> [limit]"); co_return 1; }
    types::rpi_depth_request_t req;
    req.symbol = args[0];
    if (args.size() > 1) req.limit = std::stoi(args[1]);
    co_return co_await exec_market_data(c, req);
}

boost::cobalt::task<int> cmd_trading_schedule(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_market_data(c, types::trading_schedule_request_t{});
}

} // namespace demo
