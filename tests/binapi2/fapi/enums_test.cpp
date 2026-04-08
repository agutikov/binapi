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
// security_type_t — to_string only (no glaze meta)
// --------------------------------------------------------------------------

TEST(Enums, SecurityType) {
    EXPECT_EQ(to_string(security_type_t::none), "none");
    EXPECT_EQ(to_string(security_type_t::market_data), "market_data");
    EXPECT_EQ(to_string(security_type_t::user_stream), "user_stream");
    EXPECT_EQ(to_string(security_type_t::user_data), "user_data");
    EXPECT_EQ(to_string(security_type_t::trade), "trade");
}

// --------------------------------------------------------------------------
// order_side_t
// --------------------------------------------------------------------------

TEST(Enums, OrderSide) {
    EXPECT_EQ(to_string(order_side_t::buy), "BUY");
    EXPECT_EQ(to_string(order_side_t::sell), "SELL");

    check_json_roundtrip(order_side_t::buy, "BUY");
    check_json_roundtrip(order_side_t::sell, "SELL");
}

// --------------------------------------------------------------------------
// order_type_t
// --------------------------------------------------------------------------

TEST(Enums, OrderType) {
    EXPECT_EQ(to_string(order_type_t::limit), "LIMIT");
    EXPECT_EQ(to_string(order_type_t::market), "MARKET");
    EXPECT_EQ(to_string(order_type_t::stop), "STOP");
    EXPECT_EQ(to_string(order_type_t::stop_market), "STOP_MARKET");
    EXPECT_EQ(to_string(order_type_t::take_profit), "TAKE_PROFIT");
    EXPECT_EQ(to_string(order_type_t::take_profit_market), "TAKE_PROFIT_MARKET");
    EXPECT_EQ(to_string(order_type_t::trailing_stop_market), "TRAILING_STOP_MARKET");

    check_json_roundtrip(order_type_t::limit, "LIMIT");
    check_json_roundtrip(order_type_t::market, "MARKET");
    check_json_roundtrip(order_type_t::stop, "STOP");
    check_json_roundtrip(order_type_t::stop_market, "STOP_MARKET");
    check_json_roundtrip(order_type_t::take_profit, "TAKE_PROFIT");
    check_json_roundtrip(order_type_t::take_profit_market, "TAKE_PROFIT_MARKET");
    check_json_roundtrip(order_type_t::trailing_stop_market, "TRAILING_STOP_MARKET");
}

// --------------------------------------------------------------------------
// time_in_force_t
// --------------------------------------------------------------------------

TEST(Enums, TimeInForce) {
    EXPECT_EQ(to_string(time_in_force_t::gtc), "GTC");
    EXPECT_EQ(to_string(time_in_force_t::ioc), "IOC");
    EXPECT_EQ(to_string(time_in_force_t::fok), "FOK");
    EXPECT_EQ(to_string(time_in_force_t::gtx), "GTX");
    EXPECT_EQ(to_string(time_in_force_t::gtd), "GTD");
    EXPECT_EQ(to_string(time_in_force_t::rpi), "RPI");

    check_json_roundtrip(time_in_force_t::gtc, "GTC");
    check_json_roundtrip(time_in_force_t::ioc, "IOC");
    check_json_roundtrip(time_in_force_t::fok, "FOK");
    check_json_roundtrip(time_in_force_t::gtx, "GTX");
    check_json_roundtrip(time_in_force_t::gtd, "GTD");
    check_json_roundtrip(time_in_force_t::rpi, "RPI");
}

// --------------------------------------------------------------------------
// kline_interval_t
// --------------------------------------------------------------------------

