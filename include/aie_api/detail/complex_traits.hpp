// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_COMPLEX_TRAITS__HPP__
#define __AIE_API_DETAIL_COMPLEX_TRAITS__HPP__

#include "accum.hpp"
#include "../concepts.hpp"

namespace aie::detail::utils {

template <typename T>
struct get_complex_component_type;

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

template <> struct get_complex_component_type<cint16>  { using type = int16_t; };
template <> struct get_complex_component_type<cint32>  { using type = int32_t; };

#if AIE_API_MATH_VERSION >= 100 || __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <> struct get_complex_component_type<cbfloat16> { using type = bfloat16; };
#endif
template <> struct get_complex_component_type<cfloat>    { using type = float;    };
#endif

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__

/*
 * Obtain the type of the real and imaginary components of a given complex type
 */
template <typename T>
using get_complex_component_type_t = typename get_complex_component_type<T>::type;


} // namespace aie::detail::utils

namespace aie::detail {

template <typename T>
struct remove_complex;

template <ElemBaseType T>
    requires(is_complex_v<T>)
struct remove_complex<T> : utils::get_complex_component_type<T> {};

template <ElemBaseType T>
    requires(!is_complex_v<T>)
struct remove_complex<T> { using type = T; };

template <AccumElemBaseType T>
struct remove_complex<T> {
    using type = accum_tag_t<is_floating_point_class(accum_class_for_tag<T>::value())
                                ? AccumClass::FP
                                : AccumClass::Int,
                             accum_bits_for_tag_v<T>>;
};

template <Accum A>
    requires(A::value_class() == AccumClass::CInt)
struct remove_complex<A> {
    using value_type = typename remove_complex<typename A::value_type>::type;
    using type = accum<value_type, A::size()>;
};

template <Vector V>
    requires(V::is_complex())
struct remove_complex<V> {
    using value_type = utils::get_complex_component_type_t<typename V::value_type>;
    using type = vector<value_type, V::size()>;
};

template <typename T>
using remove_complex_t = typename remove_complex<T>::type;

template <typename T>
struct add_complex;

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

template <> struct add_complex<int16> { using type = cint16; };
template <> struct add_complex<int32> { using type = cint32; };
#if __AIE_API_CBF16_SUPPORT__
template <> struct add_complex<bfloat16> { using type = cbfloat16; };
#endif
#if AIE_API_MATH_VERSION >= 100 || __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct add_complex<float> { using type = cfloat; };
#endif

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__

template <AccumElemBaseType T>
struct add_complex<T> {
    using type = accum_tag_t<is_floating_point_class(accum_class_for_tag<T>::value())
                                ? AccumClass::CFP
                                : AccumClass::CInt,
                             accum_bits_for_tag_v<T>>;
};

template <Accum T>
struct add_complex<T> { using type = accum<typename add_complex<typename T::value_type>::type, T::size()>; };

template <Vector T>
struct add_complex<T> { using type = vector<typename add_complex<typename T::value_type>::type, T::size()>; };

template <typename T>
using add_complex_t = typename add_complex<T>::type;

} // namespace aie::detail

#endif // __AIE_API_DETAIL_COMPLEX_TRAITS__HPP__
