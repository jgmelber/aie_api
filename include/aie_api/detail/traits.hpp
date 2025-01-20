// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_TRAITS_ACCUM__HPP__
#define __AIE_API_DETAIL_TRAITS_ACCUM__HPP__

namespace aie::detail {

/**
 * Similar to std::tuple_element but does not require a tuple instantiation
 */
template <unsigned I, typename... Args>
    requires(I < sizeof...(Args))
struct pack_element;

template <typename Arg0, typename... Args>
struct pack_element<0, Arg0, Args...> { using type = Arg0; };

template <typename Arg0, typename Arg1, typename... Args>
struct pack_element<1, Arg0, Arg1, Args...> { using type = Arg1; };

template <typename Arg0, typename Arg1, typename Arg2, typename... Args>
struct pack_element<2, Arg0, Arg1, Arg2, Args...> { using type = Arg2; };

template <unsigned I, typename Arg0, typename Arg1, typename Arg2, typename... Args>
    requires(I > 2)
struct pack_element<I, Arg0, Arg1, Arg2, Args...> : pack_element<I - 3, Args...> {};

template <unsigned I, typename... Args>
using pack_element_t = typename pack_element<I, Args...>::type;

/**
 * Checks that an object type has a non-overloaded function call operator.
 *
 * If the operator is overloaded, `&T::operator()` expression will not resolve to a function pointer correctly.
 *
 * Lambda expressions satisfy this requirement.
 */
template <typename T>
concept Callable = requires {
    { &T::operator() };
};

/** Returns the I-th argument's type of a lambda function object call operator. */
template <unsigned I, typename T> struct invoke_arg;

/** Returns the result type of a lambda function object call operator. */
template <typename T> struct invoke_result;

template <unsigned I, typename Result, typename Functor, typename... Args>
    requires(I < sizeof...(Args))
struct invoke_arg<I, Result (Functor::*)(Args...) const> {
    using type = pack_element_t<I, Args...>;
};

template <unsigned I, typename Functor>
    requires(Callable<Functor>)
struct invoke_arg<I, Functor> : invoke_arg<I, decltype(&Functor::operator())> {};

template <unsigned I, typename T>
using invoke_arg_t = typename invoke_arg<I, T>::type;

template <typename Result, typename Functor, typename... Args>
struct invoke_result<Result (Functor::*)(Args...) const> {
    using type = Result;
};

template <typename T>
using invoke_result_t = typename invoke_result<T>::type;

} // namespace aie::detail

#endif

