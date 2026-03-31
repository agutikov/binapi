#include "common.hpp"

namespace binapi2::umf::rest::detail {

query_map
make_futures_data_query(const types::futures_data_request& request)
{
    query_map query{ { "symbol", request.symbol }, { "period", to_string(request.period) } };
    if (request.limit) {
        query["limit"] = std::to_string(*request.limit);
    }
    if (request.startTime) {
        query["startTime"] = std::to_string(*request.startTime);
    }
    if (request.endTime) {
        query["endTime"] = std::to_string(*request.endTime);
    }
    return query;
}

} // namespace binapi2::umf::rest::detail
