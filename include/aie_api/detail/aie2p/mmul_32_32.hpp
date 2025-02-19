// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MMUL_32_32__HPP__
#define __AIE_API_DETAIL_AIE2P_MMUL_32_32__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../interleave.hpp"
#include "../broadcast.hpp"
#include "../shuffle.hpp"
#include "../transpose.hpp"
#include "../utils.hpp"

#include "emulated_mmul_intrinsics.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_32_32;

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 2, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x2_2x8_32bx32b(a.template grow<16>(), a_sign, b, b_sign, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x2_2x8_32bx32b(a.template grow<16>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 4, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [a1, a2] = interleave_unzip<TypeA, 8>::run(a.template extract<8>(0), a.template extract<8>(1), 2);

        this->data = ::mac_4x2_2x8_32bx32b(a1.template grow<16>(), a_sign, b.template extract<16>(0), b_sign, this->data, this->zero);
        this->data = ::mac_4x2_2x8_32bx32b(a2.template grow<16>(), a_sign, b.template extract<16>(1), b_sign, this->data, false);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [a1, a2] = interleave_unzip<TypeA, 8>::run(a.template extract<8>(0), a.template extract<8>(1), 2);

        this->data = ::mul_4x2_2x8_32bx32b(a1.template grow<16>(), a_sign, b.template extract<16>(0), b_sign);
        this->data = ::mac_4x2_2x8_32bx32b(a2.template grow<16>(), a_sign, b.template extract<16>(1), b_sign, this->data, false);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 4, 4, TypeA, TypeB, 64>
{
private:
    // Using composition to avoid ambiguity with types
    mmul_32_32<4, 4, 8, TypeA, TypeB, 64> impl;
public:
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;
    using accum_type = accum<acc64, 16>;

    mmul_32_32() = default;

    template <typename T>
    __aie_inline
    explicit mmul_32_32(const vector<T, 16> &v, int shift = 0) :
        impl(wide_vec(v), shift)
    {
    }

    __aie_inline
    explicit mmul_32_32(const accum_type &a, bool to_zero = false) :
        impl(wide_acc(a), to_zero)
    {
    }

    __aie_inline 
    void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto b_4x8 = wide_vec(b);
        impl.mac(a, a_sign, b_4x8, b_sign);
    }

    __aie_inline 
    void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto b_4x8 = concat(interleave_zip<TypeB, 16>::run(b, {}, 4));
        impl.mul(a, a_sign, b_4x8, b_sign);
    }

    // We need to drop the right half of the 4x8 result matrix to get a 4x4 matrix
    template <typename T>
    __aie_inline
    vector<T, 16> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return to_accum().template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    vector<T, 16> to_vector(int shift = 0) const
    {
        return to_accum().template to_vector<T>(shift);
    }

    // We need to drop the right half of the 4x8 result matrix to get a 4x4 matrix
    __aie_inline
    auto to_accum() const
    {
        return narrow_acc(impl.to_accum());
    }

private:
    template <unsigned ElemsIn>
    using vec_to_acc = vector_to_accum_cast<acc64, int32, ElemsIn>;

    template <unsigned ElemsIn>
    using acc_to_vec = accum_to_vector_cast<int32, acc64, ElemsIn>;

    // Creates a matrix vector with twice the number of columns
    // Second half of columns is undefined
    template <typename T, unsigned Elems>
    static vector<T, Elems * 2> wide_vec(const vector<T, Elems> &vec) {
        return concat(interleave_zip<T, Elems>::run(vec, {}, 4));
    }

    static accum<acc64, 32> wide_acc(const accum<acc64, 16> &acc) {
        auto tmp = acc_to_vec<16>::run(acc);
        return vec_to_acc<decltype(tmp)::size() * 2>::run(wide_vec(tmp));
    }

    // Discards the second half of columns in a matrix shaped accumulator
    static accum<acc64, 16> narrow_acc(const accum<acc64, 32> &acc) {
        auto tmp = acc_to_vec<32>::run(acc);
        return vec_to_acc<32>::run(filter<int32, decltype(tmp)::size(), FilterOp::Even>::run(tmp, 8));
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<8, 2, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 64, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<TypeA, TypeB, 64, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x2_2x8_32bx32b(a,                                            a_sign, b, b_sign, this->data[0], this->zero);
        this->data[1] = ::mac_4x2_2x8_32bx32b(a.template extract<8>(1).template grow<16>(), a_sign, b, b_sign, this->data[1], this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x2_2x8_32bx32b(a,                                            a_sign, b, b_sign);
        this->data[1] = ::mul_4x2_2x8_32bx32b(a.template extract<8>(1).template grow<16>(), a_sign, b, b_sign);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_32_32<4, 1, 8, TypeA, TypeB, 64> : public C_block<TypeA, TypeB, 64, 32, 1>
{
    using vector_A_type = vector<TypeA, 4>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 64, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        // Given a = {a0, a1, a2, a3} the following broadcasts to
        //      ai = {a0, 0, a1, 0, a2, 0, a3, 0}
        // which is used as
        //     {a0,  0,                                         {a0*b0, a0*b1, a0*b2, a0*b3, a0*b4, a0*b5, a0*b6, a0*b7,
        //      a1,  0,  *  {b0, b1, b2, b3, b4, b5, b6, b7, =   a1*b0, a1*b1, a1*b2, a1*b3, a1*b4, a1*b5, a1*b6, a1*b7,
        //      a2,  0,       0,  0,  0,  0,  0,  0,  0,  0}     a2*b0, a2*b1, a2*b2, a2*b3, a2*b4, a2*b5, a2*b6, a2*b7,
        //      a3,  0}                                          a3*b0, a3*b1, a3*b2, a3*b3, a3*b4, a3*b5, a3*b6, a3*b7,
        vector<TypeA, 16> ai = ::shuffle(a.template grow<16>(), zeros<TypeA, 16>::run(), T32_2x16_lo);
        this->data = ::mac_4x2_2x8_conf(ai.template grow<32>(), a_sign, b.template grow<16>(), b_sign, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        vector<TypeA, 16> ai = ::shuffle(a.template grow<16>(), zeros<TypeA, 16>::run(), T32_2x16_lo);
        this->data = ::mul_4x2_2x8(ai.template grow<32>(), a_sign, b.template grow<16>(), b_sign);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, int32, 64>   : public mmul_32_32<M, K, N, int32,  int32, 64>  { using mmul_32_32<M, K, N, int32,  int32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, uint32, 64> : public mmul_32_32<M, K, N, uint32, uint32, 64> { using mmul_32_32<M, K, N, uint32, uint32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int32, uint32, 64>  : public mmul_32_32<M, K, N, int32,  uint32, 64> { using mmul_32_32<M, K, N, int32,  uint32, 64>::mmul_32_32; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint32, int32, 64>  : public mmul_32_32<M, K, N, uint32, int32, 64>  { using mmul_32_32<M, K, N, uint32, int32, 64>::mmul_32_32; };


#if __AIE_API_FP32_EMULATION__
template <>
struct mmul_32_32<4, 8, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 32>;
    using vector_B_type = vector<float, 32>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mac_4x8_8x4_fp32(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_4x8_8x4_fp32(a, b);
        this->zero = false;
    }
};

template <>
struct mmul_32_32<4, 1, 4, float, float, 32> : public C_block<float, float, 32, 16, 1>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 4>;

    using C_block<float, float, 32, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
#if __AIE_API_EMULATED_FP32_ZEROIZATION__
        this->data = ::mac_elem_16_conf(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>(), this->data, this->zero, 0, 0);
#else
        this->data = ::mac_elem_16(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>(), this->data);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        this->data = ::mul_elem_16(::shuffle(a.template grow_replicate<16>(), T32_4x4), b.template grow_replicate<16>());
        this->zero = false;
    }
};

template <>
struct mmul_32_32<4, 1, 8, float, float, 32> : public C_block<float, float, 32, 32, 1>
{
    using vector_A_type = vector<float, 4>;
    using vector_B_type = vector<float, 8>;

    using C_block<float, float, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto ai = transpose<float, 32>::run(a.template grow_replicate<32>(), 8, 4);
#if __AIE_API_EMULATED_FP32_ZEROIZATION__
        this->data = ::mac_elem_32_conf(ai, b.template grow_replicate<32>(), this->data, this->zero, 0, 0);
#else
        this->data = ::mac_elem_32(ai, b.template grow_replicate<32>(), this->data);
#endif
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, const bool a_sign, const vector_B_type &b, const bool b_sign)
    {
        auto ai = transpose<float, 32>::run(a.template grow_replicate<32>(), 8, 4);
        this->data = ::mul_elem_32(ai, b.template grow_replicate<32>());
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, float, float, 32>  : public mmul_32_32<M, K, N, float, float, 32>  { using mmul_32_32<M, K, N, float, float, 32>::mmul_32_32; };
#endif

} // namespace aie::detail

#endif
