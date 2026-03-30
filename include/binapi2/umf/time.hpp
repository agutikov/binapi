#pragma once

#include <chrono>
#include <cstdint>

namespace binapi2::umf {

[[nodiscard]] inline std::uint64_t current_timestamp_ms() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

} // namespace binapi2::umf
