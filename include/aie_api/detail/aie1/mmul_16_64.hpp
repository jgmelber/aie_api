// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_16_64__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_16_64__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <unsigned AccumBits, bool IsAcc>
static constexpr auto get_mmul_16_64_op()
{
    if constexpr (IsAcc) {
        if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac4(params...);  };
        else                           return [](auto ... params) { return ::lmac4(params...); };
    }
    else {
        if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mul4(params...);  };
        else                           return [](auto ... params) { return ::lmul4(params...); };
    }
}

template <unsigned M, unsigned K, unsigned N, typename T1, typename T2, unsigned AccumBits>
struct mmul_16_64;

template <unsigned AccumBits>
struct mmul_16_64<2, 4, 2, int16, cint32, AccumBits> : public C_block<int16, cint32, AccumBits, 4, 1>
{
    using vector_A_type = vector<int16, 8>;
    using vector_B_type = vector<cint32, 8>;

    using C_block<int16, cint32, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mac_op(this->data,
                            a.template grow<32>(),    0, 0x4400, 1,
                            b.template extract<4>(0), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    2, 0x4400, 1,
                            b.template extract<4>(1), 0, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_64_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mul_op(a.template grow<32>(),    0, 0x4400, 1,
                            b.template extract<4>(0), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    2, 0x4400, 1,
                            b.template extract<4>(1), 0, 0x1010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_16_64<2, 8, 2, int16, cint32, AccumBits> : public C_block<int16, cint32, AccumBits, 4, 1>
{
    using vector_A_type = vector<int16,  16>;
    using vector_B_type = vector<cint32, 16>;

    using C_block<int16, cint32, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mac_op(this->data,
                            a.template grow<32>(),    0, 0x8800, 1,
                            b.template extract<4>(0), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    2, 0x8800, 1,
                            b.template extract<4>(1), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    4, 0x8800, 1,
                            b.template extract<4>(2), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    6, 0x8800, 1,
                            b.template extract<4>(3), 0, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_64_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mul_op(a.template grow<32>(),    0, 0x8800, 1,
                            b.template extract<4>(0), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    2, 0x8800, 1,
                            b.template extract<4>(1), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    4, 0x8800, 1,
                            b.template extract<4>(2), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a.template grow<32>(),    6, 0x8800, 1,
                            b.template extract<4>(3), 0, 0x1010, 2);
    }
};


template <unsigned AccumBits>
struct mmul_16_64<4, 4, 2, int16, cint32, AccumBits> : public C_block<int16, cint32, AccumBits, 8, 2>
{
    using vector_A_type = vector<int16,  16>;
    using vector_B_type = vector<cint32, 8>;

    using C_block<int16, cint32, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),     0, 0x4400, 1,
                               b.template extract<4>(0),  0, 0x1010, 2);
        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),     2, 0x4400, 1,
                               b.template extract<4>(1),  0, 0x1010, 2);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),     8, 0x4400, 1,
                               b.template extract<4>(0),  0, 0x1010, 2);
        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),    10, 0x4400, 1,
                               b.template extract<4>(1),  0, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_64_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data[0] = mul_op(a.template grow<32>(),     0, 0x4400, 1,
                               b.template extract<4>(0),  0, 0x1010, 2);
        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),     2, 0x4400, 1,
                               b.template extract<4>(1),  0, 0x1010, 2);

        this->data[1] = mul_op(a.template grow<32>(),     8, 0x4400, 1,
                               b.template extract<4>(0),  0, 0x1010, 2);
        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),    10, 0x4400, 1,
                               b.template extract<4>(1),  0, 0x1010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_16_64<2, 4, 4, int16, cint32, AccumBits> : public C_block<int16, cint32, AccumBits, 8, 2>
{
    using vector_A_type = vector<int16,  8>;
    using vector_B_type = vector<cint32, 16>;

    using C_block<int16, cint32, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data[0] = mac_op(this->data[0],
                               b.template extract<8>(0), 0, 0x3210, 4,
                               a.template grow<16>(),    0, 0x0000, 1);
        this->data[0] = mac_op(this->data[0],
                               b.template extract<8>(1), 0, 0x3210, 4,
                               a.template grow<16>(),    2, 0x0000, 1);

        this->data[1] = mac_op(this->data[1],
                               b.template extract<8>(0), 0, 0x3210, 4,
                               a.template grow<16>(),    4, 0x0000, 1);
        this->data[1] = mac_op(this->data[1],
                               b.template extract<8>(1), 0, 0x3210, 4,
                               a.template grow<16>(),    6, 0x0000, 1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_64_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data[0] = mul_op(b.template extract<8>(0), 0, 0x3210, 4,
                               a.template grow<16>(),    0, 0x0000, 1);
        this->data[0] = mac_op(this->data[0],
                               b.template extract<8>(1), 0, 0x3210, 4,
                               a.template grow<16>(),    2, 0x0000, 1);

        this->data[1] = mul_op(b.template extract<8>(0), 0, 0x3210, 4,
                               a.template grow<16>(),    4, 0x0000, 1);
        this->data[1] = mac_op(this->data[1],
                               b.template extract<8>(1), 0, 0x3210, 4,
                               a.template grow<16>(),    6, 0x0000, 1);
    }
};

template <unsigned AccumBits>
struct mmul_16_64<4, 4, 1, int16, cint32, AccumBits> : public C_block<int16, cint32, AccumBits, 4, 1>
{
    using vector_A_type = vector<int16,  16>;
    using vector_B_type = vector<cint32, 4>;

    using C_block<int16, cint32, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mac_op(this->data,
                            a.template grow<32>(), 0, 0xc840, 1,
                            b,                     0, 0x0000, 1);

        this->data = mac_op(this->data,
                            a.template grow<32>(), 2, 0xc840, 1,
                            b,                     2, 0x0000, 1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_64_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_16_64_op<AccumBits, true>();

        this->data = mul_op(a.template grow<32>(), 0, 0xc840, 1,
                            b,                     0, 0x0000, 1);

        this->data = mac_op(this->data,
                            a.template grow<32>(), 2, 0xc840, 1,
                            b,                     2, 0x0000, 1);
    }
};

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, cint32, AccumBits> : public mmul_16_64<M, K, N, int16, cint32, AccumBits> { using mmul_16_64<M, K, N, int16, cint32, AccumBits>::mmul_16_64; };

}

#endif
