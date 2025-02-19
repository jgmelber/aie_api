// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_32_32__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_32_32__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32;

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 2, 4, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x2_2x4_conf(a.template grow<16>(), a_sign, b.template grow<16>(), b_sign, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x2_2x4(a.template grow<16>(), a_sign, b.template grow<16>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 4, 4, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [a1, a2] = interleave_unzip<TypeA, 8>::run(a.template extract<8>(0), a.template extract<8>(1), 2);

        this->data = ::mac_4x2_2x4_conf(a1.template grow<16>(), a_sign, b, b_sign, this->data, this->zero, 0, 0);
        this->data = ::mac_4x2_2x4(a2.template grow<16>(), a_sign, b.template extract<8>(1).template grow<16>(), b_sign, this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [a1, a2] = interleave_unzip<TypeA, 8>::run(a.template extract<8>(0), a.template extract<8>(1), 2);

        this->data = ::mul_4x2_2x4(a1.template grow<16>(), a_sign, b, b_sign);
        this->data = ::mac_4x2_2x4(a2.template grow<16>(), a_sign, b.template extract<8>(1).template grow<16>(), b_sign, this->data);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<8, 2, 4, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 64, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x2_2x4_conf(a,                                            a_sign, b.template grow<16>(), b_sign, this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_4x2_2x4_conf(a.template extract<8>(1).template grow<16>(), a_sign, b.template grow<16>(), b_sign, this->data[1], this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x2_2x4(a,                                            a_sign, b.template grow<16>(), b_sign);
        this->data[1] = ::mul_4x2_2x4(a.template extract<8>(1).template grow<16>(), a_sign, b.template grow<16>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 1, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 2>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 64, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // Given a = {a0, a1, a2, a3} the following broadcasts to
        //      ai = {a0, 0, 0, a0,
        //            a1, 0, 0, a1,
        //            a2, 0, 0, a2,
        //            a3, 0, 0, a3}
        // which is used as
        //     {a0,  0,                          {a0*b0, a0*b1, a0*b2, a0*b3,
        //       0, a0,  *  {b0, b1, b2, b3,  =   a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1,  0,      b4, b5, b6, b7}      a1*b0, a1*b1, a1*b2, a1*b3,
        //       0, a1}                           a1*b4, a1*b5, a1*b6, a1*b7}
        // and
        //     {a2,  0,                          {a2*b0, a2*b1, a2*b2, a2*b3,
        //       0, a2,  *  {b0, b1, b2, b3,  =   a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3,  0,      b4, b5, b6, b7}      a3*b0, a3*b1, a3*b2, a3*b3,
        //       0, a3}                           a3*b4, a3*b5, a3*b6, a3*b7}
        // Concatenating and reshaping gives the final result:
        //     {a0*b0, a0*b1, a0*b2, a0*b3, a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1*b0, a1*b1, a1*b2, a1*b3, a1*b4, a1*b5, a1*b6, a1*b7,
        //      a2*b0, a2*b1, a2*b2, a2*b3, a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3*b0, a3*b1, a3*b2, a3*b3, a3*b4, a3*b5, a3*b6, a3*b7}
        vector<TypeA, 16> ai = a.template grow<16>();
                          ai = ::sel(::shift(zeros<TypeA, 16>::run(), ai, 4), ai, 15);
                          ai = ::shuffle(ai, T32_4x4);
        this->data[0] = ::mac_4x2_2x4_conf(ai,                         a_sign, b.template grow<16>(), b_sign, this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_4x2_2x4_conf(::shuffle(ai, T256_2x2_hi), a_sign, b.template grow<16>(), b_sign, this->data[1], this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<TypeA, 16> ai = a.template grow<16>();
                          ai = ::sel(::shift(zeros<TypeA, 16>::run(), ai, 4), ai, 15);
                          ai = ::shuffle(ai, T32_4x4);
        this->data[0] = ::mul_4x2_2x4(ai,                         a_sign, b.template grow<16>(), b_sign);
        this->data[1] = ::mul_4x2_2x4(::shuffle(ai, T256_2x2_hi), a_sign, b.template grow<16>(), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, int32, 64>   : public mmul_32_32<M, K, N, int32,  int32, 64>  { using mmul_32_32<M, K, N, int32,  int32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, uint32, 64> : public mmul_32_32<M, K, N, uint32, uint32, 64> { using mmul_32_32<M, K, N, uint32, uint32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, uint32, 64>  : public mmul_32_32<M, K, N, int32,  uint32, 64> { using mmul_32_32<M, K, N, int32,  uint32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, int32, 64>  : public mmul_32_32<M, K, N, uint32, int32, 64>  { using mmul_32_32<M, K, N, uint32, int32, 64>::mmul_32_32; };

template <>
struct mmul_32_32<4, 8, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 32>;
    using vector_B_type = vector<float, 32>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
#if __AIE_API_EMULATED_FP32_ZEROIZATION__
        this->data = ::mac_4x8_8x4_conf(a.template grow<32>(), b.template grow<32>(), this->data, this->zero, 0, 0);
#else
        this->data = ::mac_4x8_8x4(a.template grow<32>(), b.template grow<32>(), this->data);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_4x8_8x4(a.template grow<32>(), b.template grow<32>());
        this->zero = false;
    }
};

template <>
struct mmul_32_32<4, 1, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 4>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
#if __AIE_API_EMULATED_FP32_ZEROIZATION__
        this->data = ::mac_elem_16_conf(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>(), this->data, this->zero, 0, 0);
#else
        this->data = ::mac_elem_16(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>(), this->data);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_elem_16(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>());
        this->zero = false;
    }
};

template <>
struct mmul_32_32<4, 1, 8, float, float, 32> : public C_block<float, float, 32, 32, 2>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 8>;

    using C_block<float, float, 32, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto x0 = broadcast<float, 8>::run(a[0]);
        auto x1 = broadcast<float, 8>::run(a[1]);
        auto x2 = broadcast<float, 8>::run(a[2]);
        auto x3 = broadcast<float, 8>::run(a[3]);
#if __AIE_API_EMULATED_FP32_ZEROIZATION__
        this->data[0] = ::mac_elem_16_conf(::concat(x0, x1), b.template grow_replicate<16>(), this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_elem_16_conf(::concat(x2, x3), b.template grow_replicate<16>(), this->data[1], this->zero, 0, 0);
#else
        this->data[0] = ::mac_elem_16(::concat(x0, x1), b.template grow_replicate<16>(), this->data[0]);
        this->data[1] = ::mac_elem_16(::concat(x2, x3), b.template grow_replicate<16>(), this->data[1]);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto x0 = broadcast<float, 8>::run(a[0]);
        auto x1 = broadcast<float, 8>::run(a[1]);
        auto x2 = broadcast<float, 8>::run(a[2]);
        auto x3 = broadcast<float, 8>::run(a[3]);
        this->data[0] = ::mul_elem_16(::concat(x0, x1), b.template grow_replicate<16>());
        this->data[1] = ::mul_elem_16(::concat(x2, x3), b.template grow_replicate<16>());
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, float, float, 32>  : public mmul_32_32<M, K, N, float, float, 32>  { using mmul_32_32<M, K, N, float, float, 32>::mmul_32_32; };

}

#endif
