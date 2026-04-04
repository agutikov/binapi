// SPDX-License-Identifier: Apache-2.0

#include <binapi2/fapi/types/enums.hpp>
#include <gtest/gtest.h>

#include <string>

namespace {

using namespace binapi2::fapi::types;

template<typename E>
void check_json_roundtrip(E val, const std::string& wire) {
    // Serialize
    std::string json;
    (void)glz::write_json(val, json);
    EXPECT_EQ(json, "\"" + wire + "\"") << "serialize " << wire;
    // Deserialize
    E parsed{};
    auto err = glz::read_json(parsed, json);
    EXPECT_FALSE(err) << "parse " << wire;
    EXPECT_EQ(parsed, val) << "roundtrip " << wire;
}

// --------------------------------------------------------------------------
// security_type — to_string only (no glaze meta)
// --------------------------------------------------------------------------

TEST(Enums, SecurityType) {
    EXPECT_EQ(to_string(security_type::none), "none");
    EXPECT_EQ(to_string(security_type::market_data), "market_data");
    EXPECT_EQ(to_string(security_type::user_stream), "user_stream");
    EXPECT_EQ(to_string(security_type::user_data), "user_data");
    EXPECT_EQ(to_string(security_type::trade), "trade");
}

// --------------------------------------------------------------------------
// order_side
// --------------------------------------------------------------------------

TEST(Enums, OrderSide) {
    EXPECT_EQ(to_string(order_side::buy), "BUY");
    EXPECT_EQ(to_string(order_side::sell), "SELL");

    check_json_roundtrip(order_side::buy, "BUY");
    check_json_roundtrip(order_side::sell, "SELL");
}

// --------------------------------------------------------------------------
// order_type
// --------------------------------------------------------------------------

TEST(Enums, OrderType) {
    EXPECT_EQ(to_string(order_type::limit), "LIMIT");
    EXPECT_EQ(to_string(order_type::market), "MARKET");
    EXPECT_EQ(to_string(order_type::stop), "STOP");
    EXPECT_EQ(to_string(order_type::stop_market), "STOP_MARKET");
    EXPECT_EQ(to_string(order_type::take_profit), "TAKE_PROFIT");
    EXPECT_EQ(to_string(order_type::take_profit_market), "TAKE_PROFIT_MARKET");
    EXPECT_EQ(to_string(order_type::trailing_stop_market), "TRAILING_STOP_MARKET");

    check_json_roundtrip(order_type::limit, "LIMIT");
    check_json_roundtrip(order_type::market, "MARKET");
    check_json_roundtrip(order_type::stop, "STOP");
    check_json_roundtrip(order_type::stop_market, "STOP_MARKET");
    check_json_roundtrip(order_type::take_profit, "TAKE_PROFIT");
    check_json_roundtrip(order_type::take_profit_market, "TAKE_PROFIT_MARKET");
    check_json_roundtrip(order_type::trailing_stop_market, "TRAILING_STOP_MARKET");
}

// --------------------------------------------------------------------------
// time_in_force
// --------------------------------------------------------------------------

TEST(Enums, TimeInForce) {
    EXPECT_EQ(to_string(time_in_force::gtc), "GTC");
    EXPECT_EQ(to_string(time_in_force::ioc), "IOC");
    EXPECT_EQ(to_string(time_in_force::fok), "FOK");
    EXPECT_EQ(to_string(time_in_force::gtx), "GTX");
    EXPECT_EQ(to_string(time_in_force::gtd), "GTD");
    EXPECT_EQ(to_string(time_in_force::rpi), "RPI");

    check_json_roundtrip(time_in_force::gtc, "GTC");
    check_json_roundtrip(time_in_force::ioc, "IOC");
    check_json_roundtrip(time_in_force::fok, "FOK");
    check_json_roundtrip(time_in_force::gtx, "GTX");
    check_json_roundtrip(time_in_force::gtd, "GTD");
    check_json_roundtrip(time_in_force::rpi, "RPI");
}

// --------------------------------------------------------------------------
// kline_interval
// --------------------------------------------------------------------------

TEST(Enums, KlineInterval) {
    EXPECT_EQ(to_string(kline_interval::m1), "1m");
    EXPECT_EQ(to_string(kline_interval::m3), "3m");
    EXPECT_EQ(to_string(kline_interval::m5), "5m");
    EXPECT_EQ(to_string(kline_interval::m15), "15m");
    EXPECT_EQ(to_string(kline_interval::m30), "30m");
    EXPECT_EQ(to_string(kline_interval::h1), "1h");
    EXPECT_EQ(to_string(kline_interval::h2), "2h");
    EXPECT_EQ(to_string(kline_interval::h4), "4h");
    EXPECT_EQ(to_string(kline_interval::h6), "6h");
    EXPECT_EQ(to_string(kline_interval::h8), "8h");
    EXPECT_EQ(to_string(kline_interval::h12), "12h");
    EXPECT_EQ(to_string(kline_interval::d1), "1d");
    EXPECT_EQ(to_string(kline_interval::d3), "3d");
    EXPECT_EQ(to_string(kline_interval::w1), "1w");
    EXPECT_EQ(to_string(kline_interval::mo1), "1M");

    check_json_roundtrip(kline_interval::m1, "1m");
    check_json_roundtrip(kline_interval::m3, "3m");
    check_json_roundtrip(kline_interval::m5, "5m");
    check_json_roundtrip(kline_interval::m15, "15m");
    check_json_roundtrip(kline_interval::m30, "30m");
    check_json_roundtrip(kline_interval::h1, "1h");
    check_json_roundtrip(kline_interval::h2, "2h");
    check_json_roundtrip(kline_interval::h4, "4h");
    check_json_roundtrip(kline_interval::h6, "6h");
    check_json_roundtrip(kline_interval::h8, "8h");
    check_json_roundtrip(kline_interval::h12, "12h");
    check_json_roundtrip(kline_interval::d1, "1d");
    check_json_roundtrip(kline_interval::d3, "3d");
    check_json_roundtrip(kline_interval::w1, "1w");
    check_json_roundtrip(kline_interval::mo1, "1M");
}

// --------------------------------------------------------------------------
// position_side
// --------------------------------------------------------------------------

TEST(Enums, PositionSide) {
    EXPECT_EQ(to_string(position_side::both), "BOTH");
    EXPECT_EQ(to_string(position_side::long_side), "LONG");
    EXPECT_EQ(to_string(position_side::short_side), "SHORT");

    check_json_roundtrip(position_side::both, "BOTH");
    check_json_roundtrip(position_side::long_side, "LONG");
    check_json_roundtrip(position_side::short_side, "SHORT");
}

// --------------------------------------------------------------------------
// working_type
// --------------------------------------------------------------------------

TEST(Enums, WorkingType) {
    EXPECT_EQ(to_string(working_type::mark_price), "MARK_PRICE");
    EXPECT_EQ(to_string(working_type::contract_price), "CONTRACT_PRICE");

    check_json_roundtrip(working_type::mark_price, "MARK_PRICE");
    check_json_roundtrip(working_type::contract_price, "CONTRACT_PRICE");
}

// --------------------------------------------------------------------------
// response_type
// --------------------------------------------------------------------------

TEST(Enums, ResponseType) {
    EXPECT_EQ(to_string(response_type::ack), "ACK");
    EXPECT_EQ(to_string(response_type::result), "RESULT");

    check_json_roundtrip(response_type::ack, "ACK");
    check_json_roundtrip(response_type::result, "RESULT");
}

// --------------------------------------------------------------------------
// margin_type
// --------------------------------------------------------------------------

TEST(Enums, MarginType) {
    EXPECT_EQ(to_string(margin_type::isolated), "ISOLATED");
    EXPECT_EQ(to_string(margin_type::crossed), "CROSSED");

    // Glaze meta has multiple aliases; last mapping wins for serialization.
    // "ISOLATED"/"isolated" → isolated; glaze writes "isolated".
    // "CROSSED"/"cross" → crossed; glaze writes "cross".
    check_json_roundtrip(margin_type::isolated, "isolated");
    check_json_roundtrip(margin_type::crossed, "cross");
}

// --------------------------------------------------------------------------
// contract_type
// --------------------------------------------------------------------------

TEST(Enums, ContractType) {
    EXPECT_EQ(to_string(contract_type::perpetual), "PERPETUAL");
    EXPECT_EQ(to_string(contract_type::current_month), "CURRENT_MONTH");
    EXPECT_EQ(to_string(contract_type::next_month), "NEXT_MONTH");
    EXPECT_EQ(to_string(contract_type::current_quarter), "CURRENT_QUARTER");
    EXPECT_EQ(to_string(contract_type::next_quarter), "NEXT_QUARTER");
    EXPECT_EQ(to_string(contract_type::perpetual_delivering), "PERPETUAL_DELIVERING");
    EXPECT_EQ(to_string(contract_type::tradifi_perpetual), "TRADIFI_PERPETUAL");

    check_json_roundtrip(contract_type::perpetual, "PERPETUAL");
    check_json_roundtrip(contract_type::current_month, "CURRENT_MONTH");
    check_json_roundtrip(contract_type::next_month, "NEXT_MONTH");
    check_json_roundtrip(contract_type::current_quarter, "CURRENT_QUARTER");
    check_json_roundtrip(contract_type::next_quarter, "NEXT_QUARTER");
    check_json_roundtrip(contract_type::perpetual_delivering, "PERPETUAL_DELIVERING");
    check_json_roundtrip(contract_type::tradifi_perpetual, "TRADIFI_PERPETUAL");
}

// --------------------------------------------------------------------------
// contract_status
// --------------------------------------------------------------------------

TEST(Enums, ContractStatus) {
    EXPECT_EQ(to_string(contract_status::pending_trading), "PENDING_TRADING");
    EXPECT_EQ(to_string(contract_status::trading), "TRADING");
    EXPECT_EQ(to_string(contract_status::pre_delivering), "PRE_DELIVERING");
    EXPECT_EQ(to_string(contract_status::delivering), "DELIVERING");
    EXPECT_EQ(to_string(contract_status::delivered), "DELIVERED");
    EXPECT_EQ(to_string(contract_status::pre_settle), "PRE_SETTLE");
    EXPECT_EQ(to_string(contract_status::settling), "SETTLING");
    EXPECT_EQ(to_string(contract_status::close), "CLOSE");

    check_json_roundtrip(contract_status::pending_trading, "PENDING_TRADING");
    check_json_roundtrip(contract_status::trading, "TRADING");
    check_json_roundtrip(contract_status::pre_delivering, "PRE_DELIVERING");
    check_json_roundtrip(contract_status::delivering, "DELIVERING");
    check_json_roundtrip(contract_status::delivered, "DELIVERED");
    check_json_roundtrip(contract_status::pre_settle, "PRE_SETTLE");
    check_json_roundtrip(contract_status::settling, "SETTLING");
    check_json_roundtrip(contract_status::close, "CLOSE");
}

// --------------------------------------------------------------------------
// order_status
// --------------------------------------------------------------------------

TEST(Enums, OrderStatus) {
    EXPECT_EQ(to_string(order_status::new_order), "NEW");
    EXPECT_EQ(to_string(order_status::partially_filled), "PARTIALLY_FILLED");
    EXPECT_EQ(to_string(order_status::filled), "FILLED");
    EXPECT_EQ(to_string(order_status::canceled), "CANCELED");
    EXPECT_EQ(to_string(order_status::rejected), "REJECTED");
    EXPECT_EQ(to_string(order_status::expired), "EXPIRED");
    EXPECT_EQ(to_string(order_status::expired_in_match), "EXPIRED_IN_MATCH");

    check_json_roundtrip(order_status::new_order, "NEW");
    check_json_roundtrip(order_status::partially_filled, "PARTIALLY_FILLED");
    check_json_roundtrip(order_status::filled, "FILLED");
    check_json_roundtrip(order_status::canceled, "CANCELED");
    check_json_roundtrip(order_status::rejected, "REJECTED");
    check_json_roundtrip(order_status::expired, "EXPIRED");
    check_json_roundtrip(order_status::expired_in_match, "EXPIRED_IN_MATCH");
}

// --------------------------------------------------------------------------
// stp_mode
// --------------------------------------------------------------------------

TEST(Enums, StpMode) {
    EXPECT_EQ(to_string(stp_mode::expire_taker), "EXPIRE_TAKER");
    EXPECT_EQ(to_string(stp_mode::expire_both), "EXPIRE_BOTH");
    EXPECT_EQ(to_string(stp_mode::expire_maker), "EXPIRE_MAKER");

    check_json_roundtrip(stp_mode::expire_taker, "EXPIRE_TAKER");
    check_json_roundtrip(stp_mode::expire_both, "EXPIRE_BOTH");
    check_json_roundtrip(stp_mode::expire_maker, "EXPIRE_MAKER");
}

// --------------------------------------------------------------------------
// price_match
// --------------------------------------------------------------------------

TEST(Enums, PriceMatch) {
    EXPECT_EQ(to_string(price_match::none), "NONE");
    EXPECT_EQ(to_string(price_match::opponent), "OPPONENT");
    EXPECT_EQ(to_string(price_match::opponent_5), "OPPONENT_5");
    EXPECT_EQ(to_string(price_match::opponent_10), "OPPONENT_10");
    EXPECT_EQ(to_string(price_match::opponent_20), "OPPONENT_20");
    EXPECT_EQ(to_string(price_match::queue), "QUEUE");
    EXPECT_EQ(to_string(price_match::queue_5), "QUEUE_5");
    EXPECT_EQ(to_string(price_match::queue_10), "QUEUE_10");
    EXPECT_EQ(to_string(price_match::queue_20), "QUEUE_20");

    check_json_roundtrip(price_match::none, "NONE");
    check_json_roundtrip(price_match::opponent, "OPPONENT");
    check_json_roundtrip(price_match::opponent_5, "OPPONENT_5");
    check_json_roundtrip(price_match::opponent_10, "OPPONENT_10");
    check_json_roundtrip(price_match::opponent_20, "OPPONENT_20");
    check_json_roundtrip(price_match::queue, "QUEUE");
    check_json_roundtrip(price_match::queue_5, "QUEUE_5");
    check_json_roundtrip(price_match::queue_10, "QUEUE_10");
    check_json_roundtrip(price_match::queue_20, "QUEUE_20");
}

// --------------------------------------------------------------------------
// income_type
// --------------------------------------------------------------------------

TEST(Enums, IncomeType) {
    EXPECT_EQ(to_string(income_type::transfer), "TRANSFER");
    EXPECT_EQ(to_string(income_type::welcome_bonus), "WELCOME_BONUS");
    EXPECT_EQ(to_string(income_type::realized_pnl), "REALIZED_PNL");
    EXPECT_EQ(to_string(income_type::funding_fee), "FUNDING_FEE");
    EXPECT_EQ(to_string(income_type::commission), "COMMISSION");
    EXPECT_EQ(to_string(income_type::insurance_clear), "INSURANCE_CLEAR");
    EXPECT_EQ(to_string(income_type::referral_kickback), "REFERRAL_KICKBACK");
    EXPECT_EQ(to_string(income_type::commission_rebate), "COMMISSION_REBATE");
    EXPECT_EQ(to_string(income_type::api_rebate), "API_REBATE");
    EXPECT_EQ(to_string(income_type::contest_reward), "CONTEST_REWARD");
    EXPECT_EQ(to_string(income_type::cross_collateral_transfer), "CROSS_COLLATERAL_TRANSFER");
    EXPECT_EQ(to_string(income_type::options_premium_fee), "OPTIONS_PREMIUM_FEE");
    EXPECT_EQ(to_string(income_type::options_settle_profit), "OPTIONS_SETTLE_PROFIT");
    EXPECT_EQ(to_string(income_type::internal_transfer), "INTERNAL_TRANSFER");
    EXPECT_EQ(to_string(income_type::auto_exchange), "AUTO_EXCHANGE");
    EXPECT_EQ(to_string(income_type::delivered_settelment), "DELIVERED_SETTELMENT");
    EXPECT_EQ(to_string(income_type::coin_swap_deposit), "COIN_SWAP_DEPOSIT");
    EXPECT_EQ(to_string(income_type::coin_swap_withdraw), "COIN_SWAP_WITHDRAW");
    EXPECT_EQ(to_string(income_type::position_limit_increase_fee), "POSITION_LIMIT_INCREASE_FEE");

    check_json_roundtrip(income_type::transfer, "TRANSFER");
    check_json_roundtrip(income_type::welcome_bonus, "WELCOME_BONUS");
    check_json_roundtrip(income_type::realized_pnl, "REALIZED_PNL");
    check_json_roundtrip(income_type::funding_fee, "FUNDING_FEE");
    check_json_roundtrip(income_type::commission, "COMMISSION");
    check_json_roundtrip(income_type::insurance_clear, "INSURANCE_CLEAR");
    check_json_roundtrip(income_type::referral_kickback, "REFERRAL_KICKBACK");
    check_json_roundtrip(income_type::commission_rebate, "COMMISSION_REBATE");
    check_json_roundtrip(income_type::api_rebate, "API_REBATE");
    check_json_roundtrip(income_type::contest_reward, "CONTEST_REWARD");
    check_json_roundtrip(income_type::cross_collateral_transfer, "CROSS_COLLATERAL_TRANSFER");
    check_json_roundtrip(income_type::options_premium_fee, "OPTIONS_PREMIUM_FEE");
    check_json_roundtrip(income_type::options_settle_profit, "OPTIONS_SETTLE_PROFIT");
    check_json_roundtrip(income_type::internal_transfer, "INTERNAL_TRANSFER");
    check_json_roundtrip(income_type::auto_exchange, "AUTO_EXCHANGE");
    check_json_roundtrip(income_type::delivered_settelment, "DELIVERED_SETTELMENT");
    check_json_roundtrip(income_type::coin_swap_deposit, "COIN_SWAP_DEPOSIT");
    check_json_roundtrip(income_type::coin_swap_withdraw, "COIN_SWAP_WITHDRAW");
    check_json_roundtrip(income_type::position_limit_increase_fee, "POSITION_LIMIT_INCREASE_FEE");
}

// --------------------------------------------------------------------------
// futures_data_period
// --------------------------------------------------------------------------

TEST(Enums, FuturesDataPeriod) {
    EXPECT_EQ(to_string(futures_data_period::m5), "5m");
    EXPECT_EQ(to_string(futures_data_period::m15), "15m");
    EXPECT_EQ(to_string(futures_data_period::m30), "30m");
    EXPECT_EQ(to_string(futures_data_period::h1), "1h");
    EXPECT_EQ(to_string(futures_data_period::h2), "2h");
    EXPECT_EQ(to_string(futures_data_period::h4), "4h");
    EXPECT_EQ(to_string(futures_data_period::h6), "6h");
    EXPECT_EQ(to_string(futures_data_period::h12), "12h");
    EXPECT_EQ(to_string(futures_data_period::d1), "1d");

    check_json_roundtrip(futures_data_period::m5, "5m");
    check_json_roundtrip(futures_data_period::m15, "15m");
    check_json_roundtrip(futures_data_period::m30, "30m");
    check_json_roundtrip(futures_data_period::h1, "1h");
    check_json_roundtrip(futures_data_period::h2, "2h");
    check_json_roundtrip(futures_data_period::h4, "4h");
    check_json_roundtrip(futures_data_period::h6, "6h");
    check_json_roundtrip(futures_data_period::h12, "12h");
    check_json_roundtrip(futures_data_period::d1, "1d");
}

// --------------------------------------------------------------------------
// execution_type
// --------------------------------------------------------------------------

TEST(Enums, ExecutionType) {
    EXPECT_EQ(to_string(execution_type::new_order), "NEW");
    EXPECT_EQ(to_string(execution_type::partial_fill), "PARTIAL_FILL");
    EXPECT_EQ(to_string(execution_type::fill), "FILL");
    EXPECT_EQ(to_string(execution_type::canceled), "CANCELED");
    EXPECT_EQ(to_string(execution_type::rejected), "REJECTED");
    EXPECT_EQ(to_string(execution_type::expired), "EXPIRED");
    EXPECT_EQ(to_string(execution_type::trade), "TRADE");

    check_json_roundtrip(execution_type::new_order, "NEW");
    check_json_roundtrip(execution_type::partial_fill, "PARTIAL_FILL");
    check_json_roundtrip(execution_type::fill, "FILL");
    check_json_roundtrip(execution_type::canceled, "CANCELED");
    check_json_roundtrip(execution_type::rejected, "REJECTED");
    check_json_roundtrip(execution_type::expired, "EXPIRED");
    check_json_roundtrip(execution_type::trade, "TRADE");
}

// --------------------------------------------------------------------------
// rate_limit_type
// --------------------------------------------------------------------------

TEST(Enums, RateLimitType) {
    EXPECT_EQ(to_string(rate_limit_type::request_weight), "REQUEST_WEIGHT");
    EXPECT_EQ(to_string(rate_limit_type::orders_1s), "ORDERS_1S");
    EXPECT_EQ(to_string(rate_limit_type::orders_1m), "ORDERS_1M");
    EXPECT_EQ(to_string(rate_limit_type::orders_1h), "ORDERS_1H");
    EXPECT_EQ(to_string(rate_limit_type::orders_1d), "ORDERS_1D");

    check_json_roundtrip(rate_limit_type::request_weight, "REQUEST_WEIGHT");
    check_json_roundtrip(rate_limit_type::orders_1s, "ORDERS_1S");
    check_json_roundtrip(rate_limit_type::orders_1m, "ORDERS_1M");
    check_json_roundtrip(rate_limit_type::orders_1h, "ORDERS_1H");
    check_json_roundtrip(rate_limit_type::orders_1d, "ORDERS_1D");
}

// --------------------------------------------------------------------------
// rate_limit_interval
// --------------------------------------------------------------------------

TEST(Enums, RateLimitInterval) {
    EXPECT_EQ(to_string(rate_limit_interval::second), "SECOND");
    EXPECT_EQ(to_string(rate_limit_interval::minute), "MINUTE");
    EXPECT_EQ(to_string(rate_limit_interval::hour), "HOUR");
    EXPECT_EQ(to_string(rate_limit_interval::day), "DAY");

    check_json_roundtrip(rate_limit_interval::second, "SECOND");
    check_json_roundtrip(rate_limit_interval::minute, "MINUTE");
    check_json_roundtrip(rate_limit_interval::hour, "HOUR");
    check_json_roundtrip(rate_limit_interval::day, "DAY");
}

// --------------------------------------------------------------------------
// auto_close_type
// --------------------------------------------------------------------------

TEST(Enums, AutoCloseType) {
    EXPECT_EQ(to_string(auto_close_type::liquidation), "LIQUIDATION");
    EXPECT_EQ(to_string(auto_close_type::adl), "ADL");

    check_json_roundtrip(auto_close_type::liquidation, "LIQUIDATION");
    check_json_roundtrip(auto_close_type::adl, "ADL");
}

// --------------------------------------------------------------------------
// delta_type
// --------------------------------------------------------------------------

TEST(Enums, DeltaType) {
    EXPECT_EQ(to_string(delta_type::add), "1");
    EXPECT_EQ(to_string(delta_type::reduce), "2");

    check_json_roundtrip(delta_type::add, "1");
    check_json_roundtrip(delta_type::reduce, "2");
}

// --------------------------------------------------------------------------
// algo_type
// --------------------------------------------------------------------------

TEST(Enums, AlgoType) {
    EXPECT_EQ(to_string(algo_type::twap), "TWAP");
    EXPECT_EQ(to_string(algo_type::vp), "VP");

    check_json_roundtrip(algo_type::twap, "TWAP");
    check_json_roundtrip(algo_type::vp, "VP");
}

// --------------------------------------------------------------------------
// algo_status
// --------------------------------------------------------------------------

TEST(Enums, AlgoStatus) {
    EXPECT_EQ(to_string(algo_status::working), "WORKING");
    EXPECT_EQ(to_string(algo_status::cancelled), "CANCELLED");
    EXPECT_EQ(to_string(algo_status::rejected), "REJECTED");
    EXPECT_EQ(to_string(algo_status::expired), "EXPIRED");
    EXPECT_EQ(to_string(algo_status::triggered), "TRIGGERED");

    check_json_roundtrip(algo_status::working, "WORKING");
    check_json_roundtrip(algo_status::cancelled, "CANCELLED");
    check_json_roundtrip(algo_status::rejected, "REJECTED");
    check_json_roundtrip(algo_status::expired, "EXPIRED");
    check_json_roundtrip(algo_status::triggered, "TRIGGERED");
}

} // anonymous namespace
