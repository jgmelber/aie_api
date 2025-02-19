// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_C32_C32__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_C32_C32__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_c32_c32;

template <>
struct mmul_c32_c32<1, 2, 8, 64> : public C_block<cint32, cint32, 64, 8, 1>
{
    using TypeA = cint32;
    using TypeB = cint32;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
#if __AIE_API_MUL_CONJUGATE_32BIT_INTRINSICS__
        this->data = ::mac_elem_8_conf(tmp1, b.template extract<8>(0), this->data, this->zero, OP_TERM_NEG_COMPLEX, 0, 0);
#else
        this->data = ::mac_elem_8_conf(tmp1, b.template extract<8>(0), this->data, this->zero, 0, 0);
#endif
        this->data = ::mac_elem_8     (tmp2, b.template extract<8>(1), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 8> tmp1 = broadcast<TypeA, 8>::run(a[0]);
        const vector<TypeA, 8> tmp2 = broadcast<TypeA, 8>::run(a[1]);
        this->data = ::mul_elem_8(tmp1, b.template extract<8>(0));
        this->data = ::mac_elem_8(tmp2, b.template extract<8>(1), this->data);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint32, cint32, 64> : public mmul_c32_c32<M, K, N, 64> { using mmul_c32_c32<M, K, N, 64>::mmul_c32_c32; };

}

#endif
