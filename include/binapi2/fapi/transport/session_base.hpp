// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/config.hpp>

#include <utility>

namespace binapi2::fapi::transport {

class session_base
{
public:
    explicit session_base(config cfg) : cfg_(std::move(cfg)) {}

    virtual ~session_base() = default;

protected:
    config cfg_;
};

} // namespace binapi2::fapi::transport
