// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ABS_SQUARE__HPP__
#define __AIE_API_DETAIL_AIE2_ABS_SQUARE__HPP__

#include "../../accum.hpp"
#include "../../vector.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <typename T, typename TR, unsigned Elems>
struct abs_square_common
{
    using vector_type = vector<T, Elems>;
    using real_type   = utils::get_complex_component_type_t<T>;

    static constexpr unsigned accum_bits = default_accum_bits<real_type, real_type>();

    static_assert(is_floating_point_v<T> == is_floating_point_v<TR>,
                  "Cannot convert between int and floating point types");

    __aie_inline
    static auto run(const vector_type &v, int shift)
    {
        if constexpr (vector_type::bits() == 128) {
            return abs_square_common<T, TR, Elems * 2>::run(v.template grow<Elems * 2>(), shift).template extract<Elems>(0);
        }
        else {
            auto [re, im] = unzip_complex(v);

            auto acc = mul<MulMacroOp::Mul,     accum_bits, real_type, real_type>::run(re, true, re, true, false);
                 acc = mul<MulMacroOp::Add_Mul, accum_bits, real_type, real_type>::run(im, true, im, true, false, acc);

            return acc.template to_vector<TR>(shift);
        }
    }
};

template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cint16, TR, Elems> : abs_square_common<cint16, TR, Elems> {};

template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cint32, TR, Elems> : abs_square_common<cint32, TR, Elems> {};

#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cbfloat16, TR, Elems> : abs_square_common<cbfloat16, TR, Elems> {};
#endif

template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cfloat, TR, Elems> : abs_square_common<cfloat, TR, Elems> {};
#endif

}

#endif
