// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_C32_C16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_C32_C16__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_c32_c16;

template <>
struct mmul_c32_c16<1, 2, 4, 64> :
#if __AIE_API_8_LANE_MUL_ELEM_8__
                                   public C_block_larger_internal<cint32, cint16, 64, 4, 2>
#else
                                   public C_block_larger_internal<cint32, cint16, 64, 4, 4>
#endif
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 8>;

#if __AIE_API_8_LANE_MUL_ELEM_8__
    using C_block_larger_internal<TypeA, TypeB, 64, 4, 2>::C_block_larger_internal;
#else
    using C_block_larger_internal<TypeA, TypeB, 64, 4, 4>::C_block_larger_internal;
#endif

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
struct mmul_c32_c16<1, 2, 8, 64> :
#if __AIE_API_8_LANE_MUL_ELEM_8__
                                   public C_block<cint32, cint16, 64, 8, 1>
#else
                                   public C_block_larger_internal<cint32, cint16, 64, 8, 2>
#endif
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 16>;

#if __AIE_API_8_LANE_MUL_ELEM_8__
    using C_block<TypeA, TypeB, 64, 8, 1>::C_block;
#else
    using C_block_larger_internal<TypeA, TypeB, 64, 8, 2>::C_block_larger_internal;
#endif

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
struct mmul_c32_c16<1, 2, 16, 64> : public C_block<cint32, cint16, 64, 16, 1>
{
    using TypeA = cint32;
    using TypeB = cint16;

    using vector_A_type = vector<TypeA, 2>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 16> tmp1 = broadcast<TypeA, 16>::run(a[0]);
        const vector<TypeA, 16> tmp2 = broadcast<TypeA, 16>::run(a[1]);

        // mac_elem_8_conf flags: int zero_acc1, int shift16, int sub_mask, int sub_mul, int sub_acc1
        // cint32 x cint16 operands require sub_mask = OP_TERM_NEG_COMPLEX
        this->data = ::mac_elem_16_conf(tmp1, b.template extract<16>(0), this->data, this->zero, 0, OP_TERM_NEG_COMPLEX, 0, 0);
        this->data = ::mac_elem_16     (tmp2, b.template extract<16>(1), this->data);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const vector<TypeA, 16> tmp1 = broadcast<TypeA, 16>::run(a[0]);
        const vector<TypeA, 16> tmp2 = broadcast<TypeA, 16>::run(a[1]);
        this->data = ::mul_elem_16(tmp1, b.template extract<16>(0));
        this->data = ::mac_elem_16(tmp2, b.template extract<16>(1), this->data);
        this->zero = false;
    }
};
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint32, cint16, 64> : public mmul_c32_c16<M, K, N, 64> { using mmul_c32_c16<M, K, N, 64>::mmul_c32_c16; };

}

#endif
