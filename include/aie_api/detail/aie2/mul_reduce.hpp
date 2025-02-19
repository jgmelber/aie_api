// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MUL_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE2_MUL_REDUCE__HPP__

#include "../interleave.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct mul_reduce_bits_impl<8, 32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret;

        if constexpr (Elems == 16 || Elems == 32) {
            vector_type tmp1 = v;

            for (unsigned s = Elems / 2; s > 0; s /= 2) {
                vector_type tmp2;

                std::tie(tmp1, tmp2) = interleave_unzip<T, Elems>::run(tmp1, vector_type(), s);

                tmp1 = mul<MulMacroOp::Mul, 32, T, T>::run(tmp1, tmp2).template to_vector<T>();
            }

            ret = tmp1[0];
        }
        else if constexpr (Elems == 64) {
            vector_type tmp1 = v;

            #pragma unroll
            for (unsigned s = Elems / 2; s > 0; s /= 2) {
                vector_type tmp2;

                std::tie(tmp1, tmp2) = interleave_unzip<T, Elems>::run(tmp1, vector_type(), s);

                tmp1 = mul<MulMacroOp::Mul, 32, T, T>::run(tmp1, tmp2).template to_vector<T>();
            }

            ret = tmp1[0];
        }
        else if constexpr (Elems == 128) {
            T a = mul_reduce<32, T, Elems / 2>::run(v.template extract<Elems / 2>(0));
            T b = mul_reduce<32, T, Elems / 2>::run(v.template extract<Elems / 2>(1));

            ret = a * b;
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct mul_reduce_bits_impl<16, 32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret;

        if constexpr (Elems == 8 || Elems == 16) {
            vector_type tmp1 = v;

            for (unsigned s = Elems / 2; s > 0; s /= 2) {
                vector_type tmp2;

                std::tie(tmp1, tmp2) = interleave_unzip<T, Elems>::run(tmp1, vector_type(), s);

                tmp1 = mul<MulMacroOp::Mul, 32, T, T>::run(tmp1, tmp2).template to_vector<T>();
            }

            ret = tmp1[0];
        }
        else if constexpr (Elems == 32) {
            vector_type tmp1 = v;

            #pragma unroll
            for (unsigned s = Elems / 2; s > 0; s /= 2) {
                vector_type tmp2;

                std::tie(tmp1, tmp2) = interleave_unzip<T, Elems>::run(tmp1, vector_type(), s);

                tmp1 = mul<MulMacroOp::Mul, 32, T, T>::run(tmp1, tmp2).template to_vector<T>();
            }

            ret = tmp1[0];
        }
        else if constexpr (Elems == 64) {
            T a = mul_reduce<32, T, Elems / 2>::run(v.template extract<Elems / 2>(0));
            T b = mul_reduce<32, T, Elems / 2>::run(v.template extract<Elems / 2>(1));

            ret = a * b;
        }

        return ret;
    }
};

// Re-enable when emulated suppport for 32-bit x 32-bit mul is added
#if 0
template <typename T, unsigned Elems>
struct mul_reduce_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        T ret;

        if constexpr (Elems == 8) {
            vector_type tmp1 = v;

            for (unsigned s = 4; s > 0; s /= 2) {
                vector_type tmp2;

                std::tie(tmp1, tmp2) = interleave_unzip<T, 8>::run(tmp1, vector_type(), s);

                tmp1 = mul_bits_impl<32, T, 8>::run(tmp1, tmp2).to_vector();
            }

            ret = tmp1[0];
        }
        else if constexpr (Elems == 16) {
            vector<T, Elems / 2> tmp = mul_bits_impl<32, T, Elems / 2>::run(v.template extract<Elems / 2>(0),
                                                                            v.template extract<Elems / 2>(1)).to_vector();
            ret = mul_reduce_bits_impl<32, T, Elems / 2>::run(tmp);
        }
        else if constexpr (Elems == 32) {
            vector<T, Elems / 4> tmp1 = mul_bits_impl<32, T, Elems / 4>::run(v.template extract<Elems / 4>(0),
                                                                             v.template extract<Elems / 4>(1)).to_vector();
            vector<T, Elems / 4> tmp2 = mul_bits_impl<32, T, Elems / 4>::run(v.template extract<Elems / 4>(2),
                                                                             v.template extract<Elems / 4>(3)).to_vector();

            tmp1 = mul_bits_impl<32, T, Elems / 4>::run(tmp1, tmp2).to_vector();

            ret = mul_reduce_bits_impl<32, T, Elems / 4>::run(tmp1);
        }

        return ret;
    }
};
#endif

}

#endif
