// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_C16_16__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_C16_16__HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../accum.hpp"
#include "../interleave.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeB, unsigned AccumBits>
struct mmul_c16_16;

template <typename TypeB, unsigned AccumBits>
struct C_block_c16_16_interleave
{
    using               TypeA = cint16;

    using          accum_type = accum<accum_tag_t<AccumClass::CInt, AccumBits>, 16>;
    using internal_accum_type = accum<acc64, 16>;

    internal_accum_type real;
    internal_accum_type imag;
    bool       zero;

    __aie_inline
    C_block_c16_16_interleave() : zero(true)
    {}

    __aie_inline
    C_block_c16_16_interleave(const accum_type &acc, bool to_zero = false)
    {
        zero = to_zero;
        std::tie(real, imag) = unzip_complex(acc);
    }

    template <typename TR>
    __aie_inline
    C_block_c16_16_interleave(const vector<TR, 16> &v, int shift = 0) : C_block_c16_16_interleave(accum_type(v, shift))
    {
    }

    __aie_inline
    accum_type to_accum() const
    {
        accum_type ret = combine_into_complex(real, imag);
        return ret;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename TR>
    __aie_inline
    vector<TR, 16> to_vector(int shift = 0) const
    {
        return to_accum().template to_vector<TR>(shift);
    }
};

template <typename TypeB>
struct mmul_c16_16<2, 4, 8, TypeB, 64> : public C_block_c16_16_interleave<TypeB, 64>
{
    using         TypeA = cint16;

    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_c16_16_interleave<TypeB, 64>::C_block_c16_16_interleave;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16, 32>::run(reals.template grow<32>(), vector<int16, 32>(), 1);

        this->real = ::mac_2x4_4x8_conf(tmp1, true, b, b_sign, this->real, this->zero, 0, 0, 0);
        this->imag = ::mac_2x4_4x8_conf(tmp2, true, b, b_sign, this->imag, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16, 32>::run(reals.template grow<32>(), vector<int16, 32>(), 1);

        this->real = ::mul_2x4_4x8(tmp1, true, b, b_sign);
        this->imag = ::mul_2x4_4x8(tmp2, true, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeB>
struct mmul_c16_16<4, 4, 4, TypeB, 64> : public C_block_c16_16_interleave<TypeB, 64>
{
    using         TypeA = cint16;

    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block_c16_16_interleave<TypeB, 64>::C_block_c16_16_interleave;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16, 32>::run(reals, vector<int16, 32>(), 1);

        this->real = ::mac_4x4_4x4_conf(tmp1, true, b.template grow<32>(), b_sign, this->real, this->zero, 0, 0, 0);
        this->imag = ::mac_4x4_4x4_conf(tmp2, true, b.template grow<32>(), b_sign, this->imag, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16, 32>::run(reals, vector<int16, 32>(), 1);

        this->real = ::mul_4x4_4x4(tmp1, true, b.template grow<32>(), b_sign);
        this->imag = ::mul_4x4_4x4(tmp2, true, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};


template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, uint16, 64> : public mmul_c16_16<M, K, N, uint16, 64> { using mmul_c16_16<M, K, N, uint16, 64>::mmul_c16_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, int16, 64>  : public mmul_c16_16<M, K, N,  int16, 64> { using mmul_c16_16<M, K, N,  int16, 64>::mmul_c16_16; };

}

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif // __AIE_API_DETAIL_AIE2_MMUL_C16_16__HPP__