TEST(Enums, KlineInterval) {
    EXPECT_EQ(to_string(kline_interval_t::m1), "1m");
    EXPECT_EQ(to_string(kline_interval_t::m3), "3m");
    EXPECT_EQ(to_string(kline_interval_t::m5), "5m");
    EXPECT_EQ(to_string(kline_interval_t::m15), "15m");
    EXPECT_EQ(to_string(kline_interval_t::m30), "30m");
    EXPECT_EQ(to_string(kline_interval_t::h1), "1h");
    EXPECT_EQ(to_string(kline_interval_t::h2), "2h");
    EXPECT_EQ(to_string(kline_interval_t::h4), "4h");
    EXPECT_EQ(to_string(kline_interval_t::h6), "6h");
    EXPECT_EQ(to_string(kline_interval_t::h8), "8h");
    EXPECT_EQ(to_string(kline_interval_t::h12), "12h");
    EXPECT_EQ(to_string(kline_interval_t::d1), "1d");
    EXPECT_EQ(to_string(kline_interval_t::d3), "3d");
    EXPECT_EQ(to_string(kline_interval_t::w1), "1w");
    EXPECT_EQ(to_string(kline_interval_t::mo1), "1M");

    check_json_roundtrip(kline_interval_t::m1, "1m");
    check_json_roundtrip(kline_interval_t::m3, "3m");
    check_json_roundtrip(kline_interval_t::m5, "5m");
    check_json_roundtrip(kline_interval_t::m15, "15m");
    check_json_roundtrip(kline_interval_t::m30, "30m");
    check_json_roundtrip(kline_interval_t::h1, "1h");
    check_json_roundtrip(kline_interval_t::h2, "2h");
    check_json_roundtrip(kline_interval_t::h4, "4h");
    check_json_roundtrip(kline_interval_t::h6, "6h");
    check_json_roundtrip(kline_interval_t::h8, "8h");
    check_json_roundtrip(kline_interval_t::h12, "12h");
    check_json_roundtrip(kline_interval_t::d1, "1d");
    check_json_roundtrip(kline_interval_t::d3, "3d");
    check_json_roundtrip(kline_interval_t::w1, "1w");
    check_json_roundtrip(kline_interval_t::mo1, "1M");
}

// --------------------------------------------------------------------------
// position_side_t
// --------------------------------------------------------------------------

TEST(Enums, PositionSide) {
    EXPECT_EQ(to_string(position_side_t::both), "BOTH");
    EXPECT_EQ(to_string(position_side_t::long_side), "LONG");
    EXPECT_EQ(to_string(position_side_t::short_side), "SHORT");

    check_json_roundtrip(position_side_t::both, "BOTH");
    check_json_roundtrip(position_side_t::long_side, "LONG");
    check_json_roundtrip(position_side_t::short_side, "SHORT");
}

// --------------------------------------------------------------------------
// working_type_t
// --------------------------------------------------------------------------

TEST(Enums, WorkingType) {
    EXPECT_EQ(to_string(working_type_t::mark_price_t), "MARK_PRICE");
    EXPECT_EQ(to_string(working_type_t::contract_price), "CONTRACT_PRICE");

    check_json_roundtrip(working_type_t::mark_price_t, "MARK_PRICE");
    check_json_roundtrip(working_type_t::contract_price, "CONTRACT_PRICE");
}

// --------------------------------------------------------------------------
// response_type_t
// --------------------------------------------------------------------------

TEST(Enums, ResponseType) {
    EXPECT_EQ(to_string(response_type_t::ack), "ACK");
    EXPECT_EQ(to_string(response_type_t::result), "RESULT");

    check_json_roundtrip(response_type_t::ack, "ACK");
    check_json_roundtrip(response_type_t::result, "RESULT");
}

// --------------------------------------------------------------------------
// margin_type_t
// --------------------------------------------------------------------------

TEST(Enums, MarginType) {
    EXPECT_EQ(to_string(margin_type_t::isolated), "ISOLATED");
    EXPECT_EQ(to_string(margin_type_t::crossed), "CROSSED");

    // Glaze meta has multiple aliases; last mapping wins for serialization.
    // "ISOLATED"/"isolated" → isolated; glaze writes "isolated".
    // "CROSSED"/"cross" → crossed; glaze writes "cross".
    check_json_roundtrip(margin_type_t::isolated, "isolated");
    check_json_roundtrip(margin_type_t::crossed, "cross");
}

// --------------------------------------------------------------------------
// contract_type_t
// --------------------------------------------------------------------------

