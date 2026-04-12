// SPDX-License-Identifier: Apache-2.0
//
// Market data stream commands — async WebSocket streams using cobalt generators.

#include "cmd_stream.hpp"

#include <binapi2/futures_usdm_api.hpp>

#include <spdlog/spdlog.h>

namespace demo {

namespace types = binapi2::fapi::types;

// ---------------------------------------------------------------------------
// Existing commands (refactored to use exec_stream)
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_stream_book_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-book-ticker <symbol>"); co_return 1; }
    types::book_ticker_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_mark_price(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-mark-price <symbol>"); co_return 1; }
    types::mark_price_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_kline(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: stream-kline <symbol> <interval>"); co_return 1; }
    types::kline_subscription sub;
    sub.symbol = args[0];
    sub.interval = parse_enum<types::kline_interval_t>(args[1]);
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-ticker <symbol>"); co_return 1; }
    types::ticker_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_depth(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-depth <symbol> [levels]"); co_return 1; }
    types::partial_book_depth_subscription sub;
    sub.symbol = args[0];
    if (args.size() > 1) sub.levels = std::stoi(args[1]);
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_all_book_tickers(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_book_ticker_subscription{});
}

boost::cobalt::task<int> cmd_stream_all_tickers(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_market_ticker_subscription{});
}

boost::cobalt::task<int> cmd_stream_all_mini_tickers(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_market_mini_ticker_subscription{});
}

boost::cobalt::task<int> cmd_stream_liquidation(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-liquidation <symbol>"); co_return 1; }
    types::liquidation_order_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

// ---------------------------------------------------------------------------
// New stream commands
// ---------------------------------------------------------------------------

boost::cobalt::task<int> cmd_stream_aggregate_trade(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-aggregate-trade <symbol>"); co_return 1; }
    types::aggregate_trade_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_diff_depth(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-diff-depth <symbol> [speed]"); co_return 1; }
    types::diff_book_depth_subscription sub;
    sub.symbol = args[0];
    if (args.size() > 1) sub.speed = args[1];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_mini_ticker(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-mini-ticker <symbol>"); co_return 1; }
    types::mini_ticker_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_all_liquidations(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_market_liquidation_order_subscription{});
}

boost::cobalt::task<int> cmd_stream_all_mark_prices(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_market_mark_price_subscription{});
}

boost::cobalt::task<int> cmd_stream_continuous_kline(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.size() < 2) { spdlog::error("usage: stream-continuous-kline <pair> <interval>"); co_return 1; }
    types::continuous_contract_kline_subscription sub;
    sub.pair = args[0];
    sub.interval = parse_enum<types::kline_interval_t>(args[1]);
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_composite_index(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-composite-index <symbol>"); co_return 1; }
    types::composite_index_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_contract_info(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::contract_info_subscription{});
}

boost::cobalt::task<int> cmd_stream_asset_index(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-asset-index <symbol>"); co_return 1; }
    types::asset_index_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

boost::cobalt::task<int> cmd_stream_all_asset_index(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::all_asset_index_subscription{});
}

boost::cobalt::task<int> cmd_stream_trading_session(binapi2::futures_usdm_api& c, const args_t& /*args*/)
{
    co_return co_await exec_stream(c, types::trading_session_subscription{});
}

boost::cobalt::task<int> cmd_stream_rpi_diff_depth(binapi2::futures_usdm_api& c, const args_t& args)
{
    if (args.empty()) { spdlog::error("usage: stream-rpi-diff-depth <symbol>"); co_return 1; }
    types::rpi_diff_book_depth_subscription sub;
    sub.symbol = args[0];
    co_return co_await exec_stream(c, sub);
}

} // namespace demo
