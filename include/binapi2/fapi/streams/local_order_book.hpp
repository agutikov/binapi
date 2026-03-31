// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/result.hpp>
#include <binapi2/fapi/types/market_data.hpp>
#include <binapi2/fapi/types/streams.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace binapi2::fapi {
class client;
}

namespace binapi2::fapi::streams {

class market_streams;

struct order_book_snapshot
{
    std::uint64_t last_update_id{};
    std::map<std::string, std::string, std::greater<>> bids{};
    std::map<std::string, std::string, std::less<>> asks{};
};

class local_order_book
{
public:
    using snapshot_callback = std::function<void(const order_book_snapshot&)>;

    local_order_book(market_streams& streams, client& rest_client) noexcept;

    [[nodiscard]] result<void> start(const std::string& symbol, int depth_limit = 1000);
    void stop();
    [[nodiscard]] order_book_snapshot snapshot() const;

    void set_snapshot_callback(snapshot_callback callback);

private:
    void fetch_snapshot();
    void apply_event(const types::depth_stream_event& event);
    void apply_levels(const std::vector<types::price_level>& levels, std::map<std::string, std::string, std::greater<>>& side);
    void apply_levels(const std::vector<types::price_level>& levels, std::map<std::string, std::string, std::less<>>& side);

    market_streams& streams_;
    client& rest_client_;
    std::string symbol_{};
    int depth_limit_{ 1000 };

    mutable std::mutex mutex_{};
    order_book_snapshot book_{};
    std::uint64_t last_u_{};
    bool synced_{ false };
    bool running_{ false };
    std::vector<types::depth_stream_event> buffer_{};
    snapshot_callback on_snapshot_{};
};

} // namespace binapi2::fapi::streams