TEST(Enums, ContractType) {
    EXPECT_EQ(to_string(contract_type_t::perpetual), "PERPETUAL");
    EXPECT_EQ(to_string(contract_type_t::current_month), "CURRENT_MONTH");
    EXPECT_EQ(to_string(contract_type_t::next_month), "NEXT_MONTH");
    EXPECT_EQ(to_string(contract_type_t::current_quarter), "CURRENT_QUARTER");
    EXPECT_EQ(to_string(contract_type_t::next_quarter), "NEXT_QUARTER");
    EXPECT_EQ(to_string(contract_type_t::perpetual_delivering), "PERPETUAL_DELIVERING");
    EXPECT_EQ(to_string(contract_type_t::tradifi_perpetual), "TRADIFI_PERPETUAL");

    check_json_roundtrip(contract_type_t::perpetual, "PERPETUAL");
    check_json_roundtrip(contract_type_t::current_month, "CURRENT_MONTH");
    check_json_roundtrip(contract_type_t::next_month, "NEXT_MONTH");
    check_json_roundtrip(contract_type_t::current_quarter, "CURRENT_QUARTER");
    check_json_roundtrip(contract_type_t::next_quarter, "NEXT_QUARTER");
    check_json_roundtrip(contract_type_t::perpetual_delivering, "PERPETUAL_DELIVERING");
    check_json_roundtrip(contract_type_t::tradifi_perpetual, "TRADIFI_PERPETUAL");
}

// --------------------------------------------------------------------------
// contract_status_t
// --------------------------------------------------------------------------

TEST(Enums, ContractStatus) {
    EXPECT_EQ(to_string(contract_status_t::pending_trading), "PENDING_TRADING");
    EXPECT_EQ(to_string(contract_status_t::trading), "TRADING");
    EXPECT_EQ(to_string(contract_status_t::pre_delivering), "PRE_DELIVERING");
    EXPECT_EQ(to_string(contract_status_t::delivering), "DELIVERING");
    EXPECT_EQ(to_string(contract_status_t::delivered), "DELIVERED");
    EXPECT_EQ(to_string(contract_status_t::pre_settle), "PRE_SETTLE");
    EXPECT_EQ(to_string(contract_status_t::settling), "SETTLING");
    EXPECT_EQ(to_string(contract_status_t::close), "CLOSE");

    check_json_roundtrip(contract_status_t::pending_trading, "PENDING_TRADING");
    check_json_roundtrip(contract_status_t::trading, "TRADING");
    check_json_roundtrip(contract_status_t::pre_delivering, "PRE_DELIVERING");
    check_json_roundtrip(contract_status_t::delivering, "DELIVERING");
    check_json_roundtrip(contract_status_t::delivered, "DELIVERED");
    check_json_roundtrip(contract_status_t::pre_settle, "PRE_SETTLE");
    check_json_roundtrip(contract_status_t::settling, "SETTLING");
    check_json_roundtrip(contract_status_t::close, "CLOSE");
}

// --------------------------------------------------------------------------
// order_status_t
// --------------------------------------------------------------------------

TEST(Enums, OrderStatus) {
    EXPECT_EQ(to_string(order_status_t::new_order), "NEW");
    EXPECT_EQ(to_string(order_status_t::partially_filled), "PARTIALLY_FILLED");
    EXPECT_EQ(to_string(order_status_t::filled), "FILLED");
    EXPECT_EQ(to_string(order_status_t::canceled), "CANCELED");
    EXPECT_EQ(to_string(order_status_t::rejected), "REJECTED");
    EXPECT_EQ(to_string(order_status_t::expired), "EXPIRED");
    EXPECT_EQ(to_string(order_status_t::expired_in_match), "EXPIRED_IN_MATCH");

    check_json_roundtrip(order_status_t::new_order, "NEW");
    check_json_roundtrip(order_status_t::partially_filled, "PARTIALLY_FILLED");
    check_json_roundtrip(order_status_t::filled, "FILLED");
    check_json_roundtrip(order_status_t::canceled, "CANCELED");
    check_json_roundtrip(order_status_t::rejected, "REJECTED");
    check_json_roundtrip(order_status_t::expired, "EXPIRED");
    check_json_roundtrip(order_status_t::expired_in_match, "EXPIRED_IN_MATCH");
}

// --------------------------------------------------------------------------
// stp_mode_t
// --------------------------------------------------------------------------

TEST(Enums, StpMode) {
    EXPECT_EQ(to_string(stp_mode_t::expire_taker), "EXPIRE_TAKER");
    EXPECT_EQ(to_string(stp_mode_t::expire_both), "EXPIRE_BOTH");
    EXPECT_EQ(to_string(stp_mode_t::expire_maker), "EXPIRE_MAKER");

    check_json_roundtrip(stp_mode_t::expire_taker, "EXPIRE_TAKER");
    check_json_roundtrip(stp_mode_t::expire_both, "EXPIRE_BOTH");
    check_json_roundtrip(stp_mode_t::expire_maker, "EXPIRE_MAKER");
}

