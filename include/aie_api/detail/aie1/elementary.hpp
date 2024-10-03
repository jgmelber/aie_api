// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ELEMENTARY__HPP__
#define __AIE_API_DETAIL_AIE1_ELEMENTARY__HPP__

#include "../vector.hpp"

// TODO: Temporary workaround as these are not properly defined. Remove once they are added to compiler headers.
#if defined(FLOAT2FIX_FAST) || defined(AIE_OPTION_FLOAT2FIX_FAST)
#ifndef float2fix16
#define float2fix16 float2fix16_fast
#endif
#else
#ifndef float2fix16
#define float2fix16 float2fix16_safe
#endif
#endif

namespace aie::detail {

#define PI_ELEMENTARY     3.14159265359
#define INV_PI_ELEMENTARY (1.0/PI_ELEMENTARY)

inline float fpmul_scalar_op(float a, float b)
{
    vector<float, 8> v = ::fpmul(vector<float, 16>(a), 0, 0x0, vector<float, 8>(b), 0, 0x0);
    float c = v[0];

    return c;
}

template <ElementaryOp Op, typename TR>
struct elementary_bits_impl<Op, 32, TR, float>
{
    __aie_inline
    static auto run(const float &a, int shift = 0)
    {
        if      constexpr (Op == ElementaryOp::Sqrt) {
            return ::_sqrtf(a);
        }
        else if constexpr (Op == ElementaryOp::Inv) {
            return ::inv(a);
        }
        else if constexpr (Op == ElementaryOp::InvSqrt) {
            return ::invsqrt(a);
        }
        else if constexpr (Op == ElementaryOp::Cos) {
            int32 temp = ::float2fix(fpmul_scalar_op(a, INV_PI_ELEMENTARY), 31);
            temp = ::sincos(temp).real;

            return ::fix2float(temp, 15);
        }
        else if constexpr (Op == ElementaryOp::Sin) {
            int32 temp = ::float2fix(fpmul_scalar_op(a, INV_PI_ELEMENTARY), 31);
            temp = ::sincos(temp).imag;

            return ::fix2float(temp, 15);
        }
        else if constexpr (Op == ElementaryOp::SinCos) {
            const int32 temp = ::float2fix(fpmul_scalar_op(a, INV_PI_ELEMENTARY), 31);
            const cint16 result = ::sincos(temp);
            const cfloat result_float = ::fix2float(result, 15);

            return std::make_pair(result_float.imag, result_float.real);
        }
        else if constexpr (Op == ElementaryOp::SinCosComplex) {
            const int32 temp = ::float2fix(fpmul_scalar_op(a, INV_PI_ELEMENTARY), 31);
            const cint16 result = ::sincos(temp);
            const cfloat result_float = ::fix2float(result, 15);

            return result_float;
        }

        else if constexpr (Op == ElementaryOp::Float2Fix) {
            return ::float2fix(a, shift);
        }
    }
};

template <typename TR>
struct elementary_bits_impl<ElementaryOp::Float2Fix, 32, TR, float>
{
    __aie_inline
    static TR run(const float &a, int shift = 0)
    {
        return ::float2fix(a, shift);
    }
};

template <ElementaryOp Op, typename TR>
struct elementary_bits_impl<Op, 32, TR, uint32>
{
    __aie_inline
    static auto run(const uint32 &a, int shift = 0)
    {
        if      constexpr (Op == ElementaryOp::Cos) {
            return (::sincos(a)).real;
        }
        else if constexpr (Op == ElementaryOp::Sin) {
            return (::sincos(a)).imag;
        }
        else if constexpr (Op == ElementaryOp::SinCos) {
            cint16 temp = ::sincos(a);
            return std::make_pair(temp.imag, temp.real);
        }
        else if constexpr (Op == ElementaryOp::SinCosComplex)  {
            return ::sincos(a);
        }
    }
};

template <typename TR>
struct elementary_bits_impl<ElementaryOp::Fix2Float, 32, TR, int32>
{
    __aie_inline
    static float run(const int32 &a, int shift = 0)
    {
        return ::fix2float(a, shift);
    }
};

template <ElementaryOp Op, unsigned N>
struct elementary_vector_bits_impl<Op, 32, float, float, N>
{
    using vector_type = vector<float, N>;

    __aie_inline
    static vector_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        vector_type ret;

