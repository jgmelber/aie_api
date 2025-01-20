// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_C16_C16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_C16_C16__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_c16_c16;

template <>
struct mmul_c16_c16<1, 4, 8, 64> : public C_block_larger_internal<cint16, cint16, 64, 8, 2>
{
    using TypeA = cint16;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_larger_internal<TypeA, TypeB, 64, 8, 2>::C_block_larger_internal;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // mac_elem_16_2_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint16 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_16_2_conf(::concat(broadcast<cint16, 16>::run(a[0]), broadcast<cint16, 16>::run(a[1])),
                                          ::concat(::shuffle(b.template extract<16>(0), vector<TypeB, 16>(), DINTLV_lo_256o512),
                                                   ::shuffle(b.template extract<16>(0), vector<TypeB, 16>(), DINTLV_hi_256o512)), this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_16_2     (::concat(broadcast<cint16, 16>::run(a[2]), broadcast<cint16, 16>::run(a[3])),
                                          ::concat(::shuffle(b.template extract<16>(1), vector<TypeB, 16>(), DINTLV_lo_256o512),
                                                   ::shuffle(b.template extract<16>(1), vector<TypeB, 16>(), DINTLV_hi_256o512)), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_elem_16_2(::concat(broadcast<cint16, 16>::run(a[0]), broadcast<cint16, 16>::run(a[1])),
                                     ::concat(::shuffle(b.template extract<16>(0), vector<TypeB, 16>(), DINTLV_lo_256o512),
                                              ::shuffle(b.template extract<16>(0), vector<TypeB, 16>(), DINTLV_hi_256o512)));
        this->data = ::mac_elem_16_2(::concat(broadcast<cint16, 16>::run(a[2]), broadcast<cint16, 16>::run(a[3])),
                                     ::concat(::shuffle(b.template extract<16>(1), vector<TypeB, 16>(), DINTLV_lo_256o512),
                                              ::shuffle(b.template extract<16>(1), vector<TypeB, 16>(), DINTLV_hi_256o512)), this->data);
        this->zero = false;
    }
};

template <>
struct mmul_c16_c16<2, 2, 16, 64> : public C_block<cint16, cint16, 64, 32, 2>
{
    using TypeA = cint16;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // mac_elem_16_2_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint16 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data[0] = ::mac_elem_16_2_conf(::concat(broadcast<cint16, 16>::run(a[0]), broadcast<cint16, 16>::run(a[1])), b, this->data[0], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data[1] = ::mac_elem_16_2_conf(::concat(broadcast<cint16, 16>::run(a[2]), broadcast<cint16, 16>::run(a[3])), b, this->data[1], this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_elem_16_2(::concat(broadcast<cint16, 16>::run(a[0]), broadcast<cint16, 16>::run(a[1])), b);
        this->data[1] = ::mul_elem_16_2(::concat(broadcast<cint16, 16>::run(a[2]), broadcast<cint16, 16>::run(a[3])), b);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, cint16, 64> : public mmul_c16_c16<M, K, N, 64> { using mmul_c16_c16<M, K, N, 64>::mmul_c16_c16; };

}

#endif
