// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

#pragma once

#include <binapi2/fapi/error.hpp>

#include <memory>
#include <optional>
#include <utility>

namespace binapi2::fapi {

template<typename T>
struct result
{
    std::optional<T> value{};
    error err{};

    [[nodiscard]] explicit operator bool() const noexcept { return err.code == error_code::none; }

    [[nodiscard]] T& operator*() noexcept { return *value; }

    [[nodiscard]] const T& operator*() const noexcept { return *value; }

    [[nodiscard]] T* operator->() noexcept { return std::addressof(*value); }

    [[nodiscard]] const T* operator->() const noexcept { return std::addressof(*value); }

    static result success(T v)
    {
        result r;
        r.value = std::move(v);
        return r;
    }

    static result failure(error e)
    {
        result r;
        r.err = std::move(e);
        return r;
    }
};

template<>
struct result<void>
{
    error err{};

    [[nodiscard]] explicit operator bool() const noexcept { return err.code == error_code::none; }

    static result success() { return {}; }

    static result failure(error e)
    {
        result r;
        r.err = std::move(e);
        return r;
    }
};

} // namespace binapi2::fapi
