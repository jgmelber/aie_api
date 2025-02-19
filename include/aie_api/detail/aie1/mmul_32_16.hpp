// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_32_16__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_32_16__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"

namespace aie::detail {

template <unsigned AccumBits, unsigned Lanes, bool IsAcc>
static constexpr auto get_mmul_32_16_op()
{
    if constexpr (IsAcc) {
        if constexpr (Lanes == 4) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac4(params...);  };
        }
        else if constexpr (Lanes == 8) {
            if constexpr (AccumBits <= 48) return [](auto ... params) { return ::mac8(params...);  };
            else                           return [](auto ... params) { return ::lmac8(params...); };
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
    }
}

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_16;

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<2, 2, 4, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 8, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<int16, 8>;

    using C_block<TypeA, int16, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data = mac_op(this->data,
                            a.template grow<16>(), 0, 0x22220000, 1,
                            b.template grow<16>(), 0, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();

        this->data = mul_op(a.template grow<16>(), 0, 0x22220000, 1,
                            b.template grow<16>(), 0, 0x32103210, 4);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<2, 4, 4, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 8, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<int16, 16>;

    using C_block<TypeA, int16, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data = mac_op(this->data,
                            a.template grow<16>(), 0, 0x44440000, 1,
                            b,                     0, 0x32103210, 4);

        this->data = mac_op(this->data,
                            a.template grow<16>(), 2, 0x44440000, 1,
                            b,                     8, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data = mul_op(a.template grow<16>(), 0, 0x44440000, 1,
                            b,                     0, 0x32103210, 4);

        this->data = mac_op(this->data,
                            a.template grow<16>(), 2, 0x44440000, 1,
                            b,                     8, 0x32103210, 4);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<4, 2, 4, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 16, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<int16, 8>;

    using C_block<TypeA, int16, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template grow<16>(), 0, 0x22220000, 1,
                               b.template grow<16>(), 0, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<16>(), 4, 0x22220000, 1,
                               b.template grow<16>(), 0, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();

        this->data[0] = mul_op(a.template grow<16>(), 0, 0x22220000, 1,
                               b.template grow<16>(), 0, 0x32103210, 4);

        this->data[1] = mul_op(a.template grow<16>(), 4, 0x22220000, 1,
                               b.template grow<16>(), 0, 0x32103210, 4);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<4, 4, 2, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 8, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<int16, 8>;

    using C_block<TypeA, int16, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data = mac_op(this->data,
                            a,                     0, 0xcc884400, 1,
                            b.template grow<16>(), 0, 0x10101010, 2);

        this->data = mac_op(this->data,
                            a,                     2, 0xcc884400, 1,
                            b.template grow<16>(), 4, 0x10101010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data = mul_op(a,                     0, 0xcc884400, 1,
                            b.template grow<16>(), 0, 0x10101010, 2);

        this->data = mac_op(this->data,
                            a,                     2, 0xcc884400, 1,
                            b.template grow<16>(), 4, 0x10101010, 2);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<4, 4, 4, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 16, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<int16, 16>;

    using C_block<TypeA, int16, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               a, 0, 0x44440000, 1,
                               b, 0, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a, 8, 0x44440000, 1,
                               b, 0, 0x32103210, 4);

        this->data[0] = mac_op(this->data[0],
                               a, 2, 0x44440000, 1,
                               b, 8, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a, 10, 0x44440000, 1,
                               b,  8, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mul_op(a, 0, 0x44440000, 1,
                               b, 0, 0x32103210, 4);

        this->data[1] = mul_op(a, 8, 0x44440000, 1,
                               b, 0, 0x32103210, 4);

        this->data[0] = mac_op(this->data[0],
                               a, 2, 0x44440000, 1,
                               b, 8, 0x32103210, 4);

        this->data[1] = mac_op(this->data[1],
                               a, 10, 0x44440000, 1,
                               b,  8, 0x32103210, 4);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<2, 2, 8, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 16, 2>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<int16, 16>;

    using C_block<TypeA, int16, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template grow<16>(), 0, 0x00000000, 1,
                               b,                     0, 0x76543210, 8);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<16>(), 2, 0x00000000, 1,
                               b,                     0, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();

        this->data[0] = mul_op(a.template grow<16>(), 0, 0x00000000, 1,
                               b,                     0, 0x76543210, 8);

        this->data[1] = mul_op(a.template grow<16>(), 2, 0x00000000, 1,
                               b,                     0, 0x76543210, 8);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_32_16<2, 4, 8, TypeA, int16, AccumBits> : public C_block<TypeA, int16, AccumBits, 16, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<int16, 32>;

    using C_block<TypeA, int16, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mac_op(this->data[0],
                               a.template grow<16>(),     0, 0x00000000, 1,
                               b.template extract<16>(0), 0, 0x76543210, 8);

        this->data[0] = mac_op(this->data[0],
                               a.template grow<16>(),     2, 0x00000000, 1,
                               b.template extract<16>(1), 0, 0x76543210, 8);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<16>(),     4, 0x00000000, 1,
                               b.template extract<16>(0), 0, 0x76543210, 8);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<16>(),     6, 0x00000000, 1,
                               b.template extract<16>(1), 0, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto mul_op = get_mmul_32_16_op<AccumBits, 8, false>();
        constexpr auto mac_op = get_mmul_32_16_op<AccumBits, 8, true>();

        this->data[0] = mul_op(a.template grow<16>(),     0, 0x00000000, 1,
                               b.template extract<16>(0), 0, 0x76543210, 8);

        this->data[0] = mac_op(this->data[0],
                               a.template grow<16>(),     2, 0x00000000, 1,
                               b.template extract<16>(1), 0, 0x76543210, 8);

        this->data[1] = mul_op(a.template grow<16>(),     4, 0x00000000, 1,
                               b.template extract<16>(0), 0, 0x76543210, 8);

        this->data[1] = mac_op(this->data[1],
                               a.template grow<16>(),     6, 0x00000000, 1,
                               b.template extract<16>(1), 0, 0x76543210, 8);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, int16, 48> : public mmul_32_16<M, K, N, cint16, int16, 48>             { using mmul_32_16<M, K, N, cint16, int16, 48>::mmul_32_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int32, int16, AccumBits> : public mmul_32_16<M, K, N, int32, int16, AccumBits> { using mmul_32_16<M, K, N,  int32, int16, AccumBits>::mmul_32_16; };

}

#endif
