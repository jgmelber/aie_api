// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_16_8__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_16_8__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_8;

template <typename TypeA, typename TypeB>
struct mmul_16_8<4, 4, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 32, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_larger_internal<TypeA, TypeB, 32, 32, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x4_4x8_conf(a.template grow<32>(), a_sign, b.template grow<64>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x4_4x8(a.template grow<32>(), a_sign, b.template grow<64>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_8<8, 4, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x4_4x8_conf(a, a_sign, b.template grow<64>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x4_4x8(a, a_sign, b.template grow<64>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_8<4, 8, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x8_8x8_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x8_8x8(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_8<2, 16, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type        = vector<TypeA, 32>;
    using vector_B_sparse_type = sparse_vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;
    using C_block_tmp = C_block<TypeA, TypeB, 64, 32, 1>;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        C_block_tmp tmp;

        tmp.data = ::mac_4x16_16x8T_conf(a.template grow<64>(), a_sign, b, b_sign, this->data.template grow<32>(), this->zero, 0, 0, 0);

        this->data = tmp.data.template extract<16>(0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        C_block_tmp tmp;

        tmp.data = ::mul_4x16_16x8T(a.template grow<64>(), a_sign, b, b_sign);

        this->data = tmp.data.template extract<16>(0);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_8<4, 16, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type        = vector<TypeA, 64>;
    using vector_B_sparse_type = sparse_vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mac_4x16_16x8T_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mul_4x16_16x8T(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, int8, AccumBits>   : public mmul_16_8<M, K, N, int16,  int8, AccumBits>  { using mmul_16_8<M, K, N, int16,  int8, AccumBits>::mmul_16_8; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, uint8, AccumBits> : public mmul_16_8<M, K, N, uint16, uint8, AccumBits> { using mmul_16_8<M, K, N, uint16, uint8, AccumBits>::mmul_16_8; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, uint8, AccumBits>  : public mmul_16_8<M, K, N, int16,  uint8, AccumBits> { using mmul_16_8<M, K, N, int16,  uint8, AccumBits>::mmul_16_8; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, int8, AccumBits>  : public mmul_16_8<M, K, N, uint16, int8, AccumBits>  { using mmul_16_8<M, K, N, uint16, int8, AccumBits>::mmul_16_8; };

}

#endif