        if constexpr (vector_type::bits() == 1024) {
            for (unsigned i = 0; i < N; ++i)
                ret[i] = elementary<Op, float, float>::run(v[i]);
        }
        else {
            for (unsigned i = 0; i < N; ++i)
                ret = ret.push(elementary<Op, float, float>::run(v[(N - 1) - i]));
        }

        return ret;
    }
};

template <typename TR, unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Float2Fix, 32, TR, float, N>
{
    using vector_type = vector<float, N>;

#if __AIE_API_FLOAT_CONVERSION_VECTORIZED__ && AIE_API_AIE1_FLOAT_CONVERSION_VECTORIZED
    __aie_inline
    static vector<TR, N> run_int16_vectorized(const vector_type &v, int shift = 0) requires(std::is_same_v<TR, int16>)
    {
        vector<TR, N> ret;
        constexpr unsigned num_iter = N >= 16? N / 16 : 1;

        utils::unroll_times<num_iter>([&](auto i) __aie_inline {
            const vector<TR, 16> tmp = ::float2fix_vectorized(v.template grow_extract<16>(i), shift);

            if constexpr (N < 16)
                ret.template insert(i, tmp.template extract<N>(i));
            else
                ret.template insert(i, tmp);
        });

        return ret;
    }
#endif

    __aie_inline
    static vector<TR, N> run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
#if __AIE_API_FLOAT_CONVERSION_VECTORIZED__ && AIE_API_AIE1_FLOAT_CONVERSION_VECTORIZED
        if constexpr (std::is_same_v<TR, int16>)
            return run_int16_vectorized(v, shift);
#endif

        vector<TR, N> ret;
        constexpr unsigned num_iter = N >= 8? N / 8 : 1;

        utils::unroll_times<num_iter>([&](auto i) __aie_inline {
            vector<TR, 8> tmp;

            if constexpr (std::is_same_v<TR, int16>)
                tmp = ::float2fix16(v.template grow_extract<8>(i), shift);
            else
                tmp = ::float2fix(v.template grow_extract<8>(i), shift);

            if constexpr (N == 4)
                ret.template insert(i, tmp.template extract<4>(i));
            else
                ret.template insert(i, tmp);
        });

        return ret;
    }
};


