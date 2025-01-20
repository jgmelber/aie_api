// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_SQUARE__HPP__
#define __AIE_API_DETAIL_SQUARE__HPP__

#include <cstdlib>
#include <cmath>
#include <climits>

#include "mul.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned AccumBits, unsigned TypeBits, typename T>
struct square_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using accum_tag = accum_tag_for_type<T, AccumBits>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        return mul<MulOp, AccumBits, T, T>::run(v, is_signed_v<T>, v, is_signed_v<T>, acc...);
    }
#endif
};

template <MulMacroOp MulOp, unsigned AccumBits, unsigned TypeBits, typename T>
struct square_bits
{
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static auto run(const vector_type<Elems> &v, const Acc &... acc)
    {
#if __AIE_ARCH__ == 10
        return square_bits_impl<MulOp, AccumBits, TypeBits, T>::run(v, acc...);
#else
        return mul<MulOp, AccumBits, T, T>::run(v, is_signed_v<T>, v, is_signed_v<T>, acc...);
#endif
    }
};

template <MulMacroOp MulOp, unsigned AccumBits, typename T>
using square = square_bits<MulOp, AccumBits, type_bits_v<T>, T>;

}

#if __AIE_ARCH__ == 10

#include "aie1/square.hpp"

#endif

#endif
