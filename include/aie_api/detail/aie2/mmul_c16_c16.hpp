// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_C16_C16__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_C16_C16__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_c16_c16;

template <>
struct mmul_c16_c16<1, 4, 8, 64> : public C_block<cint16, cint16, 64, 8, 1>
{
    using TypeA = cint16;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cint32, 2> a_up = vector_cast<cint32>(a);

        vector<TypeA, 16> tmp1 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[0]));
        vector<TypeA, 16> tmp2 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[1]));

        // mac_elem_8_2_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint16 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_8_2_conf(::shuffle(tmp1, tmp1, T32_8x4_lo), b.template extract<16>(0), this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_8_2     (::shuffle(tmp2, tmp2, T32_8x4_lo), b.template extract<16>(1), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cint32, 2> a_up = vector_cast<cint32>(a);

        vector<TypeA, 16> tmp1 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[0]));
        vector<TypeA, 16> tmp2 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[1]));

        this->data = ::mul_elem_8_2(::shuffle(tmp1, tmp1, T32_8x4_lo), b.template extract<16>(0));
        this->data = ::mac_elem_8_2(::shuffle(tmp2, tmp2, T32_8x4_lo), b.template extract<16>(1), this->data);
        this->zero = false;
    }
};

template <>
struct mmul_c16_c16<2, 4, 8, 64> : public C_block<cint16, cint16, 64, 16, 2>
{
    using TypeA = cint16;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cint32, 4> a_up = vector_cast<cint32>(a);

        vector<TypeA, 16> tmp1 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[0]));
        vector<TypeA, 16> tmp2 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[1]));

        // mac_elem_8_2_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint16 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[0] = ::mac_elem_8_2_conf(::shuffle(tmp1, tmp1, T32_8x4_lo), b.template extract<16>(0), this->data[0], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[0] = ::mac_elem_8_2     (::shuffle(tmp2, tmp2, T32_8x4_lo), b.template extract<16>(1), this->data[0]);

        vector<TypeA, 16> tmp3 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[2]));
        vector<TypeA, 16> tmp4 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[3]));

        // mac_elem_8_2_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint16 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[1] = ::mac_elem_8_2_conf(::shuffle(tmp3, tmp3, T32_8x4_lo), b.template extract<16>(0), this->data[1], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[1] = ::mac_elem_8_2     (::shuffle(tmp4, tmp4, T32_8x4_lo), b.template extract<16>(1), this->data[1]);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<cint32, 4> a_up = vector_cast<cint32>(a);

        vector<TypeA, 16> tmp1 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[0]));
        vector<TypeA, 16> tmp2 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[1]));

        this->data[0] = ::mul_elem_8_2(::shuffle(tmp1, tmp1, T32_8x4_lo), b.template extract<16>(0));
        this->data[0] = ::mac_elem_8_2(::shuffle(tmp2, tmp2, T32_8x4_lo), b.template extract<16>(1), this->data[0]);

        vector<TypeA, 16> tmp3 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[2]));
        vector<TypeA, 16> tmp4 = vector_cast<cint16>(broadcast<cint32, 8>::run(a_up[3]));

        this->data[1] = ::mul_elem_8_2(::shuffle(tmp3, tmp3, T32_8x4_lo), b.template extract<16>(0));
        this->data[1] = ::mac_elem_8_2(::shuffle(tmp4, tmp4, T32_8x4_lo), b.template extract<16>(1), this->data[1]);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, cint16, 64> : public mmul_c16_c16<M, K, N, 64> { using mmul_c16_c16<M, K, N, 64>::mmul_c16_c16; };

}

#endif
