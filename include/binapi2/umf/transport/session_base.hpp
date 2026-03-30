#pragma once

#include <binapi2/umf/config.hpp>

#include <utility>

namespace binapi2::umf::transport {

class session_base {
  public:
    explicit session_base(config cfg)
        : cfg_(std::move(cfg)) {}

    virtual ~session_base() = default;

  protected:
    config cfg_;
};

} // namespace binapi2::umf::transport
