// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_16_32__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_16_32__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"

namespace aie::detail {

template <unsigned AccumBits, unsigned Lanes, bool IsAcc>
static constexpr auto get_mmul_16_32_op()
{
    if constexpr (IsAcc) {
        if constexpr (Lanes == 4) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac4(params...);  };
        }
        else if constexpr (Lanes == 8) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac8(params...);  };
            else                           return [](auto ... params) { return ::lmac8(params...); };
        }
        else if constexpr (Lanes == 16) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac16(params...);  };
        }
    }
    else {
        if constexpr (Lanes == 4) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mul4(params...);  };
        }
        else if constexpr (Lanes == 8) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mul8(params...);  };
            else                           return [](auto ... params) { return ::lmul8(params...); };
        }
        else if constexpr (Lanes == 16) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mul16(params...);  };
        }
    }
}

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_32;

template <typename TypeB, unsigned AccumBits>
struct mmul_16_32<4, 4, 4, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 16, 2>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<int16, TypeB, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),    0, 0x44440000, 1,
                               b.template extract<8>(0), 0, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),    8, 0x44440000, 1,
                               b.template extract<8>(0), 0, 0x32103210, 4);

        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),    2, 0x44440000, 1,
                               b.template extract<8>(1), 0, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),    10, 0x44440000, 1,
                               b.template extract<8>(1),  0, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_32_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 8, true>();

        this->data[0] = mul_op(a.template grow<32>(),    0, 0x44440000, 1,
                               b.template extract<8>(0), 0, 0x32103210, 4);

        this->data[1] = mul_op(a.template grow<32>(),    8, 0x44440000, 1,
                               b.template extract<8>(0), 0, 0x32103210, 4);

        this->data[0] = mac_op(this->data[0],
                               a.template grow<32>(),    2, 0x44440000, 1,
                               b.template extract<8>(1), 0, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<32>(),    10, 0x44440000, 1,
                               b.template extract<8>(1),  0, 0x32103210, 4);
    }
};

template <unsigned AccumBits>
struct mmul_16_32<2, 4, 8, int16, int32, AccumBits> : public C_block<int16, int32, AccumBits, 16, 2>
{
    using vector_A_type = vector<int16, 8>;
    using vector_B_type = vector<int32, 32>;

    using C_block<int16, int32, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               b.template extract<16>(0), 0, 0x76543210, 8,
                               a.template grow<16>(),     0, 0x00000000, 1);

        this->data[0] = mac_op(this->data[0],
                               b.template extract<16>(1), 0, 0x76543210, 8,
                               a.template grow<16>(),     2, 0x00000000, 1);

        this->data[1] = mac_op(this->data[1],
                               b.template extract<16>(0), 0, 0x76543210, 8,
                               a.template grow<16>(),     4, 0x00000000, 1);

        this->data[1] = mac_op(this->data[1],
                               b.template extract<16>(1), 0, 0x76543210, 8,
                               a.template grow<16>(),     6, 0x00000000, 1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_32_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 8, true>();

        this->data[0] = mul_op(b.template extract<16>(0), 0, 0x76543210, 8,
                               a.template grow<16>(),     0, 0x00000000, 1);

        this->data[0] = mac_op(this->data[0],
                               b.template extract<16>(1), 0, 0x76543210, 8,
                               a.template grow<16>(),     2, 0x00000000, 1);

        this->data[1] = mul_op(b.template extract<16>(0), 0, 0x76543210, 8,
                               a.template grow<16>(),     4, 0x00000000, 1);

        this->data[1] = mac_op(this->data[1],
                               b.template extract<16>(1), 0, 0x76543210, 8,
                               a.template grow<16>(),     6, 0x00000000, 1);
    }
};

template <typename TypeB, unsigned AccumBits>
struct mmul_16_32<4, 2, 2, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 8, 1>
{
    using vector_A_type = vector<int16, 8>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<int16, TypeB, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 8, true>();

        this->data = mac_op(this->data,
                            a.template grow<32>(), 0, 0x66442200, 1,
                            b.template grow<8>(),  0, 0x10101010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_32_op<AccumBits, 8, false>();

        this->data = mul_op(a.template grow<32>(), 0, 0x66442200, 1,
                            b.template grow<8>(),  0, 0x10101010, 2);
    }
};

template <unsigned AccumBits>
struct mmul_16_32<4, 4, 1, int16, cint16, AccumBits> : public C_block<int16, cint16, AccumBits, 4, 1>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<cint16, 4>;

    using C_block<int16, cint16, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_16_32_op<AccumBits, 4, true>();

        this->data = mac_op(this->data,
                            a.template grow<32>(), 0, 0xc840, 1,
                            b.template grow<8>(),  0, 0x0000, 1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_16_32_op<AccumBits, 4, false>();

        this->data = mul_op(a.template grow<32>(), 0, 0xc840, 1,
                            b.template grow<8>(),  0, 0x0000, 1);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, cint16, 48>       : public mmul_16_32<M, K, N, int16, cint16, 48>       { using mmul_16_32<M, K, N, int16, cint16, 48>::mmul_16_32; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, int32, AccumBits> : public mmul_16_32<M, K, N, int16, int32, AccumBits> { using mmul_16_32<M, K, N, int16, int32, AccumBits>::mmul_16_32; };

}

#endif
