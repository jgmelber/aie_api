// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_64_32__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_64_32__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mul_64_32_4lanes()
{
    if      constexpr (is_floating_point_v<TypeA>)             return [](auto &&...args) { return ::fpmul(args...); };
    else if constexpr (is_complex_v<TypeB> && AccumBits <= 48) return [](auto &&...args) { return ::mul4(args...);  };
    else                                                       return [](auto &&...args) { return ::lmul4(args...); };
}

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mac_64_32_4lanes()
{
    if     constexpr  (is_floating_point_v<TypeA>)             return [](auto &&...args) { return ::fpmac(args...); };
    else if constexpr (is_complex_v<TypeB> && AccumBits <= 48) return [](auto &&...args) { return ::mac4(args...);  };
    else                                                       return [](auto &&...args) { return ::lmac4(args...); };
}

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_64_32;

template <unsigned AccumBits>
struct mmul_64_32<1, 2, 2, cint32, int32, AccumBits> : public C_block<cint32, int32, AccumBits, 2, 1>
{
    using vector_A_type = vector<cint32, 2>;
    using vector_B_type = vector<int32,  4>;

    using C_block<cint32, int32, AccumBits, 2, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::lmac2(this->data,
                             a.template grow<8>(), 0, 0x00, 1,
                             b.template grow<8>(), 0, 0x10, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::lmul2(a.template grow<8>(), 0, 0x00, 1,
                             b.template grow<8>(), 0, 0x10, 2);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_64_32<2, 2, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a.template grow<8>(), 0, 0x2200,
                            b.template grow<8>(), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<8>(), 1, 0x2200,
                            b.template grow<8>(), 2, 0x1010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_64_32_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a.template grow<8>(), 0, 0x2200,
                            b.template grow<8>(), 0, 0x1010);

        this->data = op_mac(this->data,
                            a.template grow<8>(), 1, 0x2200,
                            b.template grow<8>(), 2, 0x1010);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_64_32<2, 4, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a, 0, 0x4400,
                            b, 0, 0x1010);

        this->data = op_mac(this->data,
                            a, 1, 0x4400,
                            b, 2, 0x1010);

        this->data = op_mac(this->data,
                            a, 2, 0x4400,
                            b, 4, 0x1010);

        this->data = op_mac(this->data,
                            a, 3, 0x4400,
                            b, 6, 0x1010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_64_32_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a, 0, 0x4400,
                            b, 0, 0x1010);

        this->data = op_mac(this->data,
                            a, 1, 0x4400,
                            b, 2, 0x1010);

        this->data = op_mac(this->data,
                            a, 2, 0x4400,
                            b, 4, 0x1010);

        this->data = op_mac(this->data,
                            a, 3, 0x4400,
                            b, 6, 0x1010);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_64_32<4, 4, 1, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a,                    0, 0xc840,
                            b.template grow<8>(), 0, 0x0000);

        this->data = op_mac(this->data,
                            a,                    1, 0xc840,
                            b.template grow<8>(), 1, 0x0000);

        this->data = op_mac(this->data,
                            a,                    2, 0xc840,
                            b.template grow<8>(), 2, 0x0000);

        this->data = op_mac(this->data,
                            a,                    3, 0xc840,
                            b.template grow<8>(), 3, 0x0000);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_64_32_4lanes<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_64_32_4lanes<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a,                    0, 0xc840,
                            b.template grow<8>(), 0, 0x0000);

        this->data = op_mac(this->data,
                            a,                    1, 0xc840,
                            b.template grow<8>(), 1, 0x0000);

        this->data = op_mac(this->data,
                            a,                    2, 0xc840,
                            b.template grow<8>(), 2, 0x0000);

        this->data = op_mac(this->data,
                            a,                    3, 0xc840,
                            b.template grow<8>(), 3, 0x0000);
    }
};

template <>
struct mmul_64_32<2, 4, 1, cfloat, float, 32> : public C_block_larger_internal<cfloat, float, 32, 2, 2>
{
    using vector_A_type = vector<cfloat, 8>;
    using vector_B_type = vector<float, 4>;

    using C_block_larger_internal<cfloat, float, 32, 2, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cfloat, 4> tmp = ::fpmul(a.template grow<16>(), 0, 0x6240,
                                        b.template grow<8>(),  0, 0x2200);
                          tmp = ::fpmac(tmp,
                                        a.template grow<16>(), 1, 0x6240,
                                        b.template grow<8>(),  1, 0x2200);
                          tmp = ::fpadd(tmp, tmp, 2, 0x0010);
        this->data = ::fpadd(this->data, tmp, 0, 0x0010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cfloat, 4> tmp = ::fpmul(a.template grow<16>(), 0, 0x6240,
                                        b.template grow<8>(),  0, 0x2200);
                          tmp = ::fpmac(tmp,
                                        a.template grow<16>(), 1, 0x6240,
                                        b.template grow<8>(),  1, 0x2200);
        this->data = ::fpadd(tmp, tmp, 2, 0x0010);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint32, int32, 80>         : public mmul_64_32<M, K, N, cint32, int32, 80>         { using mmul_64_32<M, K, N, cint32, int32, 80>::mmul_64_32; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, cint32, cint16, AccumBits> : public mmul_64_32<M, K, N, cint32, cint16, AccumBits> { using mmul_64_32<M, K, N, cint32, cint16, AccumBits>::mmul_64_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cfloat, float, 32>         : public mmul_64_32<M, K, N, cfloat, float, 32>         { using mmul_64_32<M, K, N, cfloat, float, 32>::mmul_64_32; };

}

#endif