// --------------------------------------------------------------------------
// price_match_t
// --------------------------------------------------------------------------

TEST(Enums, PriceMatch) {
    EXPECT_EQ(to_string(price_match_t::none), "NONE");
    EXPECT_EQ(to_string(price_match_t::opponent), "OPPONENT");
    EXPECT_EQ(to_string(price_match_t::opponent_5), "OPPONENT_5");
    EXPECT_EQ(to_string(price_match_t::opponent_10), "OPPONENT_10");
    EXPECT_EQ(to_string(price_match_t::opponent_20), "OPPONENT_20");
    EXPECT_EQ(to_string(price_match_t::queue), "QUEUE");
    EXPECT_EQ(to_string(price_match_t::queue_5), "QUEUE_5");
    EXPECT_EQ(to_string(price_match_t::queue_10), "QUEUE_10");
    EXPECT_EQ(to_string(price_match_t::queue_20), "QUEUE_20");

    check_json_roundtrip(price_match_t::none, "NONE");
    check_json_roundtrip(price_match_t::opponent, "OPPONENT");
    check_json_roundtrip(price_match_t::opponent_5, "OPPONENT_5");
    check_json_roundtrip(price_match_t::opponent_10, "OPPONENT_10");
    check_json_roundtrip(price_match_t::opponent_20, "OPPONENT_20");
    check_json_roundtrip(price_match_t::queue, "QUEUE");
    check_json_roundtrip(price_match_t::queue_5, "QUEUE_5");
    check_json_roundtrip(price_match_t::queue_10, "QUEUE_10");
    check_json_roundtrip(price_match_t::queue_20, "QUEUE_20");
}

// --------------------------------------------------------------------------
// income_type_t
// --------------------------------------------------------------------------

TEST(Enums, IncomeType) {
    EXPECT_EQ(to_string(income_type_t::transfer), "TRANSFER");
    EXPECT_EQ(to_string(income_type_t::welcome_bonus), "WELCOME_BONUS");
    EXPECT_EQ(to_string(income_type_t::realized_pnl), "REALIZED_PNL");
    EXPECT_EQ(to_string(income_type_t::funding_fee), "FUNDING_FEE");
    EXPECT_EQ(to_string(income_type_t::commission), "COMMISSION");
    EXPECT_EQ(to_string(income_type_t::insurance_clear), "INSURANCE_CLEAR");
    EXPECT_EQ(to_string(income_type_t::referral_kickback), "REFERRAL_KICKBACK");
    EXPECT_EQ(to_string(income_type_t::commission_rebate), "COMMISSION_REBATE");
    EXPECT_EQ(to_string(income_type_t::api_rebate), "API_REBATE");
    EXPECT_EQ(to_string(income_type_t::contest_reward), "CONTEST_REWARD");
    EXPECT_EQ(to_string(income_type_t::cross_collateral_transfer), "CROSS_COLLATERAL_TRANSFER");
    EXPECT_EQ(to_string(income_type_t::options_premium_fee), "OPTIONS_PREMIUM_FEE");
    EXPECT_EQ(to_string(income_type_t::options_settle_profit), "OPTIONS_SETTLE_PROFIT");
    EXPECT_EQ(to_string(income_type_t::internal_transfer), "INTERNAL_TRANSFER");
    EXPECT_EQ(to_string(income_type_t::auto_exchange), "AUTO_EXCHANGE");
    EXPECT_EQ(to_string(income_type_t::delivered_settelment), "DELIVERED_SETTELMENT");
    EXPECT_EQ(to_string(income_type_t::coin_swap_deposit), "COIN_SWAP_DEPOSIT");
    EXPECT_EQ(to_string(income_type_t::coin_swap_withdraw), "COIN_SWAP_WITHDRAW");
    EXPECT_EQ(to_string(income_type_t::position_limit_increase_fee), "POSITION_LIMIT_INCREASE_FEE");

    check_json_roundtrip(income_type_t::transfer, "TRANSFER");
    check_json_roundtrip(income_type_t::welcome_bonus, "WELCOME_BONUS");
    check_json_roundtrip(income_type_t::realized_pnl, "REALIZED_PNL");
    check_json_roundtrip(income_type_t::funding_fee, "FUNDING_FEE");
    check_json_roundtrip(income_type_t::commission, "COMMISSION");
    check_json_roundtrip(income_type_t::insurance_clear, "INSURANCE_CLEAR");
    check_json_roundtrip(income_type_t::referral_kickback, "REFERRAL_KICKBACK");
    check_json_roundtrip(income_type_t::commission_rebate, "COMMISSION_REBATE");
    check_json_roundtrip(income_type_t::api_rebate, "API_REBATE");
    check_json_roundtrip(income_type_t::contest_reward, "CONTEST_REWARD");
    check_json_roundtrip(income_type_t::cross_collateral_transfer, "CROSS_COLLATERAL_TRANSFER");
    check_json_roundtrip(income_type_t::options_premium_fee, "OPTIONS_PREMIUM_FEE");
    check_json_roundtrip(income_type_t::options_settle_profit, "OPTIONS_SETTLE_PROFIT");
    check_json_roundtrip(income_type_t::internal_transfer, "INTERNAL_TRANSFER");
    check_json_roundtrip(income_type_t::auto_exchange, "AUTO_EXCHANGE");
    check_json_roundtrip(income_type_t::delivered_settelment, "DELIVERED_SETTELMENT");
    check_json_roundtrip(income_type_t::coin_swap_deposit, "COIN_SWAP_DEPOSIT");
    check_json_roundtrip(income_type_t::coin_swap_withdraw, "COIN_SWAP_WITHDRAW");
    check_json_roundtrip(income_type_t::position_limit_increase_fee, "POSITION_LIMIT_INCREASE_FEE");
}

