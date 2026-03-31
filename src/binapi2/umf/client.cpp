#include <binapi2/umf/client.hpp>

namespace binapi2::umf {

client::client(boost::asio::io_context& io_context, config cfg) :
    account(*this), market_data(*this), trade(*this), user_data_streams(*this), io_context_(io_context), cfg_(std::move(cfg)),
    http_(io_context_, cfg_)
{
}

config&
client::configuration() noexcept
{
    return cfg_;
}

const config&
client::configuration() const noexcept
{
    return cfg_;
}

boost::asio::io_context&
client::context() noexcept
{
    return io_context_;
}

transport::http_client&
client::transport() noexcept
{
    return http_;
}

} // namespace binapi2::umf
