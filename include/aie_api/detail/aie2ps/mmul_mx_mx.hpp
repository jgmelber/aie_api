// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2PS_MMUL_MX_MX__HPP__
#define __AIE_API_DETAIL_AIE2PS_MMUL_MX_MX__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_mx_mx;

template <typename T> 
struct mmul_mx_mx<4, 16, 16, T, T, 32> : public C_block<T, T, 32, 64, 1>
{
    using vector_A_type = block_vector<T, 64>;
    using vector_B_type = block_vector<T, 256>;

    using C_block<T, T, 32, 64, 1>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x16_16x16T(a.template grow<128>(), b);
        this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x16_16x16T_conf(a.template grow<128>(), 1, b, 1, this->data, this->zero, 0, 0);
        this->zero = false;
    }
};

template <typename T> requires (utils::is_one_of_v<T, mx4, mx6>)
struct mmul_mx_mx<8, 16, 16, T, T, 32> : public C_block<T, T, 32, 128, 2>
{
    using vector_A_type = block_vector<T, 128>;
    using vector_B_type = block_vector<T, 256>;

    using C_block<T, T, 32, 128, 2>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x16_16x16T(a,                                              b);
        this->data[1] = ::mul_4x16_16x16T(a.template extract<64>(1).template grow<128>(), b);
        this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x16_16x16T_conf(a,                                              1, b, 1, this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_4x16_16x16T_conf(a.template extract<64>(1).template grow<128>(), 1, b, 1, this->data[1], this->zero, 0, 0);
        this->zero = false;
    }
};

template <> 
struct mmul_mx_mx<4, 16, 16, mx9, mx9, 32> : public C_block<mx9, mx9, 32, 64, 1>
{
    using T             = mx9;
    using vector_A_type = block_vector<T, 64>;
    using vector_B_type = block_vector<T, 256>;

    using C_block<T, T, 32, 64, 1>::C_block;

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector_A_type as = ::shuffle(a, T64_4x2);
        vector_B_type bs;

        bs.insert<64>(0, ::shuffle(b.template extract<64>(0), b.template extract<64>(1), T64_8x2_lo));
        bs.insert<64>(1, ::shuffle(b.template extract<64>(2), b.template extract<64>(3), T64_8x2_lo));
        bs.insert<64>(2, ::shuffle(b.template extract<64>(0), b.template extract<64>(1), T64_8x2_hi));
        bs.insert<64>(3, ::shuffle(b.template extract<64>(2), b.template extract<64>(3), T64_8x2_hi));

        this->data = ::mul_4x16_16x16T(as, bs);
        this->zero = false;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector_A_type as = ::shuffle(a, T64_4x2);
        vector_B_type bs;

        bs.insert<64>(0, ::shuffle(b.template extract<64>(0), b.template extract<64>(1), T64_8x2_lo));
        bs.insert<64>(1, ::shuffle(b.template extract<64>(2), b.template extract<64>(3), T64_8x2_lo));
        bs.insert<64>(2, ::shuffle(b.template extract<64>(0), b.template extract<64>(1), T64_8x2_hi));
        bs.insert<64>(3, ::shuffle(b.template extract<64>(2), b.template extract<64>(3), T64_8x2_hi));

        this->data = ::mac_4x16_16x16T_conf(as, 1, bs, 1, this->data, this->zero, 0, 0);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, mx4,  mx4, 32> : public mmul_mx_mx<M, K, N, mx4, mx4, 32> { using mmul_mx_mx<M, K, N, mx4, mx4, 32>::mmul_mx_mx; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, mx6,  mx6, 32> : public mmul_mx_mx<M, K, N, mx6, mx6, 32> { using mmul_mx_mx<M, K, N, mx6, mx6, 32>::mmul_mx_mx; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, mx9,  mx9, 32> : public mmul_mx_mx<M, K, N, mx9, mx9, 32> { using mmul_mx_mx<M, K, N, mx9, mx9, 32>::mmul_mx_mx; };

}

#endif
