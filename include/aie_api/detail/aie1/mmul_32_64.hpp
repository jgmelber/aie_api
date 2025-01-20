// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_32_64__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_32_64__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mul_32_64_4lanes()
{
    if      constexpr (is_floating_point_v<TypeA>)             return [](auto &&...args) { return ::fpmul(args...); };
    else if constexpr (is_complex_v<TypeB> && AccumBits <= 48) return [](auto &&...args) { return ::mul4(args...);  };
    else                                                       return [](auto &&...args) { return ::lmul4(args...); };
}

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mac_32_64_4lanes()
{
    if     constexpr  (is_floating_point_v<TypeA>)             return [](auto &&...args) { return ::fpmac(args...); };
    else if constexpr (is_complex_v<TypeB> && AccumBits <= 48) return [](auto &&...args) { return ::mac4(args...);  };
    else                                                       return [](auto &&...args) { return ::lmac4(args...); };
}

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_64;

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_64<2, 2, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a.template grow<32>(), 0, 0x2200,
                            b,                     0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(), 1, 0x2200,
                            b,                     2, 0x1010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_64_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a.template grow<32>(), 0, 0x2200,
                            b,                     0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(), 1, 0x2200,
                            b,                     2, 0x1010);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_64<2, 4, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a.template grow<32>(),    0, 0x4400,
                            b.template extract<4>(0), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    1, 0x4400,
                            b.template extract<4>(0), 2, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    2, 0x4400,
                            b.template extract<4>(1), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    3, 0x4400,
                            b.template extract<4>(1), 2, 0x1010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_64_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a.template grow<32>(),    0, 0x4400,
                            b.template extract<4>(0), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    1, 0x4400,
                            b.template extract<4>(0), 2, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    2, 0x4400,
                            b.template extract<4>(1), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<32>(),    3, 0x4400,
                            b.template extract<4>(1), 2, 0x1010);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_64<4, 2, 1, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 2>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a.template grow<32>(), 0, 0x6420,
                            b.template grow<4>(),  0, 0x0000);

        this->data = op_mac(this->data,
                            a.template grow<32>(), 1, 0x6420,
                            b.template grow<4>(),  1, 0x0000);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_64_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_64_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a.template grow<32>(), 0, 0x6420,
                            b.template grow<4>(),  0, 0x0000);

        this->data = op_mac(this->data,
                            a.template grow<32>(), 1, 0x6420,
                            b.template grow<4>(),  1, 0x0000);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, cint32, 80>         : public mmul_32_64<M, K, N, int32, cint32, 80>         { using mmul_32_64<M, K, N, int32, cint32, 80>::mmul_32_64; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, cint16, cint32, AccumBits> : public mmul_32_64<M, K, N, cint16, cint32, AccumBits> { using mmul_32_64<M, K, N, cint16, cint32, AccumBits>::mmul_32_64; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, float, cfloat, 32>         : public mmul_32_64<M, K, N, float, cfloat, 32>         { using mmul_32_64<M, K, N, float, cfloat, 32>::mmul_32_64; };

}

#endif
