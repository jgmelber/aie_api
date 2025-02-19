// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_16_16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_16_16__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16;

template <typename TypeA, typename TypeB>
struct mmul_16_16<4, 2, 8, TypeA, TypeB, 32> : public C_block_larger_internal<TypeA, TypeB, 32, 32, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block_larger_internal<TypeA, TypeB, 32, 32, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x2_2x8_conf(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x2_2x8(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_16<8, 2, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x2_2x8_conf(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x2_2x8(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<2, 4, 8, TypeA, TypeB, AccumBits> : public C_block_larger_internal<TypeA, TypeB, 64, 16, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_larger_internal<TypeA, TypeB, 64, 16, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x4_4x8_conf(a.template grow<32>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x4_4x8(a.template grow<32>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<4, 4, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x4_4x8_conf(a.template grow<32>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x4_4x8(a.template grow<32>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<2, 8, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type        = vector<TypeA, 16>;
    using vector_B_sparse_type = sparse_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;
    using C_block_tmp = C_block<TypeA, TypeB, 64, 32, 1>;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        C_block_tmp tmp;

        tmp.data = ::mac_4x8_8x8T_conf(a.template grow<32>(), a_sign, b, b_sign, this->data.template grow<32>(), this->zero, 0, 0, 0);

        this->data = tmp.data.template extract<16>(0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        C_block_tmp tmp;

        tmp.data = ::mul_4x8_8x8T(a.template grow<32>(), a_sign, b, b_sign);

        this->data = tmp.data.template extract<16>(0);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<4, 8, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type        = vector<TypeA, 32>;
    using vector_B_sparse_type = sparse_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mac_4x8_8x8T_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mul_4x8_8x8T(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_16<8, 1, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // Given a = {a0, a1, a2, a3, a4, a5, a6, a7} the following broadcasts to
        //      ai = {a0, 0, a1, 0, a2, 0, a3, 0, a4, 0, a5, 0, a6, 0, a7, 0}
        // which is used as
        //     {a0,  0,                                         {a0*b0, a0*b1, a0*b2, a0*b3, a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1,  0,                                          a1*b0, a1*b1, a1*b2, a1*b3, a1*b4, a1*b5, a1*b6, a1*b7,
        //      a2,  0,                                          a2*b0, a2*b1, a2*b2, a2*b3, a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3,  0,  *  {b0, b1, b2, b3, b4, b5, b6, b7, =   a3*b0, a3*b1, a3*b2, a3*b3, a3*b4, a3*b5, a3*b6, a3*b7,
        //      a4,  0,       0,  0,  0,  0,  0,  0,  0,  0}     a4*b0, a4*b1, a4*b2, a4*b3, a4*b4, a4*b5, a4*b6, a4*b7,
        //      a5,  0,                                          a5*b0, a5*b1, a5*b2, a5*b3, a5*b4, a5*b5, a5*b6, a5*b7,
        //      a6,  0,                                          a6*b0, a6*b1, a6*b2, a6*b3, a6*b4, a6*b5, a6*b6, a6*b7,
        //      a7,  0}                                          a7*b0, a7*b1, a7*b2, a7*b3, a7*b4, a7*b5, a7*b6, a7*b7,
        auto ai = ::shuffle(a.template grow<32>(), zeros<TypeA, 32>::run(), T16_2x32_lo);
        this->data = ::mac_8x2_2x8_conf(ai, a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto ai = ::shuffle(a.template grow<32>(), zeros<TypeA, 32>::run(), T16_2x32_lo);
        this->data = ::mul_8x2_2x8(ai, a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, int16, AccumBits>   : public mmul_16_16<M, K, N, int16,  int16, AccumBits>  { using mmul_16_16<M, K, N, int16, int16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, uint16, AccumBits> : public mmul_16_16<M, K, N, uint16, uint16, AccumBits> { using mmul_16_16<M, K, N, uint16, uint16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, uint16, AccumBits>  : public mmul_16_16<M, K, N, int16,  uint16, AccumBits> { using mmul_16_16<M, K, N, int16,  uint16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, int16, AccumBits>  : public mmul_16_16<M, K, N, uint16, int16, AccumBits>  { using mmul_16_16<M, K, N, uint16, int16, AccumBits>::mmul_16_16; };

}

#endif
