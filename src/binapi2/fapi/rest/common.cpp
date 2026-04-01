// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#include "common.hpp"

#include <binapi2/fapi/query.hpp>

namespace binapi2::fapi::rest::detail {

query_map
make_futures_data_query(const types::futures_data_request& request)
{
    return to_query_map(request);
}

} // namespace binapi2::fapi::rest::detail
