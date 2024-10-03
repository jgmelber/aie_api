// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_64_16__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_64_16__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"
#include "../add.hpp"

namespace aie::detail {

template <unsigned AccumBits, bool IsAcc>
static constexpr auto get_mmul_64_16_op()
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

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_64_16;

template <unsigned AccumBits>
struct mmul_64_16<2, 4, 2, cint32, int16, AccumBits> : public C_block<cint32, int16, AccumBits, 4, 1>
{
    using vector_A_type = vector<cint32, 8>;
    using vector_B_type = vector<int16,  8>;

    using C_block<cint32, int16, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data = mac_op(this->data,
                            a,                     0, 0x4400, 1,
                            b.template grow<16>(), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a,                     2, 0x4400, 1,
                            b.template grow<16>(), 4, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_64_16_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data = mul_op(a,                     0, 0x4400, 1,
                            b.template grow<16>(), 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a,                     2, 0x4400, 1,
                            b.template grow<16>(), 4, 0x1010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_64_16<2, 8, 2, cint32, int16, AccumBits> : public C_block<cint32, int16, AccumBits, 4, 1>
{
    using vector_A_type = vector<cint32, 16>;
    using vector_B_type = vector<int16,  16>;

    using C_block<cint32, int16, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data = mac_op(this->data,
                            a, 0, 0x8800, 1,
                            b, 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a, 2, 0x8800, 1,
                            b, 4, 0x1010, 2);

        this->data = mac_op(this->data,
                            a, 4, 0x8800, 1,
                            b, 8, 0x1010, 2);

        this->data = mac_op(this->data,
                            a,  6, 0x8800, 1,
                            b, 12, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_64_16_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data = mul_op(a, 0, 0x8800, 1,
                            b, 0, 0x1010, 2);

        this->data = mac_op(this->data,
                            a, 2, 0x8800, 1,
                            b, 4, 0x1010, 2);

        this->data = mac_op(this->data,
                            a, 4, 0x8800, 1,
                            b, 8, 0x1010, 2);

        this->data = mac_op(this->data,
                            a,  6, 0x8800, 1,
                            b, 12, 0x1010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_64_16<4, 4, 2, cint32, int16, AccumBits> : public C_block<cint32, int16, AccumBits, 8, 2>
{
    using vector_A_type = vector<cint32, 16>;
    using vector_B_type = vector<int16,  8>;

    using C_block<cint32, int16, AccumBits, 8, 2>::C_block;

    //TODO: check if this gets better performance by varying start offset from 0 instead of so many constant offsets
    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template extract<8>(0),  0, 0x4400, 1,
                               b.template grow<16>(),     0, 0x1010, 2);
        this->data[0] = mac_op(this->data[0],
                               a.template extract<8>(0),  2, 0x4400, 1,
                               b.template grow<16>(),     4, 0x1010, 2);

        this->data[1] = mac_op(this->data[1],
                               a.template extract<8>(1),  0, 0x4400, 1,
                               b.template grow<16>(),     0, 0x1010, 2);
        this->data[1] = mac_op(this->data[1],
                               a.template extract<8>(1),  2, 0x4400, 1,
                               b.template grow<16>(),     4, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_64_16_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data[0] = mul_op(a.template extract<8>(0),  0, 0x4400, 1,
                               b.template grow<16>(),     0, 0x1010, 2);
        this->data[0] = mac_op(this->data[0],
                               a.template extract<8>(0),  2, 0x4400, 1,
                               b.template grow<16>(),     4, 0x1010, 2);

        this->data[1] = mul_op(a.template extract<8>(1),  0, 0x4400, 1,
                               b.template grow<16>(),     0, 0x1010, 2);
        this->data[1] = mac_op(this->data[1],
                               a.template extract<8>(1),  2, 0x4400, 1,
                               b.template grow<16>(),     4, 0x1010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_64_16<2, 4, 4, cint32, int16, AccumBits> : public C_block<cint32, int16, AccumBits, 8, 2>
{
    using vector_A_type = vector<cint32, 8>;
    using vector_B_type = vector<int16,  16>;

    using C_block<cint32, int16, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_mmul_64_16_op<AccumBits, true>();

        this->data[0] = op(this->data[0],
                           a, 0, 0x0000, 1,
                           b, 0, 0x3210, 4);
        this->data[0] = op(this->data[0],
                           a, 2, 0x0000, 1,
                           b, 8, 0x3210, 4);

        this->data[1] = op(this->data[1],
                           a, 4, 0x0000, 1,
                           b, 0, 0x3210, 4);
        this->data[1] = op(this->data[1],
                           a, 6, 0x0000, 1,
                           b, 8, 0x3210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_64_16_op<AccumBits, false>();
        constexpr auto mac_op = get_mmul_64_16_op<AccumBits, true>();

        this->data[0] = mul_op(a, 0, 0x0000, 1,
                               b, 0, 0x3210, 4);
        this->data[0] = mac_op(this->data[0],
                               a, 2, 0x0000, 1,
                               b, 8, 0x3210, 4);

        this->data[1] = mul_op(a, 4, 0x0000, 1,
                               b, 0, 0x3210, 4);
        this->data[1] = mac_op(this->data[1],
                               a, 6, 0x0000, 1,
                               b, 8, 0x3210, 4);
    }
};

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, cint32, int16, AccumBits> : public mmul_64_16<M, K, N, cint32, int16, AccumBits> { using mmul_64_16<M, K, N, cint32, int16, AccumBits>::mmul_64_16; };

}

#endif