// --------------------------------------------------------------------------
// futures_data_period_t
// --------------------------------------------------------------------------

TEST(Enums, FuturesDataPeriod) {
    EXPECT_EQ(to_string(futures_data_period_t::m5), "5m");
    EXPECT_EQ(to_string(futures_data_period_t::m15), "15m");
    EXPECT_EQ(to_string(futures_data_period_t::m30), "30m");
    EXPECT_EQ(to_string(futures_data_period_t::h1), "1h");
    EXPECT_EQ(to_string(futures_data_period_t::h2), "2h");
    EXPECT_EQ(to_string(futures_data_period_t::h4), "4h");
    EXPECT_EQ(to_string(futures_data_period_t::h6), "6h");
    EXPECT_EQ(to_string(futures_data_period_t::h12), "12h");
    EXPECT_EQ(to_string(futures_data_period_t::d1), "1d");

    check_json_roundtrip(futures_data_period_t::m5, "5m");
    check_json_roundtrip(futures_data_period_t::m15, "15m");
    check_json_roundtrip(futures_data_period_t::m30, "30m");
    check_json_roundtrip(futures_data_period_t::h1, "1h");
    check_json_roundtrip(futures_data_period_t::h2, "2h");
    check_json_roundtrip(futures_data_period_t::h4, "4h");
    check_json_roundtrip(futures_data_period_t::h6, "6h");
    check_json_roundtrip(futures_data_period_t::h12, "12h");
    check_json_roundtrip(futures_data_period_t::d1, "1d");
}

// --------------------------------------------------------------------------
// execution_type_t
// --------------------------------------------------------------------------

TEST(Enums, ExecutionType) {
    EXPECT_EQ(to_string(execution_type_t::new_order), "NEW");
    EXPECT_EQ(to_string(execution_type_t::partial_fill), "PARTIAL_FILL");
    EXPECT_EQ(to_string(execution_type_t::fill), "FILL");
    EXPECT_EQ(to_string(execution_type_t::canceled), "CANCELED");
    EXPECT_EQ(to_string(execution_type_t::rejected), "REJECTED");
    EXPECT_EQ(to_string(execution_type_t::expired), "EXPIRED");
    EXPECT_EQ(to_string(execution_type_t::trade), "TRADE");

    check_json_roundtrip(execution_type_t::new_order, "NEW");
    check_json_roundtrip(execution_type_t::partial_fill, "PARTIAL_FILL");
    check_json_roundtrip(execution_type_t::fill, "FILL");
    check_json_roundtrip(execution_type_t::canceled, "CANCELED");
    check_json_roundtrip(execution_type_t::rejected, "REJECTED");
    check_json_roundtrip(execution_type_t::expired, "EXPIRED");
    check_json_roundtrip(execution_type_t::trade, "TRADE");
}