template <unsigned Bits, typename T, unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Fix2Float, Bits, float, T, N>
{
    using vector_type = vector<T, N>;

#if __AIE_API_FLOAT_CONVERSION_VECTORIZED__ && AIE_API_AIE1_FLOAT_CONVERSION_VECTORIZED
    __aie_inline
    static vector<float, N> run_int16_vectorized(const vector_type &v, int shift = 0) requires(std::is_same_v<T, int16>)
    {
        vector<float, N> ret;

        constexpr unsigned num_iter = N >= 16? N / 16 : 1;

        utils::unroll_times<num_iter>([&](auto i) __aie_inline {
            const vector<float, 16> tmp = ::fix2float_vectorized(v.template grow_extract<16>(i), shift);

            if constexpr (N < 16)
                ret.template insert(i, tmp.template extract<N>(i));
            else
                ret.template insert(i, tmp);
        });

        return ret;
    }
#endif

    __aie_inline
    static vector<float, N> run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
#if __AIE_API_FLOAT_CONVERSION_VECTORIZED__ && AIE_API_AIE1_FLOAT_CONVERSION_VECTORIZED
        if constexpr (std::is_same_v<T, int16>)
            return run_int16_vectorized(v, shift);
#endif

        vector<float, N> ret;

        constexpr unsigned num_iter = N >= 8? N / 8 : 1;

        utils::unroll_times<num_iter>([&](auto i) __aie_inline {
            const vector<float, 8> tmp = ::fix2float(v.template grow_extract<8>(i), shift);

            if constexpr (N == 4)
                ret.template insert(i, tmp.template extract<4>(i));
            else
                ret.template insert(i, tmp);
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::SinCosComplex, 32, float, float, N>
{
    using     vector_type = vector<float, N>;
    using out_vector_type = vector<cfloat, N>;

    __aie_inline
    static out_vector_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        out_vector_type ret;

        constexpr unsigned iter_elems = std::min(N, 8u);

        vector<float, 8> inv_pi_vector;
        inv_pi_vector[0] = INV_PI_ELEMENTARY;

        utils::unroll_times<N/iter_elems>([&](auto idx) __aie_inline {
            const vector<float, 8> temp = ::fpmul(v.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210, inv_pi_vector, 0, 0x00000000);
            const vector<int32, 8> temp_fixed = ::float2fix(temp, 31);

            utils::unroll_times<iter_elems>([&](auto idx2) __aie_inline {
                const cint16 result          = ::sincos(temp_fixed[idx2]);
                const cfloat result_float    = ::fix2float(result, 15);
                ret[idx * iter_elems + idx2] = result_float;
            });
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::SinCos, 32, float, float, N>
{
    using vector_type = vector<float, N>;

    __aie_inline
    static std::pair<vector_type, vector_type> run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        vector_type ret1, ret2;

        constexpr unsigned iter_elems = std::min(N, 8u);

        vector<float, 8> inv_pi_vector;
        inv_pi_vector[0] = INV_PI_ELEMENTARY;

        utils::unroll_times<N/iter_elems>([&](auto idx) __aie_inline {
            const vector<float, 8> temp = ::fpmul(v.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210, inv_pi_vector, 0, 0x00000000);
            const vector<int32, 8> temp_fixed = ::float2fix(temp, 31);

            utils::unroll_times<iter_elems>([&](auto idx2) __aie_inline {
                const cint16 result           = ::sincos(temp_fixed[idx2]);
                const cfloat result_float     = ::fix2float(result, 15);
                ret1[idx * iter_elems + idx2] = result_float.imag;
                ret2[idx * iter_elems + idx2] = result_float.real;
            });
        });

        return std::make_pair(ret1, ret2);
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Sin, 32, float, float, N>
{
    using vector_type = vector<float, N>;

    __aie_inline
    static vector_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        vector_type ret;

        constexpr unsigned iter_elems = std::min(N, 8u);

        vector<float, 8> inv_pi_vector;
        inv_pi_vector[0] = INV_PI_ELEMENTARY;

        utils::unroll_times<N/iter_elems>([&](auto idx) __aie_inline {
            const vector<float, 8> temp = ::fpmul(v.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210, inv_pi_vector, 0, 0x00000000);
            const vector<int32, 8> temp_fixed = ::float2fix(temp, 31);

            utils::unroll_times<iter_elems>([&](auto idx2) __aie_inline {
                const cint16 result          = ::sincos(temp_fixed[idx2]);
                ret[idx * iter_elems + idx2] = ::fix2float(result.imag, 15);
            });
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Cos, 32, float, float, N>
{
    using vector_type = vector<float, N>;

    __aie_inline
    static vector_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        vector_type ret;

        constexpr unsigned iter_elems = std::min(N, 8u);

        vector<float, 8> inv_pi_vector;
        inv_pi_vector[0] = INV_PI_ELEMENTARY;

        utils::unroll_times<N/iter_elems>([&](auto idx) __aie_inline {
            const vector<float, 8> temp = ::fpmul(v.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210, inv_pi_vector, 0, 0x00000000);
            const vector<int32, 8> temp_fixed = ::float2fix(temp, 31);

            utils::unroll_times<iter_elems>([&](auto idx2) __aie_inline {
                const cint16 result          = ::sincos(temp_fixed[idx2]);
                ret[idx * iter_elems + idx2] = ::fix2float(result.real, 15);
            });
        });

        return ret;
    }
};

template <unsigned N, typename TR>
struct elementary_vector_bits_impl<ElementaryOp::SinCos, 32, TR, int32, N>
{
    using     vector_type = vector<int32, N>;
    using out_vector_type = vector<TR, N>;

    __aie_inline
    static std::pair<out_vector_type, out_vector_type> run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        out_vector_type ret1;
        out_vector_type ret2;

        for (unsigned i = 0; i < N; ++i) {
            const auto temp = elementary<ElementaryOp::SinCosComplex, cint16, uint32>::run(uint32(v[i]));
            ret1[i] = temp.imag;
            ret2[i] = temp.real;
        }

        return std::make_pair(ret1, ret2);
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::SinCosComplex, 32, cint16, int32, N>
{
    using     vector_type = vector<int32, N>;
    using out_vector_type = vector<cint16, N>;

    __aie_inline
    static out_vector_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        out_vector_type ret;

        for (unsigned i = 0; i < N; ++i)
            ret[i] = elementary<ElementaryOp::SinCosComplex, cint16, uint32>::run(uint32(v[i]));

        return ret;
    }
};

}

#endif
