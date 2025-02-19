// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_CONJ__HPP__
#define __AIE_API_DETAIL_CONJ__HPP__

#include <cmath>
#include <cstdlib>

#include "vector.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct conj_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        using namespace std;

        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            const T val = v[i];
            ret[i] = T(val.real, -val.imag);
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct conj_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        return conj_bits_impl<TypeBits, T, Elems>::run(v);
    }
};

template <typename T, unsigned Elems>
using conj = conj_bits<type_bits_v<T>, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/conj.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/conj.hpp"

#endif

#endif
