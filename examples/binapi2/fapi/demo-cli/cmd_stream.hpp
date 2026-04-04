// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "common.hpp"

namespace demo {

int cmd_stream_book_ticker(const args_t& args);
int cmd_stream_mark_price(const args_t& args);
int cmd_stream_kline(const args_t& args);
int cmd_stream_ticker(const args_t& args);
int cmd_stream_depth(const args_t& args);
int cmd_stream_all_book_tickers(const args_t& args);
int cmd_stream_all_tickers(const args_t& args);
int cmd_stream_all_mini_tickers(const args_t& args);
int cmd_stream_liquidation(const args_t& args);

} // namespace demo
