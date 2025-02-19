// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_C16_16__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_C16_16__HPP__

#include "../accum.hpp"
#include "../interleave.hpp"
#include "../vector.hpp"
#include "../filter.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeB, unsigned AccumBits>
struct mmul_c16_16;

template <typename TypeB, unsigned AccumBits, unsigned Lanes>
struct C_block_c16_16_interleave
{
    using               TypeA = cint16;

    using          accum_type = accum<accum_tag_t<AccumClass::CInt, AccumBits>, Lanes>;
    using internal_accum_type = accum<acc64, Lanes>;

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
        constexpr unsigned num_op = Lanes / 8;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            v4cacc64 tmp1 = acc.template extract<4>(2 * idx);
            v4cacc64 tmp2 = acc.template extract<4>(2 * idx + 1);
            real.template insert<8>(idx, (v8acc64)::shuffle((v8cint32)tmp1, (v8cint32)tmp2, DINTLV_lo_64o128));
            imag.template insert<8>(idx, (v8acc64)::shuffle((v8cint32)tmp1, (v8cint32)tmp2, DINTLV_hi_64o128));
        });
    }

    template <typename TR>
    __aie_inline
    C_block_c16_16_interleave(const vector<TR, Lanes> &v, int shift = 0) : C_block_c16_16_interleave(accum_type(v, shift))
    {
    }

    __aie_inline
    accum_type to_accum() const
    {
        accum_type ret;

        constexpr unsigned num_op = Lanes / 8;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            v8acc64 tmp_r = real.template extract<8>(idx);
            v8acc64 tmp_i = imag.template extract<8>(idx);
            ret.template insert<4>(2 * idx,     (v4cacc64)::shuffle((v8cint32)tmp_r, (v8cint32)tmp_i, INTLV_lo_64o128));
            ret.template insert<4>(2 * idx + 1, (v4cacc64)::shuffle((v8cint32)tmp_r, (v8cint32)tmp_i, INTLV_hi_64o128));
        });


        return ret;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename TR>
    __aie_inline
    vector<TR, Lanes> to_vector(int shift = 0) const
    {
        return to_accum().template to_vector<TR>(shift);
    }
};

template <typename TypeB>
struct mmul_c16_16<4, 4, 8, TypeB, 64> : public C_block_c16_16_interleave<TypeB, 64, 32>
{
    using         TypeA = cint16;

    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_c16_16_interleave<TypeB, 64, 32>::C_block_c16_16_interleave;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16,32>::run(reals, vector<int16, 32>(), 1);

        this->real = ::mac_4x4_4x8_conf(tmp1, true, b, b_sign, this->real, this->zero, 0, 0, 0);
        this->imag = ::mac_4x4_4x8_conf(tmp2, true, b, b_sign, this->imag, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto reals = a.template cast_to<int16>();
        const auto [tmp1, tmp2] = interleave_unzip<int16, 32>::run(reals, vector<int16, 32>(), 1);

        this->real = ::mul_4x4_4x8(tmp1, true, b, b_sign);
        this->imag = ::mul_4x4_4x8(tmp2, true, b, b_sign);
        this->zero = false;
    }
};

//TODO: Evaluate performance, and potentially use a different C_block_interleave with one 32 Lane acc64 internal for real+imag. Insert worse than concat in mac()
template <typename TypeB>
struct mmul_c16_16<2, 4, 8, TypeB, 64> : public C_block_c16_16_interleave<TypeB, 64, 16>
{
    using         TypeA = cint16;

    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_c16_16_interleave<TypeB, 64, 16>::C_block_c16_16_interleave;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto a_split = concat_vector(filter<int16, 16, FilterOp::Even>::run(a.template cast_to<int16>(), 1),
                                     filter<int16, 16, FilterOp::Odd> ::run(a.template cast_to<int16>(), 1));

        accum<acc64, 32> acc = ::mac_4x4_4x8_conf(a_split.template grow<32>(), true, b, b_sign, ::concat(this->real, this->imag), this->zero, 0, 0, 0);
        this->real = acc.template extract<16>(0);
        this->imag = acc.template extract<16>(1);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto a_split = concat_vector(filter<int16, 16, FilterOp::Even>::run(a.template cast_to<int16>(), 1),
                                     filter<int16, 16, FilterOp::Odd> ::run(a.template cast_to<int16>(), 1));

        accum<acc64, 32> acc = ::mul_4x4_4x8(a_split.template grow<32>(), true, b, b_sign);
        this->real = acc.template extract<16>(0);
        this->imag = acc.template extract<16>(1);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, uint16, 64> : public mmul_c16_16<M, K, N, uint16, 64> { using mmul_c16_16<M, K, N, uint16, 64>::mmul_c16_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cint16, int16, 64>  : public mmul_c16_16<M, K, N,  int16, 64> { using mmul_c16_16<M, K, N,  int16, 64>::mmul_c16_16; };

}

#endif
