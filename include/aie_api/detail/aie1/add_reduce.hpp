// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ADD_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE1_ADD_REDUCE__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct add_reduce_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const T ret1 = add_reduce<int16, 64>::run(v.template extract<64>(0).unpack());
            const T ret2 = add_reduce<int16, 64>::run(v.template extract<64>(1).unpack());

            return ret1 + ret2;
        }
        else {
            return (T) add_reduce<int16, Elems>::run(v.unpack());
        }
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<16, int16, Elems>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        // TODO: use ::mul8 16b x 8b to accumulate 8 values per cycle
        vector<T, 32> v2;

        if constexpr (Elems == 64) {
            v2 = ::add32(v.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210, v.template extract<32>(1),  0, 0x06040200, 0x0e0c0a08, 0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 32) {
            v2 = ::add32(                        v, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 16) {
            v2 = ::add32(    v.template grow<32>(), 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 8) {
            v2 = ::add32(    v.template grow<32>(), 0, 0x0,        0x0,        0x3210,                             4, 0x0,        0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x0,        0x0,        0x3210,                             2, 0x0,        0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x0,        0x0,        0x3210,                             0, 0x0,        0x0,        0x1);
        }

        return v2[0];
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<32, int32, Elems>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        vector<T, 16> v2;

        if constexpr (Elems == 32) {
            v2 = ::add16(v.template extract<16>(0), 0, 0x76543210, 0xfedcba98, v.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 16) {
            v2 = ::add16(                        v, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 8) {
            v2 = ::add16(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 4) {
            v2 = ::add16(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }

        return v2[0];
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<32, cint16, Elems>
{
    using           T = cint16;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &_v)
    {
        vector<int16, Elems * 2> v = vector_cast<int16>(_v);
        vector<int16, 32> v2;

        if constexpr (Elems * 2 == 64) {
            v2 = ::add32(v.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210, v.template extract<32>(1),  0, 0x06040200, 0x0e0c0a08, 0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
        }
        else if constexpr (Elems * 2 == 32) {
            v2 = ::add32(                        v, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
        }
        else if constexpr (Elems * 2 == 16) {
            v2 = ::add32(    v.template grow<32>(), 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
        }
        else if constexpr (Elems * 2 == 8) {
            v2 = ::add32(    v.template grow<32>(), 0, 0x0,        0x0,        0x3210,                             4, 0x0,        0x0,        0x3210);
            v2 = ::add32(                       v2, 0, 0x0,        0x0,        0x3210,                             2, 0x0,        0x0,        0x3210);
        }
        return vector_cast<T>(v2)[0];
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<64, cint32, Elems>
{
    using           T = cint32;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &_v)
    {
        vector<int32, Elems * 2> v = vector_cast<int32>(_v);
        vector<int32, 16> v2;

        if constexpr (Elems * 2 == 32) {
            v2 = ::add16(v.template extract<16>(0), 0, 0x76543210, 0xfedcba98, v.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
        }
        else if constexpr (Elems * 2 == 16) {
            v2 = ::add16(                        v, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
        }
        else if constexpr (Elems * 2 == 8) {
            v2 = ::add16(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = ::add16(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
        }
        else if constexpr (Elems * 2 == 4) {
            v2 = ::add16(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
        }

        return vector_cast<T>(v2)[0];
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<32, float, Elems>
{
    using T = float;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        const vector_type tmp = add_reduce_v<float, Elems>::run(v);
        return tmp.get(0);
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<64, cfloat, Elems>
{
    using T = cfloat;
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        const vector_type tmp = add_reduce_v<cfloat, Elems>::run(v);
        return tmp.get(0);
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<32, float, Elems, 1>
{
    using T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        if constexpr (Elems == 32) {
            vector<T, 8> tmp  = v.template extract<8>(0);
            vector<T, 8> tmp2 = v.template extract<8>(2);

            tmp  = ::fpadd(tmp,  v.template extract<16>(0), 8, 0x76543210);
            tmp2 = ::fpadd(tmp2, v.template extract<16>(1), 8, 0x76543210);

            tmp  = ::fpadd(tmp, tmp2.template grow<16>(), 0, 0x76543210);

            tmp = ::fpadd(tmp,  tmp.template grow<16>(), 4, 0x76543210);
            tmp = ::fpadd(tmp,  tmp.template grow<16>(), 2, 0x76543210);
            tmp = ::fpadd(tmp,  tmp.template grow<16>(), 1, 0x76543210);

            return tmp.template grow<Elems>();
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp = v.template extract<8>(0);

            tmp = ::fpadd(tmp, v,                       8, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 4, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 2, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 1, 0x76543210);

            return tmp.template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            vector<T, 8> tmp;

            tmp = ::fpadd(v,     v.template grow<16>(), 4, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 2, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 1, 0x76543210);

            return tmp;
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> tmp = v.template grow<8>();

            tmp = ::fpadd(tmp,   v.template grow<16>(), 2, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<16>(), 1, 0x76543210);

            return tmp.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<32, float, Elems, 2>
{
    using T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 32) {
            vector<T, 8> tmp1 = ::fpadd(  v1.template extract<8>(0), v1.template extract<16>(0), 8, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 0, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp2 = ::fpadd(  v2.template extract<8>(0), v2.template extract<16>(0), 8, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 0, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 2>::run(tmp1, tmp2).template grow<Elems>();
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = ::fpadd(v1.template extract<8>(0), v1, 8, 0x76543210);
            const vector<T, 8> tmp2 = ::fpadd(v2.template extract<8>(0), v2, 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 2>::run(tmp1, tmp2).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> v_12 = ::fpadd(concat_vector(v1.template extract<4>(0), v2.template extract<4>(0)),
                                              concat_vector(v1, v2), 0, 0xfedc7654);
            vector<T, 8> ret;

            ret = ::fpshuffle( v_12.template grow<16>(), 0, 0x5140);
            ret = ::fpadd(ret, v_12.template grow<16>(), 2, 0x5140);
            ret = ::fpadd(ret,  ret.template grow<16>(), 0, 0x32);

            return ret;

        }
        else if constexpr (Elems == 4) {
            vector<T, 8> ret;

            ret = ::fpshuffle( concat_vector(v1, v2).template grow<16>(), 0, 0x5140);
            ret = ::fpadd(ret, concat_vector(v1, v2).template grow<16>(), 2, 0x5140);
            ret = ::fpadd(ret,                   ret.template grow<16>(), 0, 0x32);

            return ret.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<32, float, Elems, 3>
{
    using T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const vector_type &v3)
    {
        if constexpr (Elems == 32) {
            vector<T, 8> tmp1 = ::fpadd(  v1.template extract<8>(0), v1.template extract<16>(0), 8, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 0, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp2 = ::fpadd(  v2.template extract<8>(0), v2.template extract<16>(0), 8, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 0, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp3 = ::fpadd(  v3.template extract<8>(0), v3.template extract<16>(0), 8, 0x76543210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<16>(1), 0, 0x76543210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<16>(1), 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 3>::run(tmp1, tmp2, tmp3).template grow<Elems>();
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = ::fpadd(v1.template extract<8>(0), v1, 8, 0x76543210);
            const vector<T, 8> tmp2 = ::fpadd(v2.template extract<8>(0), v2, 8, 0x76543210);
            const vector<T, 8> tmp3 = ::fpadd(v3.template extract<8>(0), v3, 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 3>::run(tmp1, tmp2, tmp3).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> v_12 = ::fpadd(concat_vector(v1.template extract<4>(0), v2.template extract<4>(0)),
                                              concat_vector(v1, v2), 0, 0xfedc7654);
            const vector<T, 8> v_3  = ::fpadd(v3, v3.template grow<16>(), 0, 0xfedc7654);
            vector<T, 8> ret;

            ret = ::fpshuffle( concat_vector(v_12, v_3), 0, 0x951840);
            ret = ::fpadd(ret, concat_vector(v_12, v_3), 2, 0x951840);
            ret = ::fpadd(ret,  ret.template grow<16>(), 0, 0x543);

            return ret;
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> ret;

            ret = ::fpshuffle( concat_vector(v1, v2, v3, vector<T, 4>()), 0, 0x951840);
            ret = ::fpadd(ret, concat_vector(v1, v2, v3, vector<T, 4>()), 2, 0x951840);
            ret = ::fpadd(ret,                   ret.template grow<16>(), 0, 0x543);

            return ret.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<32, float, Elems, 4>
{
    using T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const vector_type &v3, const vector_type &v4)
    {
        if constexpr (Elems == 32) {
            vector<T, 8> tmp1 = ::fpadd(  v1.template extract<8>(0), v1.template extract<16>(0), 8, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 0, 0x76543210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp2 = ::fpadd(  v2.template extract<8>(0), v2.template extract<16>(0), 8, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 0, 0x76543210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp3 = ::fpadd(  v3.template extract<8>(0), v3.template extract<16>(0), 8, 0x76543210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<16>(1), 0, 0x76543210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<16>(1), 8, 0x76543210);
            vector<T, 8> tmp4 = ::fpadd(  v4.template extract<8>(0), v4.template extract<16>(0), 8, 0x76543210);
                         tmp4 = ::fpadd(tmp4,                        v4.template extract<16>(1), 0, 0x76543210);
                         tmp4 = ::fpadd(tmp4,                        v4.template extract<16>(1), 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 4>::run(tmp1, tmp2, tmp3, tmp4).template grow<Elems>();
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = ::fpadd(v1.template extract<8>(0), v1, 8, 0x76543210);
            const vector<T, 8> tmp2 = ::fpadd(v2.template extract<8>(0), v2, 8, 0x76543210);
            const vector<T, 8> tmp3 = ::fpadd(v3.template extract<8>(0), v3, 8, 0x76543210);
            const vector<T, 8> tmp4 = ::fpadd(v4.template extract<8>(0), v4, 8, 0x76543210);

            return add_reduce_v_bits_impl<32, float, 8, 4>::run(tmp1, tmp2, tmp3, tmp4).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> v_12 = ::fpadd(concat_vector(v1.template extract<4>(0), v2.template extract<4>(0)),
                                              concat_vector(v1, v2), 0, 0xfedc7654);
            const vector<T, 8> v_34 = ::fpadd(concat_vector(v3.template extract<4>(0), v4.template extract<4>(0)),
                                              concat_vector(v3, v4), 0, 0xfedc7654);

            vector<T, 8> ret;

            ret = ::fpshuffle( concat_vector(v_12, v_34), 0, 0xd951c840);
            ret = ::fpadd(ret, concat_vector(v_12, v_34), 2, 0xd951c840);
            ret = ::fpadd(ret, ret.template grow<16>(),   4, 0x76543210);

            return ret;
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> ret;

            ret = ::fpshuffle( concat_vector(v1, v2, v3, v4), 0, 0xd951c840);
            ret = ::fpadd(ret, concat_vector(v1, v2, v3, v4), 2, 0xd951c840);
            ret = ::fpadd(ret, ret.template grow<16>(),       4, 0x76543210);

            return ret.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<64, cfloat, Elems, 1>
{
    using T = cfloat;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        if constexpr (Elems == 16) {
            vector<T, 4> tmp  = v.template extract<4>(0);
            vector<T, 4> tmp2 = v.template extract<4>(2);

            tmp  = ::fpadd(tmp,  v.template extract<8>(0), 4, 0x3210);
            tmp2 = ::fpadd(tmp2, v.template extract<8>(1), 4, 0x3210);

            tmp  = ::fpadd(tmp, tmp2.template grow<8>(), 0, 0x3210);

            tmp = ::fpadd(tmp,  tmp.template grow<8>(), 2, 0x3210);
            tmp = ::fpadd(tmp,  tmp.template grow<8>(), 1, 0x3210);

            return tmp.template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            vector<T, 4> tmp = v.template extract<4>(0);

            tmp = ::fpadd(tmp, v,                      4, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<8>(), 2, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<8>(), 1, 0x76543210);

            return tmp.template grow<Elems>();
        }
        else if constexpr (Elems == 4) {
            vector<T, 4> tmp;

            tmp = ::fpadd(v,     v.template grow<8>(), 2, 0x76543210);
            tmp = ::fpadd(tmp, tmp.template grow<8>(), 1, 0x76543210);

            return tmp;
        }
        else if constexpr (Elems == 2) {
            vector<T, 4> tmp = v.template grow<4>();

            tmp = ::fpadd(tmp,   v.template grow<8>(), 1, 0x76543210);

            return tmp.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<64, cfloat, Elems, 2>
{
    using T = cfloat;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 16) {
            vector<T, 4> tmp1 = ::fpadd(  v1.template extract<4>(0), v1.template extract<8>(0), 4, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 0, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp2 = ::fpadd(  v2.template extract<4>(0), v2.template extract<8>(0), 4, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 0, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 2>::run(tmp1, tmp2).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 4> tmp1 = ::fpadd(v1.template extract<4>(0), v1, 4, 0x3210);
            const vector<T, 4> tmp2 = ::fpadd(v2.template extract<4>(0), v2, 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 2>::run(tmp1, tmp2).template grow<Elems>();
        }
        else if constexpr (Elems == 4) {
            const vector<T, 4> v_12 = ::fpadd(concat_vector(v1.template extract<2>(0), v2.template extract<2>(0)),
                                              concat_vector(v1, v2), 0, 0x7632);
            vector<T, 4> ret;

            ret = ::fpshuffle( v_12.template grow<8>(), 0, 0x3120);
            ret = ::fpadd(ret,  ret.template grow<8>(), 0, 0x32);

            return ret;

        }
        else if constexpr (Elems == 2) {
            vector<T, 4> ret;

            ret = ::fpshuffle( concat_vector(v1, v2).template grow<8>(), 0, 0x3120);
            ret = ::fpadd(ret,                   ret.template grow<8>(), 0, 0x32);

            return ret.template extract<Elems>(0);
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<64, cfloat, Elems, 3>
{
    using T = cfloat;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const vector_type &v3)
    {
        if constexpr (Elems == 16) {
            vector<T, 4> tmp1 = ::fpadd(  v1.template extract<4>(0), v1.template extract<8>(0), 4, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 0, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp2 = ::fpadd(  v2.template extract<4>(0), v2.template extract<8>(0), 4, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 0, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp3 = ::fpadd(  v3.template extract<4>(0), v3.template extract<8>(0), 4, 0x3210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<8>(1), 0, 0x3210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<8>(1), 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 3>::run(tmp1, tmp2, tmp3).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 4> tmp1 = ::fpadd(v1.template extract<4>(0), v1, 4, 0x3210);
            const vector<T, 4> tmp2 = ::fpadd(v2.template extract<4>(0), v2, 4, 0x3210);
            const vector<T, 4> tmp3 = ::fpadd(v3.template extract<4>(0), v3, 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 3>::run(tmp1, tmp2, tmp3).template grow<Elems>();
        }
        else if constexpr (Elems == 4) {
            const vector<T, 4> v_12 = ::fpadd(concat_vector(v1.template extract<2>(0), v2.template extract<2>(0)),
                                              concat_vector(v1, v2), 0, 0x7632);
            const vector<T, 4> v_3  = ::fpadd(v3, v3.template grow<8>(), 0, 0x7632);
            vector<T, 4> ret;
            vector<T, 8> tmp;

            tmp = ::fpshuffle8(concat_vector(v_12, v_3),   0, 0x531420);
            ret = ::fpadd(tmp.template extract<4>(0), tmp, 0, 0x543);

            return ret;
        }
    }
};

template <unsigned Elems>
struct add_reduce_v_bits_impl<64, cfloat, Elems, 4>
{
    using T = cfloat;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const vector_type &v3, const vector_type &v4)
    {
        if constexpr (Elems == 16) {
            vector<T, 4> tmp1 = ::fpadd(  v1.template extract<4>(0), v1.template extract<8>(0), 4, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 0, 0x3210);
                         tmp1 = ::fpadd(tmp1,                        v1.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp2 = ::fpadd(  v2.template extract<4>(0), v2.template extract<8>(0), 4, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 0, 0x3210);
                         tmp2 = ::fpadd(tmp2,                        v2.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp3 = ::fpadd(  v3.template extract<4>(0), v3.template extract<8>(0), 4, 0x3210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<8>(1), 0, 0x3210);
                         tmp3 = ::fpadd(tmp3,                        v3.template extract<8>(1), 4, 0x3210);
            vector<T, 4> tmp4 = ::fpadd(  v4.template extract<4>(0), v4.template extract<8>(0), 4, 0x3210);
                         tmp4 = ::fpadd(tmp4,                        v4.template extract<8>(1), 0, 0x3210);
                         tmp4 = ::fpadd(tmp4,                        v4.template extract<8>(1), 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 4>::run(tmp1, tmp2, tmp3, tmp4).template grow<Elems>();
        }
        else if constexpr (Elems == 8) {
            const vector<T, 4> tmp1 = ::fpadd(v1.template extract<4>(0), v1, 4, 0x3210);
            const vector<T, 4> tmp2 = ::fpadd(v2.template extract<4>(0), v2, 4, 0x3210);
            const vector<T, 4> tmp3 = ::fpadd(v3.template extract<4>(0), v3, 4, 0x3210);
            const vector<T, 4> tmp4 = ::fpadd(v4.template extract<4>(0), v4, 4, 0x3210);

            return add_reduce_v_bits_impl<64, cfloat, 4, 4>::run(tmp1, tmp2, tmp3, tmp4).template grow<Elems>();
        }
        else if constexpr (Elems == 4) {
            const vector<T, 4> v_12 = ::fpadd(concat_vector(v1.template extract<2>(0), v2.template extract<2>(0)),
                                              concat_vector(v1, v2), 0, 0x7632);
            const vector<T, 4> v_34 = ::fpadd(concat_vector(v3.template extract<2>(0), v4.template extract<2>(0)),
                                              concat_vector(v3, v4), 0, 0x7632);
            vector<T, 4> ret;
            vector<T, 8> tmp;

            tmp = ::fpshuffle8( concat_vector(v_12, v_34), 0, 0x75316420);
            ret = ::fpadd(tmp.template extract<4>(0), tmp, 0, 0x7654);

            return ret;
        }
    }
};

}

#endif
