// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE_DETAIL_BIT__HPP__
#define __AIE_API_AIE_DETAIL_BIT__HPP__

#include "vector.hpp"

namespace aie::detail {

enum class BitOp {
    And,
    Not,
    Or,
    Xor,
};

// TODO: implement element/vector, element_ref/vector variants

template <unsigned TypeBits, typename T, unsigned Elems, BitOp Op>
struct bit_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == BitOp::And)
                ret[i] = v1[i] & v2[i];
            else if constexpr (Op == BitOp::Or)
                ret[i] = v1[i] | v2[i];
            else if constexpr (Op == BitOp::Xor)
                ret[i] = v1[i] ^ v2[i];
        }

        return ret;
    }


    static vector_type run(T a, const vector_type &v)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == BitOp::And)
                ret[i] = a & v[i];
            else if constexpr (Op == BitOp::Or)
                ret[i] = a | v[i];
            else if constexpr (Op == BitOp::Xor)
                ret[i] = a ^ v[i];
        }

        return ret;
    }

    static vector_type run(const vector_type &v, T a)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == BitOp::And)
                ret[i] = v[i] & a;
            else if constexpr (Op == BitOp::Or)
                ret[i] = v[i] | a;
            else if constexpr (Op == BitOp::Xor)
                ret[i] = v[i] ^ a;
        }

        return ret;
    }

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            if constexpr (Op == BitOp::Not)
                ret[i] = ~v[i];
        }

        return ret;
    }
#endif
};

template <unsigned TypeBits, typename T, unsigned Elems, BitOp Op>
struct bit_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(v1, v2);
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }

    static vector_type run(const T &a, const vector_type &v)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(a, v);
    }

    static vector_type run(const vector_type &v, const T &a)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(v, a);
    }

    static vector_type run(const vector_type &v)
    {
        return bit_bits_impl<TypeBits, T, Elems, Op>::run(v);
    }
};

template <typename T, unsigned Elems, BitOp Op>
using bit = bit_bits<type_bits_v<T>, T, Elems, Op>;

}

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/bit.hpp"

#endif

#endif
