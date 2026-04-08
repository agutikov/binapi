// SPDX-License-Identifier: Apache-2.0

/// @file enum_set.hpp
/// @brief Bitset-backed set of enum values with compile-time overflow protection.

#pragma once

#include <glaze/glaze.hpp>

#include <bitset>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

namespace binapi2::fapi::types {

/// Compile-time capacity for an enum_set: the number of distinct enum values
/// declared in the glaze metadata (including aliases).  For sequential enums
/// starting at 0, this equals max_value + 1.
///
/// Usage: enum_set<my_enum> automatically sizes its bitset to enum_capacity<my_enum>.
template<typename E>
inline constexpr std::size_t enum_capacity =
    std::tuple_size_v<decltype(glz::meta<E>::value.value)> / 2;

/// Compile-time check that every enum value in the glaze metadata fits within
/// the given Capacity.  Fires a static_assert at instantiation time.
template<typename E, std::size_t Capacity>
consteval bool enum_values_fit()
{
    constexpr auto& tup = glz::meta<E>::value.value;
    constexpr auto N = std::tuple_size_v<std::decay_t<decltype(tup)>>;
    // Iterate value elements (odd indices: 1, 3, 5, …).
    return []<std::size_t... I>(std::index_sequence<I...>) {
        return ((static_cast<std::size_t>(std::get<2 * I + 1>(tup)) < Capacity) && ...);
    }(std::make_index_sequence<N / 2>{});
}

/// A compact, bitset-backed set of enum values.
///
/// @tparam E        An enum type with sequential values starting at 0 and
///                  glaze metadata (glz::meta<E> with enumerate()).
/// @tparam Capacity Maximum number of distinct values.  Defaults to the
///                  count derived from glaze metadata.
///
/// Example:
/// @code
///   enum_set<trading_permission> perms;
///   perms.add(trading_permission::grid);
///   perms.add(trading_permission::copy);
///   assert(perms.contains(trading_permission::grid));
///   assert(perms.size() == 2);
///   for (auto p : perms) { ... }
/// @endcode
template<typename E, std::size_t Capacity = enum_capacity<E>>
class enum_set_t
{
    static_assert(std::is_enum_v<E>, "enum_set_t requires an enum type");
    static_assert(Capacity > 0, "enum_set_t capacity must be positive");
    static_assert(enum_values_fit<E, Capacity>(),
                  "enum values exceed enum_set_t capacity — increase Capacity or check enum definition");

    std::bitset<Capacity> bits_{};

    static constexpr std::size_t idx(E e) noexcept
    {
        return static_cast<std::size_t>(e);
    }

public:
    constexpr enum_set_t() = default;

    constexpr enum_set_t(std::initializer_list<E> init)
    {
        for (auto e : init)
            add(e);
    }

    constexpr bool contains(E e) const noexcept
    {
        auto i = idx(e);
        return i < Capacity && bits_.test(i);
    }

    constexpr void add(E e) noexcept
    {
        auto i = idx(e);
        assert(i < Capacity);
        bits_.set(i);
    }

    constexpr void remove(E e) noexcept
    {
        auto i = idx(e);
        assert(i < Capacity);
        bits_.reset(i);
    }

    constexpr std::size_t size() const noexcept { return bits_.count(); }
    constexpr bool empty() const noexcept { return bits_.none(); }

    constexpr bool operator==(const enum_set_t&) const = default;

    /// Forward iterator over the set values.
    class iterator
    {
        const std::bitset<Capacity>* bits_{};
        std::size_t pos_{};

        void advance() noexcept
        {
            while (pos_ < Capacity && !bits_->test(pos_))
                ++pos_;
        }

    public:
        using value_type = E;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator() = default;
        iterator(const std::bitset<Capacity>& b, std::size_t start)
            : bits_(&b), pos_(start) { advance(); }

        E operator*() const noexcept { return static_cast<E>(pos_); }

        iterator& operator++() noexcept { ++pos_; advance(); return *this; }
        iterator operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const iterator& o) const noexcept { return pos_ == o.pos_; }
        bool operator!=(const iterator& o) const noexcept { return pos_ != o.pos_; }
    };

    iterator begin() const noexcept { return {bits_, 0}; }
    iterator end() const noexcept { return {bits_, Capacity}; }
};

} // namespace binapi2::fapi::types
