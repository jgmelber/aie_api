// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MMUL_16_16__HPP__
#define __AIE_API_DETAIL_AIE2_MMUL_16_16__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../filter.hpp"
#include "../utils.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16;

template <typename TypeA, typename TypeB>
struct mmul_16_16<4, 2, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x2_2x8_conf(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x2_2x8(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_16<8, 2, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 32, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x2_2x8_conf(a.template grow<32>(),                         a_sign, b.template grow<32>(), b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_4x2_2x8_conf(::shuffle(a.template grow<32>(), T128_4x2_hi), a_sign, b.template grow<32>(), b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x2_2x8(a.template grow<32>(),                         a_sign, b.template grow<32>(), b_sign);
        this->data[1] = ::mul_4x2_2x8(::shuffle(a.template grow<32>(), T128_4x2_hi), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<2, 4, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_2x4_4x8_conf(a.template grow<32>(), a_sign, b, b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_2x4_4x8(a.template grow<32>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<4, 4, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_2x4_4x8_conf(a.template grow<32>(),                                  a_sign, b, b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_2x4_4x8_conf(shuffle_down<TypeA, 16>::run(a, 8).template grow<32>(), a_sign, b, b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_2x4_4x8(a.template grow<32>(),                                  a_sign, b, b_sign);
        this->data[1] = ::mul_2x4_4x8(shuffle_down<TypeA, 16>::run(a, 8).template grow<32>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<4, 4, 4, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x4_4x4_conf(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x4_4x4(a.template grow<32>(), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<2, 8, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 16, 1>
{
    using vector_A_type        = vector<TypeA, 16>;
    using vector_B_type = sparse_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 64, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_2x8_8x8_conf(a.template grow<32>(), a_sign, ::shuffle(b, T16_8x4), b_sign, this->data, this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_2x8_8x8(a.template grow<32>(), a_sign, ::shuffle(b, T16_8x4), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<4, 8, 8, TypeA, TypeB, AccumBits> : public C_block<TypeA, TypeB, 64, 32, 2>
{
    using vector_A_type        = vector<TypeA, 32>;
    using vector_B_type = sparse_vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 64, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_2x8_8x8_conf(a,                                             a_sign, ::shuffle(b, T16_8x4), b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_2x8_8x8_conf(a.template extract<16>(1).template grow<32>(), a_sign, ::shuffle(b, T16_8x4), b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_2x8_8x8(a,                                             a_sign, ::shuffle(b, T16_8x4), b_sign);
        this->data[1] = ::mul_2x8_8x8(a.template extract<16>(1).template grow<32>(), a_sign, ::shuffle(b, T16_8x4), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_16<2, 8, 16, TypeA, TypeB, AccumBits>
{
    using vector_A_type        = vector<TypeA, 16>;
    using vector_B_type = sparse_vector<TypeB, 128>;

    using accum_type          = accum<accum_tag_t<AccumClass::Int, AccumBits>, 32>;
    using internal_accum_type = accum<acc64, 16>;

    internal_accum_type data[2];
    bool       zero;

    __aie_inline
    mmul_16_16() : zero(true)
    {}

    __aie_inline
    mmul_16_16(const accum_type &acc, bool to_zero = false)
    {
        zero = to_zero;
        if constexpr (AccumBits > 32) {
            const auto vec1 = accum_to_vector_cast<cint32, acc64, 8>::run(acc.template extract<8>(0));
            const auto vec2 = accum_to_vector_cast<cint32, acc64, 8>::run(acc.template extract<8>(1));
            const auto vec3 = accum_to_vector_cast<cint32, acc64, 8>::run(acc.template extract<8>(2));
            const auto vec4 = accum_to_vector_cast<cint32, acc64, 8>::run(acc.template extract<8>(3));

            const auto [tmp1, tmp2] = interleave_unzip<cint32, 8>::run(vec1, vec2, 8);

            const auto [tmp3, tmp4] = interleave_unzip<cint32, 8>::run(vec3, vec4, 8);

            data[0] = ::concat(vector_to_accum_cast<acc64, cint32, 8>::run(tmp1),
                               vector_to_accum_cast<acc64, cint32, 8>::run(tmp3));
            data[1] = ::concat(vector_to_accum_cast<acc64, cint32, 8>::run(tmp2),
                               vector_to_accum_cast<acc64, cint32, 8>::run(tmp4));
        }
        else {
            const auto vec1 = accum_to_vector_cast< int32, acc32, 16>::run(acc.template extract<16>(0));
            const auto vec2 = accum_to_vector_cast< int32, acc32, 16>::run(acc.template extract<16>(1));

            const auto [tmp1, tmp2] = interleave_unzip<int32, 16>::run(vec1, vec2, 8);

            data[0].from_vector(tmp1);
            data[1].from_vector(tmp2);
        }
    }

    template <typename T>
    __aie_inline
    mmul_16_16(const vector<T, 32> &v, int shift = 0)
    {
        const auto [tmp1, tmp2] = interleave_unzip<T, 16>::run(v.template extract<16>(0), v.template extract<16>(1), 16);

        data[0].from_vector(tmp1, shift);
        data[1].from_vector(tmp2, shift);
        zero = false;
    }

    __aie_inline
    accum_type to_accum() const
    {
        accum_type ret;

        if constexpr (AccumBits > 32) {
            const auto vec1 = accum_to_vector_cast<cint32, acc64, 8>::run(data[0].template extract<8>(0));
            const auto vec2 = accum_to_vector_cast<cint32, acc64, 8>::run(data[0].template extract<8>(1));
            const auto vec3 = accum_to_vector_cast<cint32, acc64, 8>::run(data[1].template extract<8>(0));
            const auto vec4 = accum_to_vector_cast<cint32, acc64, 8>::run(data[1].template extract<8>(1));

            const auto [tmp1, tmp2] = interleave_zip<cint32, 8>::run(vec1, vec3, 8);

            ret.insert(0, vector_to_accum_cast<acc64, cint32, 8>::run(tmp1));
            ret.insert(1, vector_to_accum_cast<acc64, cint32, 8>::run(tmp2));

            const auto [tmp3, tmp4] = interleave_zip<cint32, 8>::run(vec2, vec4, 8);

            ret.insert(2, vector_to_accum_cast<acc64, cint32, 8>::run(tmp3));
            ret.insert(3, vector_to_accum_cast<acc64, cint32, 8>::run(tmp4));
        }
        else {
            const auto vec1 = accum_to_vector_cast<int32, acc64, 8>::run(data[0].template extract<8>(0));
            const auto vec2 = accum_to_vector_cast<int32, acc64, 8>::run(data[0].template extract<8>(1));
            const auto vec3 = accum_to_vector_cast<int32, acc64, 8>::run(data[1].template extract<8>(0));
            const auto vec4 = accum_to_vector_cast<int32, acc64, 8>::run(data[1].template extract<8>(1));

            auto vec32_1 = concat_vector(filter<int32, 16, FilterOp::Even>::run(vec1, 1),
                                         filter<int32, 16, FilterOp::Even>::run(vec2, 1));

            auto vec32_2 = concat_vector(filter<int32, 16, FilterOp::Even>::run(vec3, 1),
                                         filter<int32, 16, FilterOp::Even>::run(vec4, 1));

            const auto [tmp1, tmp2] = interleave_zip<int32, 16>::run(vec32_1, vec32_2, 8);

            ret.insert(0, vector_to_accum_cast<acc32, int32, 16>::run(tmp1));
            ret.insert(1, vector_to_accum_cast<acc32, int32, 16>::run(tmp2));
        }

        return ret;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    vector<T, 32> to_vector(int shift = 0) const
    {
        vector<T, 32> ret;

        const auto [tmp1, tmp2] = interleave_zip<T, 16>::run(data[0].template to_vector<T>(shift),
                                                             data[1].template to_vector<T>(shift),
                                                             8);

        ret.insert(0, tmp1);
        ret.insert(1, tmp2);

        return ret;
    }

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto tmp1 = b.template extract<64>(0);
        const auto tmp2 = b.template extract<64>(1);

        this->data[0] = ::mac_2x8_8x8_conf(a.template grow<32>(), a_sign, ::shuffle(tmp1, T16_8x4), b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_2x8_8x8_conf(a.template grow<32>(), a_sign, ::shuffle(tmp2, T16_8x4), b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto tmp1 = b.template extract<64>(0);
        const auto tmp2 = b.template extract<64>(1);

        this->data[0] = ::mul_2x8_8x8(a.template grow<32>(), a_sign, ::shuffle(tmp1, T16_8x4), b_sign);
        this->data[1] = ::mul_2x8_8x8(a.template grow<32>(), a_sign, ::shuffle(tmp2, T16_8x4), b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_16_16<8, 1, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 2>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 32, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // Given a = {a0, a1, a2, a3, a4, a5, a6, a7} the following broadcasts to
        //      ai = {a0, 0, a1, 0, a2, 0, a3, 0, a4, 0, a5, 0, a6, 0, a7, 0}
        // which is used as
        //     {a0,  0,                                         {a0*b0, a0*b1, a0*b2, a0*b3, a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1,  0,  *  {b0, b1, b2, b3, b4, b5, b6, b7, =   a1*b0, a1*b1, a1*b2, a1*b3, a1*b4, a1*b5, a1*b6, a1*b7,
        //      a2,  0,       0,  0,  0,  0,  0,  0,  0,  0}     a2*b0, a2*b1, a2*b2, a2*b3, a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3,  0}                                          a3*b0, a3*b1, a3*b2, a3*b3, a3*b4, a3*b5, a3*b6, a3*b7,
        // and
        //     {a4,  0,                                         {a4*b0, a4*b1, a4*b2, a4*b3, a4*b4, a4*b5, a4*b6, a4*b7,
        //      a5,  0,  *  {b0, b1, b2, b3, b4, b5, b6, b7, =   a5*b0, a5*b1, a5*b2, a5*b3, a5*b4, a5*b5, a5*b6, a5*b7,
        //      a6,  0,       0,  0,  0,  0,  0,  0,  0,  0}     a6*b0, a6*b1, a6*b2, a6*b3, a6*b4, a6*b5, a6*b6, a6*b7,
        //      a7,  0}                                          a7*b0, a7*b1, a7*b2, a7*b3, a7*b4, a7*b5, a7*b6, a7*b7,
        auto ai = ::shuffle(a.template grow<32>(), zeros<TypeA, 32>::run(), T16_2x32_lo);
        this->data[0] = ::mac_4x2_2x8_conf(ai,                         a_sign, b.template grow<32>(), b_sign, this->data[0], this->zero, 0, 0, 0);
        this->data[1] = ::mac_4x2_2x8_conf(::shuffle(ai, T128_4x2_hi), a_sign, b.template grow<32>(), b_sign, this->data[1], this->zero, 0, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto ai = ::shuffle(a.template grow<32>(), zeros<TypeA, 32>::run(), T16_2x32_lo);
        this->data[0] = ::mul_4x2_2x8(ai,                         a_sign, b.template grow<32>(), b_sign);
        this->data[1] = ::mul_4x2_2x8(::shuffle(ai, T128_4x2_hi), a_sign, b.template grow<32>(), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, int16, AccumBits>   : public mmul_16_16<M, K, N, int16,  int16, AccumBits>  { using mmul_16_16<M, K, N, int16, int16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, uint16, AccumBits> : public mmul_16_16<M, K, N, uint16, uint16, AccumBits> { using mmul_16_16<M, K, N, uint16, uint16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, int16, uint16, AccumBits>  : public mmul_16_16<M, K, N, int16,  uint16, AccumBits> { using mmul_16_16<M, K, N, int16,  uint16, AccumBits>::mmul_16_16; };

template <unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul<M, K, N, uint16, int16, AccumBits>  : public mmul_16_16<M, K, N, uint16, int16, AccumBits>  { using mmul_16_16<M, K, N, uint16, int16, AccumBits>::mmul_16_16; };

}

#endif
