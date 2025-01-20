// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_BF16_BF16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_BF16_BF16__HPP__

#include "../broadcast.hpp"
#include "../transpose.hpp"
#include "../vector_accum_cast.hpp"
#include "emulated_mmul_intrinsics.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_bf16_bf16;

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<8, 8, 4, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mac_8x8_8x4_bf16(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mul_8x8_8x4_bf16(a, b);
        this->zero = false;
    }
};

//TODO: This could be emulated more efficiently, but will still under-utilize the hardware versus larger shapes
template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<4, 8, 4, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 16, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mac_8x8_8x4_bf16(a.template grow<64>(), b, this->data.template grow<32>(), this->zero).template extract<16>(0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mul_8x8_8x4_bf16(a.template grow<64>(), b).template extract<16>(0);
        this->zero = false;
    }
};

#if AIE_API_EMULATE_BFLOAT16_MMUL_WITH_BFP16

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<4, 8, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 32, 2>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block_larger_internal<TypeA, TypeB, 32, 32, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 32> acc_a(a);
        accum<accfloat, 64> acc_b(transpose<TypeB, 64>::run(b, 8, 8));

        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a.grow<64>());
        v64bfp16ebs8 tmp_b = ::to_v64bfp16ebs8(acc_b);

        this->data = ::mac_8x8_8x8T_conf(tmp_a, tmp_b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 32> acc_a(a);
        accum<accfloat, 64> acc_b(transpose<TypeB, 64>::run(b, 8, 8));

        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a.grow<64>());
        v64bfp16ebs8 tmp_b = ::to_v64bfp16ebs8(acc_b);

        this->data = ::mul_8x8_8x8T(tmp_a, tmp_b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 64> acc_a(a);
        auto bt = transpose<TypeB, 64>::run(b, 8, 8);
        accum<accfloat, 64> acc_b = ::mul_elem_64(bt, ::concat(::broadcast_one_to_v32bfloat16(), ::broadcast_one_to_v32bfloat16()));

        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a);
        v64bfp16ebs8 tmp_b = ::to_v64bfp16ebs8(acc_b);

        this->data = ::mac_8x8_8x8T_conf(tmp_a, tmp_b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        accum<accfloat, 64> acc_a(a);
        auto bt = transpose<TypeB, 64>::run(b, 8, 8);
        accum<accfloat, 64> acc_b = ::mul_elem_64(bt, ::concat(::broadcast_one_to_v32bfloat16(), ::broadcast_one_to_v32bfloat16()));

        v64bfp16ebs8 tmp_a = ::to_v64bfp16ebs8(acc_a);
        v64bfp16ebs8 tmp_b = ::to_v64bfp16ebs8(acc_b);

        this->data = ::mul_8x8_8x8T(tmp_a, tmp_b);
        this->zero = false;
    }
};

#else

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 2>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;
    using C_block<TypeA, TypeB, 32, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = mac_4x8_8x8_bf16(a.template extract<32>(0), b, this->data[0], this->zero);
        this->data[1] = mac_4x8_8x8_bf16(a.template extract<32>(1), b, this->data[1], this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = mul_4x8_8x8_bf16(a.template extract<32>(0), b);
        this->data[1] = mul_4x8_8x8_bf16(a.template extract<32>(1), b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<4, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mac_4x8_8x8_bf16(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = mul_4x8_8x8_bf16(a, b);
        this->zero = false;
    }
};

#endif

template <typename TypeA, typename TypeB>
struct mmul_bf16_bf16<8, 1, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    template <MulMacroOp Op>
    using mul_op = mul_bits<Op, 32, 16, TypeA, 16, TypeB>;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_elem_64_conf(transpose<TypeA, 64>::run(a.template grow_replicate<64>(), 8, 8), true,
                                        b.template grow_replicate<64>(), true,
                                        this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_elem_64(transpose<TypeA, 64>::run(a.template grow_replicate<64>(), 8, 8),
                                   b.template grow_replicate<64>());
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat16, bfloat16, 32> : public mmul_bf16_bf16<M, K, N, bfloat16, bfloat16, 32> { using mmul_bf16_bf16<M, K, N, bfloat16, bfloat16, 32>::mmul_bf16_bf16; };

}

#endif
