// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_REVERSE__HPP__
#define __AIE_API_DETAIL_REVERSE__HPP__

#include "vector.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct reverse_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i)
            ret[i] = v[Elems - i - 1];

        return ret;
    }
#endif
};

template <typename T, unsigned Elems>
struct reverse
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        return reverse_impl<type_bits_v<T>, T, Elems>::run(v);
    }
};

}

#if __AIE_ARCH__ == 10

#include "aie1/reverse.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/reverse.hpp"

#elif __AIE_ARCH__ == 21

#include "aie2p/reverse.hpp"
#endif

#endif
