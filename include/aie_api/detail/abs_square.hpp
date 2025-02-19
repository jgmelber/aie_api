// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ABS_SQUARE__HPP__
#define __AIE_API_DETAIL_ABS_SQUARE__HPP__

#include <cmath>
#include <cstdlib>

#include "vector.hpp"

namespace aie::detail {

template <typename T, typename TR, unsigned Elems>
struct abs_square_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using return_type = vector<TR, Elems>;
    using vector_type = vector<T, Elems>;

    static return_type run(const vector_type &v, int shift)
    {
        return_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            T val = v[i];
            if constexpr (vector_type::is_floating_point()) {
                ret[i] = (val.real * val.real + val.imag * val.imag);
            }
            else {
                if (shift >= 0)
                    ret[i] = (val.real * val.real + val.imag * val.imag) << shift;
                else
                    ret[i] = (val.real * val.real + val.imag * val.imag) >> (-shift);
            }
        }

        return ret;
    }
#endif
};

template <typename T, typename TR, unsigned Elems>
struct abs_square_bits
{
    using return_type = vector<TR, Elems>;
    using vector_type = vector<T,  Elems>;

    static return_type run(const vector_type &v, int shift)
    {
        return abs_square_bits_impl<T, TR, Elems>::run(v, shift);
    }
};

template <typename T, typename TR, unsigned Elems>
using abs_square = abs_square_bits<T, TR, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/abs_square.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

//TODO: Implement abs_square on AIE2
//#include "aie2/abs.hpp"

#endif

#endif