// --------------------------------------------------------------------------
// rate_limit_type_t
// --------------------------------------------------------------------------

TEST(Enums, RateLimitType) {
    EXPECT_EQ(to_string(rate_limit_type_t::request_weight), "REQUEST_WEIGHT");
    EXPECT_EQ(to_string(rate_limit_type_t::orders_1s), "ORDERS_1S");
    EXPECT_EQ(to_string(rate_limit_type_t::orders_1m), "ORDERS_1M");
    EXPECT_EQ(to_string(rate_limit_type_t::orders_1h), "ORDERS_1H");
    EXPECT_EQ(to_string(rate_limit_type_t::orders_1d), "ORDERS_1D");

    check_json_roundtrip(rate_limit_type_t::request_weight, "REQUEST_WEIGHT");
    check_json_roundtrip(rate_limit_type_t::orders_1s, "ORDERS_1S");
    check_json_roundtrip(rate_limit_type_t::orders_1m, "ORDERS_1M");
    check_json_roundtrip(rate_limit_type_t::orders_1h, "ORDERS_1H");
    check_json_roundtrip(rate_limit_type_t::orders_1d, "ORDERS_1D");
}

// --------------------------------------------------------------------------
// rate_limit_interval_t
// --------------------------------------------------------------------------

TEST(Enums, RateLimitInterval) {
    EXPECT_EQ(to_string(rate_limit_interval_t::second), "SECOND");
    EXPECT_EQ(to_string(rate_limit_interval_t::minute), "MINUTE");
    EXPECT_EQ(to_string(rate_limit_interval_t::hour), "HOUR");
    EXPECT_EQ(to_string(rate_limit_interval_t::day), "DAY");

    check_json_roundtrip(rate_limit_interval_t::second, "SECOND");
    check_json_roundtrip(rate_limit_interval_t::minute, "MINUTE");
    check_json_roundtrip(rate_limit_interval_t::hour, "HOUR");
    check_json_roundtrip(rate_limit_interval_t::day, "DAY");
}

// --------------------------------------------------------------------------
// auto_close_type_t
// --------------------------------------------------------------------------

TEST(Enums, AutoCloseType) {
    EXPECT_EQ(to_string(auto_close_type_t::liquidation), "LIQUIDATION");
    EXPECT_EQ(to_string(auto_close_type_t::adl), "ADL");

    check_json_roundtrip(auto_close_type_t::liquidation, "LIQUIDATION");
    check_json_roundtrip(auto_close_type_t::adl, "ADL");
}

// --------------------------------------------------------------------------
// delta_type_t
// --------------------------------------------------------------------------

TEST(Enums, DeltaType) {
    EXPECT_EQ(to_string(delta_type_t::add), "1");
    EXPECT_EQ(to_string(delta_type_t::reduce), "2");

    check_json_roundtrip(delta_type_t::add, "1");
    check_json_roundtrip(delta_type_t::reduce, "2");
}

// --------------------------------------------------------------------------
// algo_type_t
// --------------------------------------------------------------------------

TEST(Enums, AlgoType) {
    EXPECT_EQ(to_string(algo_type_t::twap), "TWAP");
    EXPECT_EQ(to_string(algo_type_t::vp), "VP");

    check_json_roundtrip(algo_type_t::twap, "TWAP");
    check_json_roundtrip(algo_type_t::vp, "VP");
}

// --------------------------------------------------------------------------
// algo_status_t
// --------------------------------------------------------------------------

TEST(Enums, AlgoStatus) {
    EXPECT_EQ(to_string(algo_status_t::working), "WORKING");
    EXPECT_EQ(to_string(algo_status_t::cancelled), "CANCELLED");
    EXPECT_EQ(to_string(algo_status_t::rejected), "REJECTED");
    EXPECT_EQ(to_string(algo_status_t::expired), "EXPIRED");
    EXPECT_EQ(to_string(algo_status_t::triggered), "TRIGGERED");

    check_json_roundtrip(algo_status_t::working, "WORKING");
    check_json_roundtrip(algo_status_t::cancelled, "CANCELLED");
    check_json_roundtrip(algo_status_t::rejected, "REJECTED");
    check_json_roundtrip(algo_status_t::expired, "EXPIRED");
    check_json_roundtrip(algo_status_t::triggered, "TRIGGERED");
}

} // anonymous namespace
