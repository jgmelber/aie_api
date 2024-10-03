// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_C32_C16__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_C32_C16__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_c32_c16;

template <>
struct mmul_c32_c16<1, 2, 4, 64> : public C_block_larger_internal<cint32, cint16, 64, 4, 2>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block_larger_internal<TypeA, TypeB, 64, 4, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_8_conf(tmp1, b.template grow<16>(),                        this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_8     (tmp2, b.template extract<4>(1).template grow<16>(), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        this->data = ::mul_elem_8(tmp1, b.template grow<16>());
        this->data = ::mac_elem_8(tmp2, b.template extract<4>(1).template grow<16>(), this->data);
        this->zero = false;
    }
};

template <>
struct mmul_c32_c16<1, 2, 8, 64> : public C_block<cint32, cint16, 64, 8, 1>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_8_conf(tmp1, b,                                            this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_8     (tmp2, b.template extract<8>(1).template grow<16>(), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        this->data = ::mul_elem_8(tmp1, b);
        this->data = ::mac_elem_8(tmp2, b.template extract<8>(1).template grow<16>(), this->data);
        this->zero = false;
    }
};

template <>
struct mmul_c32_c16<2, 2, 8, 64> : public C_block<cint32, cint16, 64, 16, 2>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[0] = ::mac_elem_8_conf(tmp1, b,                                            this->data[0], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[0] = ::mac_elem_8     (tmp2, b.template extract<8>(1).template grow<16>(), this->data[0]);

        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[1] = ::mac_elem_8_conf(tmp3, b,                                            this->data[1], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[1] = ::mac_elem_8     (tmp4, b.template extract<8>(1).template grow<16>(), this->data[1]);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        this->data[0] = ::mul_elem_8(tmp1, b);
        this->data[0] = ::mac_elem_8(tmp2, b.template extract<8>(1).template grow<16>(), this->data[0]);

        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);
        this->data[1] = ::mul_elem_8(tmp3, b);
        this->data[1] = ::mac_elem_8(tmp4, b.template extract<8>(1).template grow<16>(), this->data[1]);
        this->zero = false;
    }
};

template <>
struct mmul_c32_c16<1, 4, 8, 64> : public C_block<cint32, cint16, 64, 8, 1>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_8_conf(tmp1, b.template extract<16>(0),                    this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_8     (tmp2, b.template extract<8>(1).template grow<16>(), this->data);
        this->data = ::mac_elem_8     (tmp3, b.template extract<16>(1),                    this->data);
        this->data = ::mac_elem_8     (tmp4, b.template extract<8>(3).template grow<16>(), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);
        this->data = ::mul_elem_8(tmp1, b.template extract<16>(0));
        this->data = ::mac_elem_8(tmp2, b.template extract<8>(1).template grow<16>(), this->data);
        this->data = ::mac_elem_8(tmp3, b.template extract<16>(1),                    this->data);
        this->data = ::mac_elem_8(tmp4, b.template extract<8>(3).template grow<16>(), this->data);
        this->zero = false;
    }
};

template <>
struct mmul_c32_c16<2, 4, 8, 64> : public C_block<cint32, cint16, 64, 16, 2>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[0] = ::mac_elem_8_conf(tmp1, b.template extract<16>(0),                    this->data[0], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[0] = ::mac_elem_8     (tmp2, b.template extract<8>(1).template grow<16>(), this->data[0]);
        this->data[0] = ::mac_elem_8     (tmp3, b.template extract<16>(1),                    this->data[0]);
        this->data[0] = ::mac_elem_8     (tmp4, b.template extract<8>(3).template grow<16>(), this->data[0]);

        const vector<TypeA, 8> tmp5 = broadcast<TypeA, 8>::run(a[4]);
        const vector<TypeA, 8> tmp6 = broadcast<TypeA, 8>::run(a[5]);
        const vector<TypeA, 8> tmp7 = broadcast<TypeA, 8>::run(a[6]);
        const vector<TypeA, 8> tmp8 = broadcast<TypeA, 8>::run(a[7]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[1] = ::mac_elem_8_conf(tmp5, b.template extract<16>(0),                    this->data[1], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[1] = ::mac_elem_8     (tmp6, b.template extract<8>(1).template grow<16>(), this->data[1]);
        this->data[1] = ::mac_elem_8     (tmp7, b.template extract<16>(1),                    this->data[1]);
        this->data[1] = ::mac_elem_8     (tmp8, b.template extract<8>(3).template grow<16>(), this->data[1]);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        const vector<TypeA, 8> tmp3 = broadcast<TypeA, 8>::run(a[2]);
        const vector<TypeA, 8> tmp4 = broadcast<TypeA, 8>::run(a[3]);
        this->data[0] = ::mul_elem_8(tmp1, b.template extract<16>(0));
        this->data[0] = ::mac_elem_8(tmp2, b.template extract<8>(1).template grow<16>(), this->data[0]);
        this->data[0] = ::mac_elem_8(tmp3, b.template extract<16>(1),                    this->data[0]);
        this->data[0] = ::mac_elem_8(tmp4, b.template extract<8>(3).template grow<16>(), this->data[0]);

        const vector<TypeA, 8> tmp5 = broadcast<TypeA, 8>::run(a[4]);
        const vector<TypeA, 8> tmp6 = broadcast<TypeA, 8>::run(a[5]);
        const vector<TypeA, 8> tmp7 = broadcast<TypeA, 8>::run(a[6]);
        const vector<TypeA, 8> tmp8 = broadcast<TypeA, 8>::run(a[7]);
        this->data[1] = ::mul_elem_8(tmp5, b.template extract<16>(0));
        this->data[1] = ::mac_elem_8(tmp6, b.template extract<8>(1).template grow<16>(), this->data[1]);
        this->data[1] = ::mac_elem_8(tmp7, b.template extract<16>(1),                    this->data[1]);
        this->data[1] = ::mac_elem_8(tmp8, b.template extract<8>(3).template grow<16>(), this->data[1]);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint32, cint16, 64> : public mmul_c32_c16<M, K, N, 64> { using mmul_c32_c16<M, K, N, 64>::mmul_c32_c16; };

}

#endif
