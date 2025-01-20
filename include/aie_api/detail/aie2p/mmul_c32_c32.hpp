// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_C32_C32__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_C32_C32__HPP__

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
        vector<TypeA, 16> tmp;
        tmp.template insert(0, broadcast<TypeA, 8>::run(a[0]));
        tmp.template insert(1, broadcast<TypeA, 8>::run(a[1]));

        accum<cacc64, 16> acc;

#if __AIE_API_HAS_32B_MUL__
        acc = ::mul_elem_16(tmp, b.extract<8>(0), b.extract<8>(1));
#else
        acc = ::mul_elem_16_t<MulMacroOp::Mul>(tmp, b.extract<8>(0), b.extract<8>(1));
#endif
        acc = ::add(acc, acc.template extract<8>(1).template grow<16>());
        acc = ::add_conf(this->data.template grow<16>(), acc, this->zero, 0, 0, 0);

        this->data = acc.template extract<8>(0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<TypeA, 16> tmp;
        tmp.template insert(0, broadcast<TypeA, 8>::run(a[0]));
        tmp.template insert(1, broadcast<TypeA, 8>::run(a[1]));

        accum<cacc64, 16> acc;

        // TODO: do something more efficient to add the current accumulator
#if __AIE_API_HAS_32B_MUL__
        acc = ::mul_elem_16(tmp, b.extract<8>(0), b.extract<8>(1));
#else
        acc = ::mul_elem_16_t<MulMacroOp::Mul>(tmp, b.extract<8>(0), b.extract<8>(1));
#endif
        acc = ::add(acc, acc.template extract<8>(1).template grow<16>());

        this->data = acc.template extract<8>(0);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint32, cint32, 64> : public mmul_c32_c32<M, K, N, 64> { using mmul_c32_c32<M, K, N, 64>::mmul_c32_c32; };

}

#endif
