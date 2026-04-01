// SPDX-License-Identifier: Apache-2.0
//
// binapi2 USD-M Futures client library.

/// @file result.hpp
/// @brief Lightweight result monad (`result<T>`) used as the return type for
///        all fallible fapi operations.

#pragma once

#include <binapi2/fapi/error.hpp>

#include <memory>
#include <optional>
#include <utility>

namespace binapi2::fapi {

/// @brief Holds either a success value of type @p T or an `error`.
///
/// Modelled after `std::expected` but available in C++20.  Use the bool
/// conversion to check for success, then dereference with `*` or `->` to
/// access the payload.
///
/// @tparam T  The success payload type.
template<typename T>
struct result
{
    std::optional<T> value{};
    error err{};

    /// @brief Test whether the operation succeeded (no error).
    [[nodiscard]] explicit operator bool() const noexcept { return err.code == error_code::none; }

    /// @brief Access the success value (undefined behaviour if `!*this`).
    [[nodiscard]] T& operator*() noexcept { return *value; }

    /// @brief Access the success value (const overload).
    [[nodiscard]] const T& operator*() const noexcept { return *value; }

    /// @brief Pointer-style access to the success value.
    [[nodiscard]] T* operator->() noexcept { return std::addressof(*value); }

    /// @brief Pointer-style access to the success value (const overload).
    [[nodiscard]] const T* operator->() const noexcept { return std::addressof(*value); }

    /// @brief Construct a successful result that owns @p v.
    /// @param v  The value to store.
    /// @return   A result in the success state.
    static result success(T v)
    {
        result r;
        r.value = std::move(v);
        return r;
    }

    /// @brief Construct a failed result carrying the given error.
    /// @param e  The error descriptor.
    /// @return   A result in the failure state.
    static result failure(error e)
    {
        result r;
        r.err = std::move(e);
        return r;
    }
};

/// @brief Specialisation of `result` for operations that produce no value on
///        success (e.g. fire-and-forget commands).
template<>
struct result<void>
{
    error err{};

    /// @brief Test whether the operation succeeded.
    [[nodiscard]] explicit operator bool() const noexcept { return err.code == error_code::none; }

    /// @brief Construct a successful void result.
    static result success() { return {}; }

    /// @brief Construct a failed void result carrying the given error.
    /// @param e  The error descriptor.
    /// @return   A result in the failure state.
    static result failure(error e)
    {
        result r;
        r.err = std::move(e);
        return r;
    }
};

} // namespace binapi2::fapi
