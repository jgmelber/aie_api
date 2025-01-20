// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_NEG__HPP__
#define __AIE_API_DETAIL_NEG__HPP__

#include <cmath>
#include <cstdlib>

#include "vector.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct neg_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        using namespace std;

        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (vector_type::is_signed())
                ret[i] = -(T)v[i];
            else
                ret[i] = v[i];
        }

        return ret;
    }
#endif
};

template <unsigned AccumBits, typename T, unsigned Elems>
struct neg_acc_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using accum_type = accum<T, Elems>;

    static accum_type run(const accum_type &acc)
    {
        using namespace std;

        accum_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            ret[i] = -acc[i];
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct neg_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        return neg_bits_impl<TypeBits, T, Elems>::run(v);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct neg_acc_bits
{
    using accum_type = accum<T, Elems>;

    static accum_type run(const accum_type &acc)
    {
        return neg_acc_bits_impl<TypeBits, T, Elems>::run(acc);
    }
};

template <typename T, unsigned Elems>
using neg = neg_bits<type_bits_v<T>, T, Elems>;

template <unsigned AccumBits, typename T, unsigned Elems>
using neg_acc = neg_acc_bits<AccumBits, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/neg.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/neg.hpp"

#endif

#endif
