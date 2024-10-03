// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_TRANSPOSE__HPP__
#define __AIE_API_DETAIL_TRANSPOSE__HPP__

#include "vector.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct transpose_bits_scalar
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned row, unsigned col)
    {
        vector<T, Elems> ret;

        unsigned out_idx = 0;

        for (unsigned c = 0; c < col; ++c) {
            for (unsigned r = 0; r < row; ++r) {
                ret[out_idx++] = v[r * col + c];
            }
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct transpose_bits_impl
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned row, unsigned col)
    {
        return transpose_bits_scalar<TypeBits, T, Elems>::run(v, row, col);
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct transpose_bits
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned row, unsigned col)
    {
        const unsigned _row  = chess_manifest(row)? row : chess_manifest(col)? Elems / col : row;

        if (chess_manifest(_row == 1 || _row == Elems) || Elems == 2)
            return v;

        return transpose_bits_impl<TypeBits, T, Elems>::run(v, _row, col);
    }
};

template <typename T, unsigned Elems>
using transpose = transpose_bits<type_bits_v<T>, T, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/transpose.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/transpose.hpp"

#endif

#endif
