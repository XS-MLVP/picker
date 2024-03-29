// Copyright (C) 2016-2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TYPE_SAFE_DETAIL_ASSIGN_OR_CONSTRUCT_HPP_INCLUDED
#define TYPE_SAFE_DETAIL_ASSIGN_OR_CONSTRUCT_HPP_INCLUDED

#if defined(TYPE_SAFE_IMPORT_STD_MODULE)
import std;
#else
#include <type_traits>
#endif

namespace type_safe
{
namespace detail
{
    // std::is_assignable but without user-defined conversions
    template <typename T, typename Arg>
    struct is_direct_assignable
    {
        template <typename U>
        struct consume_udc
        {
            operator U() const;
        };

        template <typename U>
        static std::true_type check(decltype(std::declval<T&>() = std::declval<consume_udc<U>>(),
                                             0)*);

        template <typename U>
        static std::false_type check(...);

        static constexpr bool value = decltype(check<Arg>(0))::value;
    };
} // namespace detail
} // namespace type_safe

#endif // TYPE_SAFE_DETAIL_ASSIGN_OR_CONSTRUCT_HPP_INCLUDED
