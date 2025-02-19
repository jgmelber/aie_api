// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_8_8__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_8_8__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_8_8;

template <typename TypeA, typename TypeB>
struct mmul_8_8<2, 8, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 16, 4>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block_larger_internal<TypeA, TypeB, 32, 16, 4>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x8_8x8_conf(a.template grow<64>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x8_8x8(a.template grow<64>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 8, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 32, 2>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block_larger_internal<TypeA, TypeB, 32, 32, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x8_8x8_conf(a.template grow<64>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x8_8x8(a.template grow<64>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 16, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 32, 2>
{
    using vector_A_type        = vector<TypeA, 64>;
    using vector_B_sparse_type = sparse_vector<TypeB, 128>;

    using C_block_larger_internal<TypeA, TypeB, 32, 32, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mac_8x16_16x8T_conf(a.template grow<128>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mul_8x16_16x8T(a.template grow<128>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<8, 16, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type        = vector<TypeA, 128>;
    using vector_B_sparse_type = sparse_vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mac_8x16_16x8T_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mul_8x16_16x8T(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x8_8x8_conf(a.template grow<64>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x8_8x8(a.template grow<64>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, int8, 32>   : public mmul_8_8<M, K, N, int8,  int8, 32>  { using mmul_8_8<M, K, N, int8,  int8, 32>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, uint8, 32> : public mmul_8_8<M, K, N, uint8, uint8, 32> { using mmul_8_8<M, K, N, uint8, uint8, 32>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, uint8, 32>  : public mmul_8_8<M, K, N, int8,  uint8, 32> { using mmul_8_8<M, K, N, int8,  uint8, 32>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, int8, 32>  : public mmul_8_8<M, K, N, uint8, int8, 32>  { using mmul_8_8<M, K, N, uint8, int8, 32>::mmul_8_8; };

}

#endif
