// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_32_16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_32_16__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

#include "emulated_mmul_intrinsics.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_16;

template <typename TypeA, typename TypeB>
struct mmul_32_16<4, 2, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x2_2x8_conf(a.template grow<16>(), a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x2_2x8(a.template grow<16>(), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_16<2, 4, 8, TypeA, TypeB, 64> : public C_block_larger_internal<TypeA, TypeB, 64, 16, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_larger_internal<TypeA, TypeB, 64, 16, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
#if __AIE_API_HAS_32x16_MMUL_INTRINSICS__
        this->data = ::mac_4x4_4x8_conf(a.template grow<16>(), a_sign, b, b_sign, this->data, this->zero, 0, 0);
#else
        this->data = mac_4x4_4x8_32bx16b(a.template grow<16>(), a_sign, b, b_sign, this->data, this->zero);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
#if __AIE_API_HAS_32x16_MMUL_INTRINSICS__
        this->data = ::mul_4x4_4x8(a.template grow<16>(), a_sign, b, b_sign);
#else
        this->data = mul_4x4_4x8_32bx16b(a.template grow<16>(), a_sign, b, b_sign);
#endif
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_16<4, 4, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
#if __AIE_API_HAS_32x16_MMUL_INTRINSICS__
        this->data = ::mac_4x4_4x8_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0);
#else
        this->data = mac_4x4_4x8_32bx16b(a.template grow<16>(), a_sign, b, b_sign, this->data, this->zero);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
#if __AIE_API_HAS_32x16_MMUL_INTRINSICS__
        this->data = ::mul_4x4_4x8(a, a_sign, b, b_sign);
#else
        this->data = mul_4x4_4x8_32bx16b(a.template grow<16>(), a_sign, b, b_sign);
#endif
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_16<4, 1, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // Given a = {a0, a1, a2, a3} the following broadcasts to
        //      ai = {a0, 0, a1, 0, a2, 0, a3, 0}
        // which is used as
        //     {a0,  0,                                         {a0*b0, a0*b1, a0*b2, a0*b3, a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1,  0,  *  {b0, b1, b2, b3, b4, b5, b6, b7, =   a1*b0, a1*b1, a1*b2, a1*b3, a1*b4, a1*b5, a1*b6, a1*b7,
        //      a2,  0,       0,  0,  0,  0,  0,  0,  0,  0}     a2*b0, a2*b1, a2*b2, a2*b3, a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3,  0}                                          a3*b0, a3*b1, a3*b2, a3*b3, a3*b4, a3*b5, a3*b6, a3*b7,
        vector<TypeA, 16> ai = ::shuffle(a.template grow<16>(), zeros<TypeA, 16>::run(), T32_2x16_lo);
        this->data = ::mac_4x2_2x8_conf(ai, a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<TypeA, 16> ai = ::shuffle(a.template grow<16>(), zeros<TypeA, 16>::run(), T32_2x16_lo);
        this->data = ::mul_4x2_2x8(ai, a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, int16, 64>   : public mmul_32_16<M, K, N, int32,  int16, 64>  { using mmul_32_16<M, K, N, int32,  int16, 64>::mmul_32_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, uint16, 64> : public mmul_32_16<M, K, N, uint32, uint16, 64> { using mmul_32_16<M, K, N, uint32, uint16, 64>::mmul_32_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, uint16, 64>  : public mmul_32_16<M, K, N, int32,  uint16, 64> { using mmul_32_16<M, K, N, int32,  uint16, 64>::mmul_32_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, int16, 64>  : public mmul_32_16<M, K, N, uint32, int16, 64>  { using mmul_32_16<M, K, N, uint32, int16, 64>::mmul_32_16; };

}

#endif
