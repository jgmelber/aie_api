// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_8_4__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_8_4__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_8_4;

#if __AIE_ARCH__ == 21

template <typename TypeA, typename TypeB>
struct mmul_8_4<4, 16, 16, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 256>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x16_16x16_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x16_16x16(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

#elif __AIE_ARCH__ == 22

template <typename TypeA, typename TypeB>
struct mmul_8_4<4, 16, 16, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 256>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto [a_left, a_right] = interleave_unzip<TypeA, 64>::run(a, a, 8);

        this->data = ::mac_4x8_8x16_conf(a_left,  a_sign, b.template extract<128>(0).unpack_sign(b_sign), b_sign, this->data, this->zero, 0, 0, 0);
        this->data = ::mac_4x8_8x16_conf(a_right, a_sign, b.template extract<128>(1).unpack_sign(b_sign), b_sign, this->data, 0, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto [a_left, a_right] = interleave_unzip<TypeA, 64>::run(a, a, 8);

        this->data = ::mul_4x8_8x16     (a_left,  a_sign, b.template extract<128>(0).unpack_sign(b_sign), b_sign);
        this->data = ::mac_4x8_8x16_conf(a_right, a_sign, b.template extract<128>(1).unpack_sign(b_sign), b_sign, this->data, 0, 0, 0, 0);
        this->zero = false;
    }
};

// TODO: CRVO-5281: Emulate AIE2+ mixed precision mmul modes on AIE2ps

template <typename TypeA, typename TypeB>
struct mmul_8_4<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_8x8_8x8_conf(a, a_sign, b.unpack_sign(b_sign), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_8x8_8x8(a, a_sign, b.unpack_sign(b_sign), b_sign);
        this->zero = false;
    }
};

#endif

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, int4, 32>   : public mmul_8_4<M, K, N, int8,  int4, 32>  { using mmul_8_4<M, K, N, int8,  int4, 32>::mmul_8_4; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, uint4, 32> : public mmul_8_4<M, K, N, uint8, uint4, 32> { using mmul_8_4<M, K, N, uint8, uint4, 32>::mmul_8_4; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, uint4, 32>  : public mmul_8_4<M, K, N, int8,  uint4, 32> { using mmul_8_4<M, K, N, int8,  uint4, 32>::mmul_8_4; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, int4, 32>  : public mmul_8_4<M, K, N, uint8, int4, 32>  { using mmul_8_4<M, K, N, uint8, int4, 32>::mmul_8_4; };

}

#endif
