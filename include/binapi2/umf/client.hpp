#pragma once

#include <binapi2/umf/config.hpp>
#include <binapi2/umf/rest/account.hpp>
#include <binapi2/umf/rest/market_data.hpp>
#include <binapi2/umf/rest/trade.hpp>
#include <binapi2/umf/rest/user_data_streams.hpp>
#include <binapi2/umf/signing.hpp>
#include <binapi2/umf/transport/http_client.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>

#include <string>

namespace binapi2::umf {

class client {
  public:
    client(boost::asio::io_context &io_context, config cfg);

    [[nodiscard]] config &configuration() noexcept;
    [[nodiscard]] const config &configuration() const noexcept;
    [[nodiscard]] transport::http_client &transport() noexcept;

    template <typename Response>
    [[nodiscard]] result<Response> execute(
        boost::beast::http::verb method,
        const std::string &path,
        const query_map &query,
        bool signed_request
    );

    rest::account_service account;
    rest::market_data_service market_data;
    rest::trade_service trade;
    rest::user_data_stream_service user_data_streams;

  private:
    boost::asio::io_context &io_context_;
    config cfg_;
    transport::http_client http_;
};

} // namespace binapi2::umf
