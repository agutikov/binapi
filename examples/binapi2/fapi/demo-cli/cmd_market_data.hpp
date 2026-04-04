// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

int cmd_ping(const args_t& args);
int cmd_time(const args_t& args);
int cmd_exchange_info(const args_t& args);
int cmd_order_book(const args_t& args);
int cmd_recent_trades(const args_t& args);
int cmd_book_ticker(const args_t& args);
int cmd_book_tickers(const args_t& args);
int cmd_price_ticker(const args_t& args);
int cmd_price_tickers(const args_t& args);
int cmd_ticker_24hr(const args_t& args);
int cmd_mark_price(const args_t& args);
int cmd_mark_prices(const args_t& args);
int cmd_klines(const args_t& args);
int cmd_funding_rate(const args_t& args);
int cmd_open_interest(const args_t& args);

} // namespace demo
