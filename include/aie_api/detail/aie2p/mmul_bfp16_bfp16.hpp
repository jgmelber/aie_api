// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_BFP16_BFP16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_BFP16_BFP16__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_bfp16_bfp16;

template <>
struct mmul_bfp16_bfp16<8, 8, 8, bfp16ebs8, bfp16ebs8, 32> : public C_block<bfp16ebs8, bfp16ebs8, 32, 64, 1>
{
    using TypeA = bfp16ebs8;
    using TypeB = bfp16ebs8;

    using vector_A_type = block_vector<TypeA, 64>;
    using vector_B_type = block_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x8_8x8T(a, b);
        this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x8_8x8T_conf(a, a_sign,  b, b_sign, this->data, this->zero, 0, 0);
        this->zero = false;
    }
};

template <>
struct mmul_bfp16_bfp16<8, 8, 16, bfp16ebs8, bfp16ebs8, 32> : public C_block<bfp16ebs8, bfp16ebs8, 32, 128, 2>
{
    using TypeA = bfp16ebs8;
    using TypeB = bfp16ebs8;

    using vector_A_type = block_vector<TypeA, 64>;
    using vector_B_type = block_vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 32, 128, 2>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
         this->data[0] = ::mul_4x8_8x16T(a,                                      b);
         this->data[1] = ::mul_4x8_8x16T(::shuffle(a, T256_2x2_hi | T32_2x2_hi), b); //TODO: Swap in combined BFP mode when it exists in the core (CRVO-4895)
         this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x8_8x16T_conf(a,                                      a_sign, b, b_sign, this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_4x8_8x16T_conf(::shuffle(a, T256_2x2_hi | T32_2x2_hi), a_sign, b, b_sign, this->data[1], this->zero, 0, 0);
        this->zero = false;
    }
};

template <>
struct mmul_bfp16_bfp16<8, 8, 8, bfloat16, bfp16ebs8, 32> : public C_block<bfloat16, bfp16ebs8, 32, 64, 1>
{
    using TypeA = bfloat16;
    using TypeB = bfp16ebs8;

    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = block_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 64> acc_a(a);
        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a);

        this->data = ::mul_8x8_8x8T(tmp_a, b);
        this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 64> acc_a(a);
        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a);

        this->data = ::mac_8x8_8x8T_conf(tmp_a, b, this->data, this->zero, 0, 0);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfp16ebs8,  bfp16ebs8,  32> : public mmul_bfp16_bfp16<M, K, N, bfp16ebs8,  bfp16ebs8,  32> { using mmul_bfp16_bfp16<M, K, N, bfp16ebs8,  bfp16ebs8,  32>::mmul_bfp16_bfp16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfp16ebs16, bfp16ebs16, 32> : public mmul_bfp16_bfp16<M, K, N, bfp16ebs16, bfp16ebs16, 32> { using mmul_bfp16_bfp16<M, K, N, bfp16ebs16, bfp16ebs16, 32>::mmul_bfp16_bfp16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat16, bfp16ebs8, 32> : public mmul_bfp16_bfp16<M, K, N, bfloat16, bfp16ebs8, 32> { using mmul_bfp16_bfp16<M, K, N, bfloat16, bfp16ebs8, 32>::mmul_bfp16_bfp16; };

}

#endif
