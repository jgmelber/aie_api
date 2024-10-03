// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ELEMENTARY__HPP__
#define __AIE_API_DETAIL_ELEMENTARY__HPP__

#include <cmath>

#include "vector.hpp"
#include "accum.hpp"

namespace aie::detail {

enum class ElementaryOp {
    Sqrt,
    Inv,
    InvSqrt,
    SinCos,
    SinCosComplex,
    Sin,
    Cos,
    Fix2Float,
    Float2Fix,
    Tanh,
    Exp2
};

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T>
struct elementary_bits_impl
{
    static TR run(const T &a, int shift = 0)
    {
        if constexpr (Op == ElementaryOp::Sqrt)
            return std::sqrt(a);
        else if constexpr (Op == ElementaryOp::Inv)
            return T(1)/a;
        else if constexpr (Op == ElementaryOp::InvSqrt)
            return T(1)/std::sqrt(a);
        else if constexpr (Op == ElementaryOp::Fix2Float)
            return TR(a >> shift);
        else if constexpr (Op == ElementaryOp::Float2Fix)
            return TR(a) << shift;
        else
            UNREACHABLE_MSG("Unsupported operation");
        // TODO: CRVO-7950: add support for tanh/exp2 emulation once support is added to libm.a
    }
};

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T, unsigned N>
struct elementary_vector_bits_impl
{
    using vector_type     = vector<T, N>;
    using vector_ret_type = vector<TR, N>;

    static auto run(const vector_type &v, int shift = 0, bool sign = vector_type::is_signed())
    {
        vector_ret_type ret;

        // Ensure that each subvector in a loop is at least 128b
        constexpr unsigned bits = std::min(vector_type::bits(), vector_ret_type::bits());

        // AIE1 supports element insert/extract for 128b only. Later architectures support it for 512b.
        constexpr unsigned native_insert_bits = (__AIE_ARCH__ == 10)? 128 : 512;
        constexpr unsigned num_loops = bits < native_insert_bits? 1 :
                                                                  bits / native_insert_bits;

        constexpr unsigned elems_per_loop = N / num_loops;

        // Apply the elementary operation to each element. Create as many loops as subvectors that can be indexed
        // efficiently.
        if constexpr (bits > native_insert_bits) {
            utils::unroll_times<num_loops>([&](auto idx) __aie_inline {
                ret.insert(idx, elementary_vector_bits_impl<Op, TypeBits, TR, T, elems_per_loop>::run(v.template extract<elems_per_loop>(idx)));
            });
        }
        else {
            for (unsigned i = 0; i < N; ++i) {
                const T val = v[i];
                TR out_val;

                if constexpr (is_complex_v<T>) {
                    if constexpr (is_floating_point_v<T>) {
                        out_val.real = elementary_bits_impl<Op, 32, TR, T>::run(val.real, shift);
                        out_val.imag = elementary_bits_impl<Op, 32, TR, T>::run(val.imag, shift);
                    }
                    else {
                        out_val.real = elementary_bits_impl<Op, 32, TR, uint32>::run(uint32(val.real), shift);
                        out_val.imag = elementary_bits_impl<Op, 32, TR, uint32>::run(uint32(val.imag), shift);
                    }
                }
                else {
                    if constexpr (is_floating_point_v<T>)
                        out_val = elementary_bits_impl<Op, 32, TR, T>::run(val, shift);
                    else
                        out_val = elementary_bits_impl<Op, 32, TR, uint32>::run(uint32(val), shift);
                }

                ret[i] = out_val;
            }
        }

        return ret;
    }
};

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T, unsigned N>
struct elementary_acc_bits_impl;

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T>
struct elementary_bits
{
    __aie_inline
    static auto run(const T &a, int shift = 0)
    {
        return elementary_bits_impl<Op, TypeBits, TR, T>::run(a, shift);
    }
};

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T, unsigned N>
struct elementary_vector_bits
{
    using vector_type = vector<T, N>;

    __aie_inline
    static auto run(const vector_type &v, int shift = 0, bool sign = vector_type::is_signed())
    {
        return elementary_vector_bits_impl<Op, TypeBits, TR, T, N>::run(v, shift, sign);
    }
};

template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T, unsigned N>
struct elementary_acc_bits
{
    using accum_type = accum<T, N>;

    __aie_inline
    static auto run(const accum_type &acc, int shift = 0, bool sign_dummy = false)
    {
        return elementary_acc_bits_impl<Op, TypeBits, TR, T, N>::run(acc, shift);
    }
};

template <ElementaryOp Op, typename TR, typename T>
using elementary        = elementary_bits<Op, type_bits_v<T>, TR, T>;

template <ElementaryOp Op, typename TR, typename T, unsigned N>
using elementary_vector = elementary_vector_bits<Op, type_bits_v<T>, TR, T, N>;

template <ElementaryOp Op, typename TR, typename T, unsigned N>
using elementary_acc    = elementary_acc_bits<Op, accum<T, N>::accum_bits(), TR, T, N>;

}

#if __AIE_ARCH__ == 10

#include "aie1/elementary.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/elementary.hpp"

#endif

#endif
