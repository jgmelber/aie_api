// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_FP32_FP32__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_FP32_FP32__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

#include "emulated_mul_intrinsics.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_fp32_fp32;

template <>
struct mmul_fp32_fp32<4, 8, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 32>;
    using vector_B_type = vector<float, 32>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mac_4x8_8x4_conf(a.template grow<32>(), b.template grow<32>(), this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_4x8_8x4(a.template grow<32>(), b.template grow<32>());
        this->zero = false;
    }
};

template <>
struct mmul_fp32_fp32<4, 1, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 4>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mac_elem_16_conf(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>(), this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_elem_16(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>());
        this->zero = false;
    }
};

template <>
struct mmul_fp32_fp32<4, 1, 8, float, float, 32> : public C_block<float, float, 32, 32, 2>
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
        this->data[0] = ::mac_elem_16_conf(::concat(x0, x1), b.template grow_replicate<16>(), this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_elem_16_conf(::concat(x2, x3), b.template grow_replicate<16>(), this->data[1], this->zero, 0, 0);
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
struct mmul<M, K, N, float, float, 32>  : public mmul_fp32_fp32<M, K, N, float, float, 32>  { using mmul_fp32_fp32<M, K, N, float, float, 32>::mmul_fp32_fp32; };

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_cfp32_cfp32;

template <typename TypeA, typename TypeB>
struct mmul_cfp32_cfp32<2, 8, 2, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 4, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 32, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = mac_2x8_8x2_conf_t(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = mul_2x8_8x2_t(a, b);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cfloat, cfloat, 32>  : public mmul_cfp32_cfp32<M, K, N, cfloat, cfloat, 32>  { using mmul_cfp32_cfp32<M, K, N, cfloat, cfloat, 32>::mmul_cfp32_cfp32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float, cfloat, 32>  : public mmul_cfp32_cfp32<M, K, N,  float, cfloat, 32>  { using mmul_cfp32_cfp32<M, K, N,  float, cfloat, 32>::mmul_cfp32_cfp32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cfloat,  float, 32>  : public mmul_cfp32_cfp32<M, K, N, cfloat,  float, 32>  { using mmul_cfp32_cfp32<M, K, N, cfloat,  float, 32>::mmul_cfp32_cfp32; };

}

#endif
