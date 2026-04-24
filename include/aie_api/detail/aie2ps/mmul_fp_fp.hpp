// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2PS_MMUL_FP_FP__HPP__
#define __AIE_API_DETAIL_AIE2PS_MMUL_FP_FP__HPP__

#include "../broadcast.hpp"
#include "../transpose.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_fp8_fp8;

template <typename TypeA, typename TypeB>
struct mmul_fp8_fp8<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool /*a_sign*/, const vector_B_type &b, bool /*b_sign*/)
    {
        this->data = ::mac_8x8_8x8_conf(a, b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool /*a_sign*/, const vector_B_type &b, bool /*b_sign*/)
    {
        this->data = ::mul_8x8_8x8(a, b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp8_fp8<4, 16, 16, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type        = vector<TypeA, 64>;
    using vector_B_sparse_type = sparse_vector<TypeB, 256>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool /*a_sign*/, const vector_B_sparse_type &b, bool /*b_sign*/)
    {
        this->data = ::mac_4x16_16x16T_conf(a, b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool /*a_sign*/, const vector_B_sparse_type &b, bool /*b_sign*/)
    {
        this->data = ::mul_4x16_16x16T(a, b);
        this->zero = false;
    }
};


template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_fp16_fp16;

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned Step>
struct C_block_interleave;

template <typename TypeA, typename TypeB, unsigned Elems, unsigned Step>
struct C_block_interleave<TypeA, TypeB, 32, Elems, Step>
{
    static constexpr unsigned NativeElems = 2 * Elems;
    using        accum_type = accum<accfloat, Elems>;
    using native_accum_type = accum<accfloat, NativeElems>;

    native_accum_type data;
    bool              zero;

    __aie_inline
    C_block_interleave() : zero(true)
    {}

    __aie_inline
    C_block_interleave(const native_accum_type &acc, bool to_zero = false) : data(acc), zero(to_zero)
    {}

    __aie_inline
    C_block_interleave(const accum_type &acc, bool to_zero = false)
    {
#if __AIE_API_FP32_EMULATION__
        const auto [v1, v2] = interleave_zip<float, Elems>::run(acc.template to_vector<float>(), vector<float, Elems>(), Step);
        data = native_accum_type(v1.template grow<NativeElems>().insert(1, v2));
#else
        const auto [v1, v2] = interleave_zip<int32, Elems>::run(acc.template cast_to<acc32>()
                                                                   .template to_vector<int32>(),
                                                                vector<int32, Elems>(),
                                                                Step);
        accum<acc32, NativeElems> tmp(v1.template grow<NativeElems>().insert(1, v2));
        data = native_accum_type(tmp.template cast_to<accfloat>());
#endif
        zero = to_zero;
    }

    template <typename T>
    __aie_inline
    C_block_interleave(const vector<T, Elems> &v, int shift = 0)
    {
        const auto [v1, v2] = interleave_zip<T, Elems>::run(v, vector<T, Elems>(), Step);
        data = native_accum_type(v1.template grow<NativeElems>().insert(1, v2));
        zero = false;
    }

    __aie_inline
    accum_type to_accum() const
    {
#if __AIE_API_FP32_EMULATION__
        const auto v = data.template to_vector<float>();
        const auto [result, dummy] = interleave_unzip<float, NativeElems>::run(v, v, Step);
        return accum_type(result.template extract<Elems>(0));
#else
        const auto v = data.template cast_to<acc32>().template to_vector<int32>();
        const auto [result, dummy] = interleave_unzip<int32, NativeElems>::run(v, v, Step);
        accum<acc32, Elems> tmp(result.template extract<Elems>(0));
        return accum_type(tmp.template cast_to<accfloat>());
#endif
    }

    template <typename T>
    __aie_inline
    auto to_vector(int shift = 0) const
    {
        const auto v = data.template to_vector<T>();
        const auto [result, dummy] = interleave_unzip<T, NativeElems>::run(v, v, Step);
        return result.template extract<Elems>(0);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp16_fp16<4, 8, 4, TypeA, TypeB, 32> : public C_block_interleave<TypeA, TypeB, 32, 16, 4>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block_interleave<TypeA, TypeB, 32, 16, 4>::C_block_interleave;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [b1, b2] = interleave_zip<TypeB, 32>::run(b, vector_B_type(), 4);
        auto tmp = b1.template grow<64>().insert(1, b2);

        this->data = ::mac_4x8_8x8_conf(a, tmp, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        const auto [b1, b2] = interleave_zip<TypeB, 32>::run(b, vector_B_type(), 4);
        auto tmp = b1.template grow<64>().insert(1, b2);

        this->data = ::mul_4x8_8x8(a, tmp);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp16_fp16<4, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_4x8_8x8_conf(a, b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_4x8_8x8(a, b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp16_fp16<8, 8, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 2>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 32, 64, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac_4x8_8x8_conf(a.template extract<32>(0), b, this->data[0], this->zero, 0, 0);
        this->data[1] = ::mac_4x8_8x8_conf(a.template extract<32>(1), b, this->data[1], this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul_4x8_8x8(a.template extract<32>(0), b);
        this->data[1] = ::mul_4x8_8x8(a.template extract<32>(1), b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp16_fp16<4, 16, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 32, 1>
{
    using vector_A_type        = vector<TypeA, 64>;
    using vector_B_sparse_type = sparse_vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 32, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mac_4x16_16x8T_conf(a, b, this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_sparse_type &b, bool b_sign)
    {
        this->data = ::mul_4x16_16x8T(a, b);
        this->zero = false;
    }
};

template <typename TypeA, typename TypeB>
struct mmul_fp16_fp16<8, 1, 8, TypeA, TypeB, 32> : public C_block<TypeA, TypeB, 32, 64, 1>
{
    using vector_A_type = vector<TypeA, 8>;
    using vector_B_type = vector<TypeB, 8>;

    using C_block<TypeA, TypeB, 32, 64, 1>::C_block;

    template <MulMacroOp Op>
    using mul_op = mul_bits<Op, 32, 16, TypeA, 16, TypeB>;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_elem_64_conf(transpose<TypeA, 64>::run(a.template grow_replicate<64>(), 8, 8), true,
                                        b.template grow_replicate<64>(), true,
                                        this->data, this->zero, 0, 0);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_elem_64(transpose<TypeA, 64>::run(a.template grow_replicate<64>(), 8, 8),
                                   b.template grow_replicate<64>());
        this->zero = false;
    }
};

#if __AIE_API_FP8_SUPPORT__
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float8,  float8, 32> : public mmul_fp8_fp8<M, K, N,  float8,  float8, 32> { using mmul_fp8_fp8<M, K, N,  float8,  float8, 32>::mmul_fp8_fp8; };
#endif

#if __AIE_API_BF8_SUPPORT__
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat8, bfloat8, 32> : public mmul_fp8_fp8<M, K, N, bfloat8, bfloat8, 32> { using mmul_fp8_fp8<M, K, N, bfloat8, bfloat8, 32>::mmul_fp8_fp8; };
#endif

#if __AIE_API_MIXED_BF8_FP8_SUPPORT__
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float8, bfloat8, 32> : public mmul_fp8_fp8<M, K, N,  float8, bfloat8, 32> { using mmul_fp8_fp8<M, K, N,  float8, bfloat8, 32>::mmul_fp8_fp8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat8,  float8, 32> : public mmul_fp8_fp8<M, K, N, bfloat8,  float8, 32> { using mmul_fp8_fp8<M, K, N, bfloat8,  float8, 32>::mmul_fp8_fp8; };
#endif

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat16, bfloat16, 32> : public mmul_fp16_fp16<M, K, N, bfloat16, bfloat16, 32> { using mmul_fp16_fp16<M, K, N, bfloat16, bfloat16, 32>::mmul_fp16_fp16; };

#if __AIE_API_FP16_SUPPORT__
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float16,  float16, 32> : public mmul_fp16_fp16<M, K, N,  float16,  float16, 32> { using mmul_fp16_fp16<M, K, N,  float16,  float16, 32>::mmul_fp16_fp16; };
#endif

#if __AIE_API_MIXED_BF16_FP16_SUPPORT__
template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  float16, bfloat16, 32> : public mmul_fp16_fp16<M, K, N,  float16, bfloat16, 32> { using mmul_fp16_fp16<M, K, N,  float16, bfloat16, 32>::mmul_fp16_fp16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, bfloat16,  float16, 32> : public mmul_fp16_fp16<M, K, N, bfloat16,  float16, 32> { using mmul_fp16_fp16<M, K, N, bfloat16,  float16, 32>::mmul_fp16_fp16; };
#endif

#if __AIE_API_CBF16_SUPPORT__
template <typename TypeA, typename TypeB, unsigned M, unsigned K, unsigned N, unsigned AccumBits>
struct mmul_cbf16;

template <typename TypeA, typename TypeB>
struct mmul_cbf16<TypeA, TypeB, 2, 8, 4, 32> : public C_block<TypeA, TypeB, 32, 8, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 32, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac_2x8_8x4_conf_t(a, b, this->data, this->zero);
        this->zero = false;
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul_2x8_8x4_t(a, b);
        this->zero = false;
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cbfloat16, cbfloat16, 32> : public mmul_cbf16<cbfloat16, cbfloat16, M, K, N, 32> { using mmul_cbf16<cbfloat16, cbfloat16, M, K, N, 32>::mmul_cbf16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N,  bfloat16, cbfloat16, 32> : public mmul_cbf16< bfloat16, cbfloat16, M, K, N, 32> { using mmul_cbf16< bfloat16, cbfloat16, M, K, N, 32>::mmul_cbf16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, cbfloat16,  bfloat16, 32> : public mmul_cbf16<cbfloat16,  bfloat16, M, K, N, 32> { using mmul_cbf16<cbfloat16,  bfloat16, M, K, N, 32>::mmul_cbf16; };
#endif

//TODO: cfloat

}

#endif
