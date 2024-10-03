// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_32_32__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_32_32__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mul_32_32()
{
    if constexpr ((is_complex_v<TypeA> || is_complex_v<TypeB>) && AccumBits <= 48) return [](auto &&...args) { return ::mul4(args...); };
    else                                                                           return [](auto &&...args) { return ::lmul4(args...); };
}

template <typename TypeA, typename TypeB, unsigned AccumBits>
constexpr auto get_op_mac_32_32()
{
    if constexpr ((is_complex_v<TypeA> || is_complex_v<TypeB>) && AccumBits <= 48) return [](auto &&...args) { return ::mac4(args...); };
    else                                                                           return [](auto &&...args) { return ::lmac4(args...); };
}

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32;

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<2, 2, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op(this->data,
                        a.template grow<16>(), 0, 0x2200, 1,
                        b.template grow<8>(),  0, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mul_32_32<TypeA, TypeB, AccumBits>();

        this->data = op(a.template grow<16>(), 0, 0x2200, 1,
                        b.template grow<8>(),  0, 0x1010, 2);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<2, 4, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op(this->data,
                        a.template grow<16>(), 0, 0x4400, 1,
                        b,                     0, 0x1010, 2);

        this->data = op(this->data,
                        a.template grow<16>(), 2, 0x4400, 1,
                        b,                     4, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_32<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a.template grow<16>(), 0, 0x4400, 1,
                            b,                     0, 0x1010, 2);

        this->data = op_mac(this->data,
                            a.template grow<16>(), 2, 0x4400, 1,
                            b,                     4, 0x1010, 2);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<2, 8, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a,    0, 0x8800, 1,
                            b.template extract<8>(0), 0, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,    2, 0x8800, 1,
                            b.template extract<8>(0), 4, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,    4, 0x8800, 1,
                            b.template extract<8>(1), 0, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,    6, 0x8800, 1,
                            b.template extract<8>(1), 4, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_32<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a,                        0, 0x8800, 1,
                            b.template extract<8>(0), 0, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,                        2, 0x8800, 1,
                            b.template extract<8>(0), 4, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,                        4, 0x8800, 1,
                            b.template extract<8>(1), 0, 0x1010, 2);

        this->data = op_mac(this->data,
                            a,                        6, 0x8800, 1,
                            b.template extract<8>(1), 4, 0x1010, 2);
    }

};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<4, 2, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 8, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op(this->data[0],
                           a.template grow<16>(), 0, 0x2200, 1,
                           b.template grow<8>(),  0, 0x1010, 2);

        this->data[1] = op(this->data[1],
                           a.template grow<16>(), 4, 0x2200, 1,
                           b.template grow<8>(),  0, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mul_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op(a.template grow<16>(), 0, 0x2200, 1,
                           b.template grow<8>(),  0, 0x1010, 2);

        this->data[1] = op(a.template grow<16>(), 4, 0x2200, 1,
                           b.template grow<8>(),  0, 0x1010, 2);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<4, 4, 2, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 8, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op(this->data[0],
                           a.template grow<16>(),  0, 0x4400, 1,
                           b.template grow<8>(),   0, 0x1010, 2);
        this->data[0] = op(this->data[0],
                           a.template grow<16>(),  2, 0x4400, 1,
                           b.template grow<8>(),   4, 0x1010, 2);

        this->data[1] = op(this->data[1],
                           a.template grow<16>(),  8, 0x4400, 1,
                           b.template grow<8>(),   0, 0x1010, 2);
        this->data[1] = op(this->data[1],
                           a.template grow<16>(), 10, 0x4400, 1,
                           b.template grow<8>(),   4, 0x1010, 2);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_32<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op_mul(a.template grow<16>(),  0, 0x4400, 1,
                               b.template grow<8>(),   0, 0x1010, 2);
        this->data[0] = op_mac(this->data[0],
                               a.template grow<16>(),  2, 0x4400, 1,
                               b.template grow<8>(),   4, 0x1010, 2);

        this->data[1] = op_mul(a.template grow<16>(),  8, 0x4400, 1,
                               b.template grow<8>(),   0, 0x1010, 2);
        this->data[1] = op_mac(this->data[1],
                               a.template grow<16>(), 10, 0x4400, 1,
                               b.template grow<8>(),   4, 0x1010, 2);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<4, 2, 4, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 16, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, AccumBits, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0].template insert<4>(0, op(this->data[0].template extract<4>(0),
                                               a.template grow<16>(), 0, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[0].template insert<4>(1, op(this->data[0].template extract<4>(1),
                                               a.template grow<16>(), 2, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[1].template insert<4>(0, op(this->data[1].template extract<4>(0),
                                               a.template grow<16>(), 4, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[1].template insert<4>(1, op(this->data[1].template extract<4>(1),
                                               a.template grow<16>(), 6, 0x0000, 1,
                                               b,                     0, 0x3210, 4));
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mul_32_32<TypeA, TypeB, AccumBits>();

        this->data[0].template insert<4>(0, op(a.template grow<16>(), 0, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[0].template insert<4>(1, op(a.template grow<16>(), 2, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[1].template insert<4>(0, op(a.template grow<16>(), 4, 0x0000, 1,
                                               b,                     0, 0x3210, 4));

        this->data[1].template insert<4>(1, op(a.template grow<16>(), 6, 0x0000, 1,
                                               b,                     0, 0x3210, 4));
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<2, 4, 4, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 8, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, AccumBits, 8, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op(this->data[0],
                           a.template grow<16>(),    0, 0x0000, 1,
                           b.template extract<8>(0), 0, 0x3210, 4);
        this->data[0] = op(this->data[0],
                           a.template grow<16>(),    2, 0x0000, 1,
                           b.template extract<8>(1), 0, 0x3210, 4);

        this->data[1] = op(this->data[1],
                           a.template grow<16>(),    4, 0x0000, 1,
                           b.template extract<8>(0), 0, 0x3210, 4);
        this->data[1] = op(this->data[1],
                           a.template grow<16>(),    6, 0x0000, 1,
                           b.template extract<8>(1), 0, 0x3210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_32<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data[0] = op_mul(a.template grow<16>(),    0, 0x0000, 1,
                               b.template extract<8>(0), 0, 0x3210, 4);
        this->data[0] = op_mac(this->data[0],
                               a.template grow<16>(),    2, 0x0000, 1,
                               b.template extract<8>(1), 0, 0x3210, 4);

        this->data[1] = op_mul(a.template grow<16>(),    4, 0x0000, 1,
                               b.template extract<8>(0), 0, 0x3210, 4);
        this->data[1] = op_mac(this->data[1],
                               a.template grow<16>(),    6, 0x0000, 1,
                               b.template extract<8>(1), 0, 0x3210, 4);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32<4, 4, 1, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, AccumBits, 4, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 4>;

    using C_block<TypeA, TypeB, AccumBits, 4, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op_mac(this->data,
                            a,                    0, 0xc840, 1,
                            b.template grow<8>(), 0, 0x0000, 1);

        this->data = op_mac(this->data,
                            a,                    2, 0xc840, 1,
                            b.template grow<8>(), 2, 0x0000, 1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        constexpr auto op_mul = get_op_mul_32_32<TypeA, TypeB, AccumBits>();
        constexpr auto op_mac = get_op_mac_32_32<TypeA, TypeB, AccumBits>();

        this->data = op_mul(a,                    0, 0xc840, 1,
                            b.template grow<8>(), 0, 0x0000, 1);

        this->data = op_mac(this->data,
                            a,                    2, 0xc840, 1,
                            b.template grow<8>(), 2, 0x0000, 1);
    }
};

template <>
struct mmul_32_32<2, 2, 2, float, float, 32> : public C_block_larger_internal<float, float, 32, 4, 2>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 4>;

    using C_block_larger_internal<float, float, 32, 4, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmac(this->data,
                             a.grow<16>(), 0, 0x2200,
                             b.grow<8>(),  0, 0x1010);

        this->data = ::fpmac(this->data,
                             a.grow<16>(), 1, 0x2200,
                             b.grow<8>(),  2, 0x1010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a.grow<16>(), 0, 0x2200,
                             b.grow<8>(),  0, 0x1010);

        this->data = ::fpmac(this->data,
                             a.grow<16>(), 1, 0x2200,
                             b.grow<8>(),  2, 0x1010);
    }
};

template <>
struct mmul_32_32<2, 4, 2, float, float, 32> : public C_block_larger_internal<float, float, 32, 4, 2>
{
    using vector_A_type = vector<float, 8>;
    using vector_B_type = vector<float, 8>;

    using C_block_larger_internal<float, float, 32, 4, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        decltype(this->data) tmp;

        tmp = ::fpmul(a.grow<16>(), 0, 0x55114400,
                      b,            0, 0x32321010);

        tmp = ::fpmac(tmp,
                      a.grow<16>(), 2, 0x55114400,
                      b,            4, 0x32321010);

        tmp = ::fpadd(tmp,
                      tmp.template to_vector<float>().template grow<16>(), 4, 0x3210);

        this->data = ::fpadd(tmp,
                             this->data.template to_vector<float>().template grow<16>(),   0, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a.grow<16>(), 0, 0x55114400,
                             b,            0, 0x32321010);

        this->data = ::fpmac(this->data,
                             a.grow<16>(), 2, 0x55114400,
                             b,            4, 0x32321010);

        this->data = ::fpadd(this->data,
                             this->data.template to_vector<float>().template grow<16>(), 4, 0x3210);
    }
};

template <>
struct mmul_32_32<2, 8, 2, float, float, 32> : public C_block_larger_internal<float, float, 32, 4, 2>
{
    using vector_A_type = vector<float, 16>;
    using vector_B_type = vector<float, 16>;

    using C_block_larger_internal<float, float, 32, 4, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        decltype(this->data) tmp;

        tmp = ::fpmul(a,                        0, 0x99118800,
                      b.template extract<8>(0), 0, 0x32321010);

        tmp = ::fpmac(tmp,
                      a,                        2, 0x99118800,
                      b.template extract<8>(0), 4, 0x32321010);

        tmp = ::fpmac(tmp,
                      a,                        4, 0x99118800,
                      b.template extract<8>(1), 0, 0x32321010);

        tmp = ::fpmac(tmp,
                      a,                        6, 0x99118800,
                      b.template extract<8>(1), 4, 0x32321010);

        tmp = ::fpadd(tmp,
                      tmp.template to_vector<float>().template grow<16>(), 4, 0x3210);

        this->data = ::fpadd(tmp,
                             this->data.template to_vector<float>().template grow<16>(),   0, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a,                        0, 0x99118800,
                             b.template extract<8>(0), 0, 0x32321010);

        this->data = ::fpmac(this->data,
                             a,                        2, 0x99118800,
                             b.template extract<8>(0), 4, 0x32321010);

        this->data = ::fpmac(this->data,
                             a,                        4, 0x99118800,
                             b.template extract<8>(1), 0, 0x32321010);

        this->data = ::fpmac(this->data,
                             a,                        6, 0x99118800,
                             b.template extract<8>(1), 4, 0x32321010);

        this->data = ::fpadd(this->data,
                             this->data.template to_vector<float>().template grow<16>(), 4, 0x3210);
    }
};

template <>
struct mmul_32_32<4, 2, 2, float, float, 32> : public C_block<float, float, 32, 8, 1>
{
    using vector_A_type = vector<float, 8>;
    using vector_B_type = vector<float, 4>;

    using C_block<float, float, 32, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmac(this->data,
                             a.grow<16>(), 0, 0x66442200,
                             b.grow<8>(),  0, 0x10101010);

        this->data = ::fpmac(this->data,
                             a.grow<16>(), 1, 0x66442200,
                             b.grow<8>(),  2, 0x10101010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a.grow<16>(), 0, 0x66442200,
                             b.grow<8>(),  0, 0x10101010);

        this->data = ::fpmac(this->data,
                             a.grow<16>(), 1, 0x66442200,
                             b.grow<8>(),  2, 0x10101010);
    }
};

template <>
struct mmul_32_32<4, 2, 4, float, float, 32> : public C_block<float, float, 32, 16, 2>
{
    using vector_A_type = vector<float, 8>;
    using vector_B_type = vector<float, 8>;

    using C_block<float, float, 32, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::fpmac(this->data[0],
                                a.template grow<16>(), 0, 0x22220000,
                                b,                     0, 0x32103210);

        this->data[0] = ::fpmac(this->data[0],
                                a.template grow<16>(), 1, 0x22220000,
                                b,                     4, 0x32103210);

        this->data[1] = ::fpmac(this->data[1],
                                a.template grow<16>(), 4, 0x22220000,
                                b,                     0, 0x32103210);

        this->data[1] = ::fpmac(this->data[1],
                                a.template grow<16>(), 5, 0x22220000,
                                b,                     4, 0x32103210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::fpmul(a.template grow<16>(), 0, 0x22220000,
                                b,                     0, 0x32103210);

        this->data[0] = ::fpmac(this->data[0],
                                a.template grow<16>(), 1, 0x22220000,
                                b,                     4, 0x32103210);

        this->data[1] = ::fpmul(a.template grow<16>(), 4, 0x22220000,
                                b,                     0, 0x32103210);

        this->data[1] = ::fpmac(this->data[1],
                                a.template grow<16>(), 5, 0x22220000,
                                b,                     4, 0x32103210);
    }
};

template <>
struct mmul_32_32<4, 4, 2, float, float, 32> : public C_block<float, float, 32, 8, 1>
{
    using vector_A_type = vector<float, 16>;
    using vector_B_type = vector<float, 8>;

    using C_block<float, float, 32, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmac(this->data,
                             a, 0, 0xcc884400,
                             b, 0, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 1, 0xcc884400,
                             b, 2, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 2, 0xcc884400,
                             b, 4, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 3, 0xcc884400,
                             b, 6, 0x10101010);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a, 0, 0xcc884400,
                             b, 0, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 1, 0xcc884400,
                             b, 2, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 2, 0xcc884400,
                             b, 4, 0x10101010);

        this->data = ::fpmac(this->data,
                             a, 3, 0xcc884400,
                             b, 6, 0x10101010);
    }
};

template <>
struct mmul_32_32<2, 4, 4, float, float, 32> : public C_block<float, float, 32, 8, 1>
{
    using vector_A_type = vector<float, 8>;
    using vector_B_type = vector<float, 16>;

    using C_block<float, float, 32, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmac(this->data,
                             b,  0, 0x32103210,
                             a,  0, 0x44440000);

        this->data = ::fpmac(this->data,
                             b,  4, 0x32103210,
                             a,  1, 0x44440000);

        this->data = ::fpmac(this->data,
                             b,  8, 0x32103210,
                             a,  2, 0x44440000);

        this->data = ::fpmac(this->data,
                             b, 12, 0x32103210,
                             a,  3, 0x44440000);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(b,  0, 0x32103210,
                             a,  0, 0x44440000);

        this->data = ::fpmac(this->data,
                             b,  4, 0x32103210,
                             a,  1, 0x44440000);

        this->data = ::fpmac(this->data,
                             b,  8, 0x32103210,
                             a,  2, 0x44440000);

        this->data = ::fpmac(this->data,
                             b, 12, 0x32103210,
                             a,  3, 0x44440000);
    }
};

template <>
struct mmul_32_32<4, 4, 1, float, float, 32> : public C_block_larger_internal<float, float, 32, 4, 2>
{
    using vector_A_type = vector<float, 16>;
    using vector_B_type = vector<float, 4>;

    using C_block_larger_internal<float, float, 32, 4, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        decltype(this->data) tmp;

        tmp = ::fpmul(a,           0, 0xd951c840,
                      b.grow<8>(), 0, 0x11110000);

        tmp = ::fpmac(tmp,
                      a,           2, 0xd951c840,
                      b.grow<8>(), 2, 0x11110000);

        tmp = ::fpadd(tmp,
                      tmp, 4, 0x3210);

        this->data = ::fpadd(this->data,
                             tmp, 0, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::fpmul(a,           0, 0xd951c840,
                             b.grow<8>(), 0, 0x11110000);

        this->data = ::fpmac(this->data,
                             a,           2, 0xd951c840,
                             b.grow<8>(), 2, 0x11110000);

        this->data = ::fpadd(this->data,
                             this->data, 4, 0x3210);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, int32, 80>          : public mmul_32_32<M, K, N, int32, int32, 80>          { using mmul_32_32<M, K, N, int32, int32, 80>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, cint16, cint16, AccumBits> : public mmul_32_32<M, K, N, cint16, cint16, AccumBits> { using mmul_32_32<M, K, N, cint16, cint16, AccumBits>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int32, cint16, AccumBits>  : public mmul_32_32<M, K, N, int32, cint16, AccumBits>  { using mmul_32_32<M, K, N, int32, cint16, AccumBits>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, cint16, int32, AccumBits>  : public mmul_32_32<M, K, N, cint16, int32, AccumBits>  { using mmul_32_32<M, K, N, cint16, int32, AccumBits>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, float, float, 32>          : public mmul_32_32<M, K, N, float, float, 32>          { using mmul_32_32<M, K, N, float, float, 32>::mmul_32_32; };

}

#endif
