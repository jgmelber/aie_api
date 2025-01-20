// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_8_4__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_8_4__HPP__

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

template <typename TypeA, typename TypeB>
struct mmul_8_4<4, 16, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x16_16x8_conf(a, a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x16_16x8(a, a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_4<8, 16, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 2>
{
    using vector_A_type = vector<TypeA, 128>;
    using vector_B_type = vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 32, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x16_16x8_conf(a.template extract<64>(0), a_sign, b.template grow<128>(), b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_4x16_16x8_conf(a.template extract<64>(1), a_sign, b.template grow<128>(), b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x16_16x8(a.template extract<64>(0), a_sign, b.template grow<128>(), b_sign);
        this->data[1] = ::mul_4x16_16x8(a.template extract<64>(1), a_sign, b.template grow<128>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_4<4, 32, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 128>;
    using vector_B_type = vector<TypeB, 256>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto [tmp1, tmp2] = interleave_unzip<TypeA, 64>::run(a.template extract<64>(0), a.template extract<64>(1), 16);

        this->data = ::mac_4x16_16x8_conf(tmp1, a_sign, b.template extract<128>(0), b_sign, this->data, this->zero, 0, 0, 0);
        this->data = ::mac_4x16_16x8     (tmp2, a_sign, b.template extract<128>(1), b_sign, this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto [tmp1, tmp2] = interleave_unzip<TypeA, 64>::run(a.template extract<64>(0), a.template extract<64>(1), 16);

        this->data = ::mul_4x16_16x8(tmp1, a_sign, b.template extract<128>(0), b_sign);
        this->data = ::mac_4x16_16x8(tmp2, a_sign, b.template extract<128>(1), b_sign, this->data);
        this->zero = false;
    }
};

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
