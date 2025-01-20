// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_16_32__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_16_32__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_32;

template <typename TypeA, typename TypeB>
struct mmul_16_32<4, 4, 4, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x4_4x4_conf(a.template grow<32>(), a_sign, b, b_sign, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x4_4x4(a.template grow<32>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_32<2, 4, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_2x4_4x8_conf(a.template grow<32>(), a_sign, b.template extract<16>(0), b.template extract<16>(1), b_sign, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_2x4_4x8(a.template grow<32>(), a_sign, b.template extract<16>(0), b.template extract<16>(1), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, int32, 64>   : public mmul_16_32<M, K, N, int16,  int32, 64>  { using mmul_16_32<M, K, N, int16,  int32, 64>::mmul_16_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint16, uint32, 64> : public mmul_16_32<M, K, N, uint16, uint32, 64> { using mmul_16_32<M, K, N, uint16, uint32, 64>::mmul_16_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, uint32, 64>  : public mmul_16_32<M, K, N, int16,  uint32, 64> { using mmul_16_32<M, K, N, int16,  uint32, 64>::mmul_16_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint16, int32, 64>  : public mmul_16_32<M, K, N, uint16, int32, 64>  { using mmul_16_32<M, K, N, uint16, int32, 64>::mmul_16_32; };

}

#endif
