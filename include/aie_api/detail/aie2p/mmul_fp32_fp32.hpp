// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_FP32_FP32__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_FP32_FP32__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../transpose.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_fp32_fp32;

#if __AIE_API_FP32_EMULATION__
template <>
struct mmul_fp32_fp32<4, 8, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 32>;
    using vector_B_type = vector<float, 32>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mac_4x8_8x4_fp32(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_4x8_8x4_fp32(a, b);
        this->zero = false;
    }
};

#if __AIE_API_CRVO_9906__
template <>
struct mmul_fp32_fp32<4, 1, 4, float, float, 32> : public C_block_larger_internal<float, float, 32, 16, 2>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 4>;

    using C_block_larger_internal<float, float, 32, 16, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mac_elem_32_conf(::set_v32float(0, ::shuffle(a.template grow_replicate<16>(), T32_4x4)),
                                        b.template grow_replicate<16>().template grow<32>(),
                                        this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_elem_32(::set_v32float(0, ::shuffle(a.template grow_replicate<16>(), T32_4x4)),
                                   b.template grow_replicate<16>().template grow<32>());
        this->zero = false;
    }
};
#else
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
#endif

template <>
struct mmul_fp32_fp32<4, 1, 8, float, float, 32> : public C_block<float, float, 32, 32, 1>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 8>;

    using C_block<float, float, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto ai = transpose<float, 32>::run(a.template grow_replicate<32>(), 8, 4);
        this->data = ::mac_elem_32_conf(ai, b.template grow_replicate<32>(), this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto ai = transpose<float, 32>::run(a.template grow_replicate<32>(), 8, 4);
        this->data = ::mul_elem_32(ai, b.template grow_replicate<32>());
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, float, float, 32>  : public mmul_fp32_fp32<M, K, N, float, float, 32>  { using mmul_fp32_fp32<M, K, N, float, float, 32>::mmul_fp32_fp32; };

#if __AIE_ARCH__ == 22 && __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_cfp32_cfp32;

template <typename TypeA, typename TypeB>
struct mmul_cfp32_cfp32<2, 8, 4, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 8, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 32, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool /*a_sign*/, const vector_B_type &b, const bool /*b_sign*/)
    {
        if constexpr (std::is_same_v<TypeB, cfloat>) {
            std::array<v8cfloat, 4> b_;
            b_[0] = b.template extract<8>(0);
            b_[1] = b.template extract<8>(1);
            b_[2] = b.template extract<8>(2);
            b_[3] = b.template extract<8>(3);
            this->data = ::mac_2x8_8x4_conf(a, __builtin_bit_cast(v32cfloat, b_), this->data, this->zero, OP_TERM_NEG_COMPLEX, 0, 0);
        }
        else {
            this->data = ::mac_2x8_8x4_conf(a, b, this->data, this->zero, OP_TERM_NEG_COMPLEX, 0, 0);
        }
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool /*a_sign*/, const vector_B_type &b, const bool /*b_sign*/)
    {
        if constexpr (std::is_same_v<TypeB, cfloat>) {
            std::array<v8cfloat, 4> b_;
            b_[0] = b.template extract<8>(0);
            b_[1] = b.template extract<8>(1);
            b_[2] = b.template extract<8>(2);
            b_[3] = b.template extract<8>(3);
            this->data = ::mul_2x8_8x4(a, __builtin_bit_cast(v32cfloat, b_));
        }
        else {
            this->data = ::mul_2x8_8x4(a, b);
        }
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cfloat, cfloat, 32>  : public mmul_cfp32_cfp32<M, K, N, cfloat, cfloat, 32>  { using mmul_cfp32_cfp32<M, K, N, cfloat, cfloat, 32>::mmul_cfp32_cfp32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float, cfloat, 32>  : public mmul_cfp32_cfp32<M, K, N,  float, cfloat, 32>  { using mmul_cfp32_cfp32<M, K, N,  float, cfloat, 32>::mmul_cfp32_cfp32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cfloat,  float, 32>  : public mmul_cfp32_cfp32<M, K, N, cfloat,  float, 32>  { using mmul_cfp32_cfp32<M, K, N, cfloat,  float, 32>::mmul_cfp32_cfp32; };

#endif

#endif

} // namespace aie::detail

#endif
