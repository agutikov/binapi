// SPDX-License-Identifier: Apache-2.0
//
// async-recorder — detail monitor REST depth snapshot.

#include "snapshot.hpp"

#include "helpers.hpp"

#include <binapi2/fapi/rest/services/market_data.hpp>
#include <binapi2/fapi/types/market_data.hpp>

#include <glaze/glaze.hpp>

#include <spdlog/spdlog.h>

#include <fstream>

namespace types = ::binapi2::fapi::types;

namespace binapi2::examples::async_recorder::detail_impl {

boost::cobalt::task<bool>
fetch_depth_snapshot(::binapi2::fapi::rest::client& client,
                     const std::filesystem::path& root,
                     const std::string& symbol_upper,
                     std::string_view tag)
{
    types::order_book_request_t req;
    req.symbol = symbol_upper;
    req.limit = 1000;

    auto r = co_await client.market_data.async_execute(req);
    if (!r) {
        spdlog::warn("detail[{}]: depth snapshot failed: {}",
                     symbol_upper, r.err.message);
        co_return false;
    }

    auto json = glz::write_json(*r);
    if (!json) {
        spdlog::warn("detail[{}]: depth snapshot serialise failed",
                     symbol_upper);
        co_return false;
    }

    const auto dir = root / "detail" / symbol_upper / "depth_snapshot";
    std::filesystem::create_directories(dir);
    const auto path = dir / (std::string(tag) + "." + now_filename_ts() + ".json");

    std::ofstream out(path);
    if (!out) {
        spdlog::warn("detail[{}]: cannot open {} for write",
                     symbol_upper, path.string());
        co_return false;
    }
    out << "{\"t\":\"" << now_iso_ms() << "\",\"d\":" << *json << "}\n";
    co_return true;
}

} // namespace binapi2::examples::async_recorder::detail_impl
