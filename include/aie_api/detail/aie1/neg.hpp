// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_NEG__HPP__
#define __AIE_API_DETAIL_AIE1_NEG__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct neg_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_neg_elems = 32;
        using native_neg_type = neg_bits_impl<16, T, native_neg_elems>;

        if constexpr (Elems <= 16) {
            ret = native_neg_type::run(v.template grow<native_neg_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<int16, 16> zero(0);

            ret  = ::sub32(zero.template grow<32>(), 0, 0x00000000, 0x00000000, 0x0000,
                                                  v, 0, 0x06040200, 0x0e0c0a08, 0x3210);
        }
        else if constexpr (Elems == 64) {
            const vector<T, 16> zero(0);

            const vector<T, 32> tmp1 = ::sub32(zero.template grow<32>(),  0, 0x00000000, 0x00000000, 0x0000,
                                               v.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210);
            ret.insert(0, tmp1);
            const vector<T, 32> tmp2 = ::sub32(zero.template grow<32>(),  0, 0x00000000, 0x00000000, 0x0000,
                                               v.template extract<32>(1), 0, 0x06040200, 0x0e0c0a08, 0x3210);
            ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct neg_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_neg_elems = 16;
        using native_neg_type = neg_bits_impl<32, T, native_neg_elems>;

        if constexpr (Elems <= 8) {
            ret = native_neg_type::run(v.template grow<native_neg_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            if constexpr (vector_type::is_complex()) {
                const vector<int16, 16> zero(0);
                vector<int16, 32> tmp = vector_cast<int16>(v);

                tmp  = ::sub32(zero.template grow<32>(), 0, 0x00000000, 0x00000000, 0x0000,
                                                    tmp, 0, 0x06040200, 0x0e0c0a08, 0x3210);

                ret = vector_cast<T>(tmp);
            }
            else {
                const vector<T, 8> zero(0);

                ret = ::sub16(zero.template grow<16>(), 0, 0x00000000, 0x00000000,
                                                     v, 0, 0x76543210, 0xfedcba98);
            }
        }
        else if constexpr (Elems == 32) {
            if constexpr (vector_type::is_complex()) {
                const vector<int16, 16> zero(0);
                vector<int16, 64> tmp = vector_cast<int16>(v);

                const vector<int16, 32> tmp1 = ::sub32(zero.template grow<32>(),    0, 0x00000000, 0x00000000, 0x0000,
                                                       tmp.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210);
                tmp.insert(0, tmp1);

                const vector<int16, 32> tmp2 = ::sub32(zero.template grow<32>(),    0, 0x00000000, 0x00000000, 0x0000,
                                                       tmp.template extract<32>(1), 0, 0x06040200, 0x0e0c0a08, 0x3210);
                tmp.insert(1, tmp2);

                ret = vector_cast<T>(tmp);
            }
            else {
                const vector<T, 8> zero(0);

                const vector<T, 16> tmp1 = ::sub16(zero.template grow<16>(),  0, 0x00000000, 0x00000000,
                                                   v.template extract<16>(0), 0, 0x76543210, 0xfedcba98);
                ret.insert(0, tmp1);
                const vector<T, 16> tmp2 = ::sub16(zero.template grow<16>(),  0, 0x00000000, 0x00000000,
                                                   v.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
                ret.insert(1, tmp2);
            }
        }

        return ret;
    }
};

template <unsigned Elems>
struct neg_bits_impl<32, float, Elems>
{
    using vector_type = vector<float, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems == 4) {
            const vector<float, 8> tmp = ::fpneg(v.template grow<16>(), 0, 0x3210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::fpneg(v.template grow<16>(), 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            ret = ::concat(::fpneg(v, 0, 0x76543210),
                           ::fpneg(v, 8, 0x76543210));
        }
        else if constexpr (Elems == 32) {
            const vector<float, 8> tmp1 = ::fpneg(v.template extract<16>(0), 0, 0x76543210); ret.insert(0, tmp1);
            const vector<float, 8> tmp2 = ::fpneg(v.template extract<16>(0), 8, 0x76543210); ret.insert(1, tmp2);
            const vector<float, 8> tmp3 = ::fpneg(v.template extract<16>(1), 0, 0x76543210); ret.insert(2, tmp3);
            const vector<float, 8> tmp4 = ::fpneg(v.template extract<16>(1), 8, 0x76543210); ret.insert(3, tmp4);
        }

        return ret;
    }
};

template <unsigned Elems>
struct neg_bits_impl<64, cint32, Elems>
{
    using vector_type = vector<cint32, Elems>;

    static vector_type run(const vector_type &v)
    {
        return neg_bits<32, int32, Elems * 2>::run(v.template cast_to<int32>()).template cast_to<cint32>();
    }
};

template <unsigned Elems>
struct neg_bits_impl<64, cfloat, Elems>
{
    using vector_type = vector<cfloat, Elems>;

    static vector_type run(const vector_type &v)
    {
        return neg_bits<32, float, Elems * 2>::run(v.template cast_to<float>()).template cast_to<cfloat>();
    }
};

}

#endif
