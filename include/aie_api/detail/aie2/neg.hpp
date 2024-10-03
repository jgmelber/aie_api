// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_NEG__HPP__
#define __AIE_API_DETAIL_AIE2_NEG__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct neg_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
         if constexpr (Elems < 256) {
             return neg_bits_impl<8, int8, Elems>::run(v.unpack()).pack();
         }
         else {
             vector<T, Elems / 2> res1 = neg_bits_impl<8, int8, Elems / 2>::run(v.template extract<Elems / 2>(0).unpack()).pack();
             vector<T, Elems / 2> res2 = neg_bits_impl<8, int8, Elems / 2>::run(v.template extract<Elems / 2>(1).unpack()).pack();

             return concat_vector(res1, res2);
         }
    }
};

template <typename T, unsigned Elems>
struct neg_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems <= 32) {
            vector<T, 64> tmp;

            tmp = ::neg(v.template grow<64>());

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 64) {
            ret = ::neg(v);
        }
        else if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1 = ::neg(v.template extract<Elems / 2>(0)); ret.insert(0, tmp1);
            const vector<T, Elems / 2> tmp2 = ::neg(v.template extract<Elems / 2>(1)); ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct neg_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems <= 16) {
            vector<T, 32> tmp;

            tmp = ::neg(v.template grow<32>());

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = ::neg(v);
        }
        else if constexpr (Elems == 64) {
            const vector<T, Elems / 2> tmp1 = ::neg(v.template extract<Elems / 2>(0)); ret.insert(0, tmp1);
            const vector<T, Elems / 2> tmp2 = ::neg(v.template extract<Elems / 2>(1)); ret.insert(1, tmp2);
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

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::neg(v.template grow<16>());

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::neg(v);
        }
        else if constexpr (Elems == 32) {
            const vector<T, Elems / 2> tmp1 = ::neg(v.template extract<Elems / 2>(0)); ret.insert(0, tmp1);
            const vector<T, Elems / 2> tmp2 = ::neg(v.template extract<Elems / 2>(1)); ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <unsigned Elems>
struct neg_bits_impl<16, bfloat16, Elems>
{
    using           T = bfloat16;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_neg_elems = 32;
        using native_neg_type = neg_bits_impl<16, T, native_neg_elems>;

        if constexpr (Elems == 8 || Elems == 16) {
            ret = native_neg_type::run(v.template grow<native_neg_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            vector<uint16, Elems> tmp;

            // Negate bfloat values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint16>(v), broadcast<uint16, Elems>::run(0x8000));

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 64) {
            ret = concat_vector(native_neg_type::run(v.template extract<native_neg_elems>(0)),
                                native_neg_type::run(v.template extract<native_neg_elems>(1)));
        }

        return ret;
    }
};

#if __AIE_API_FP32_EMULATION__
template <unsigned Elems>
struct neg_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_neg_elems = 16;
        using native_neg_type = neg_bits_impl<32, T, native_neg_elems>;

        if constexpr (Elems == 4 || Elems == 8) {
            ret = native_neg_type::run(v.template grow<native_neg_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector<uint32, Elems> tmp;

            // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint32>(v), broadcast<uint32, Elems>::run(0x80000000));

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 32) {
            ret = concat_vector(native_neg_type::run(v.template extract<native_neg_elems>(0)),
                                native_neg_type::run(v.template extract<native_neg_elems>(1)));
        }

        return ret;
    }
};
#endif

template <typename T, unsigned Elems>
struct neg_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::neg(v.template grow<8>());

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::neg(v);
        }
        else if constexpr (Elems == 16) {
            const vector<T, Elems / 2> tmp1 = ::neg(v.template extract<Elems / 2>(0)); ret.insert(0, tmp1);
            const vector<T, Elems / 2> tmp2 = ::neg(v.template extract<Elems / 2>(1)); ret.insert(1, tmp2);
        }

        return ret;
    }
};

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems>
struct neg_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_neg_elems = 8;
        using native_neg_type = neg_bits_impl<64, T, native_neg_elems>;

        if constexpr (Elems == 2 || Elems == 4) {
            ret = native_neg_type::run(v.template grow<native_neg_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            vector<uint32, Elems * 2> tmp;

            // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint32>(v), broadcast<uint32, Elems * 2>::run(0x80000000));

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 16) {
            ret = concat_vector(native_neg_type::run(v.template extract<native_neg_elems>(0)),
                                native_neg_type::run(v.template extract<native_neg_elems>(1)));
        }

        return ret;
    }
};
#endif

template <unsigned AccumBits, typename T, unsigned Elems, unsigned NativeNegElems>
struct neg_acc_common_impl
{
    using accum_type = accum<T, Elems>;

    static accum_type run(const accum_type &acc)
    {
        accum_type ret;

        using native_neg_acc_type = neg_acc_bits_impl<AccumBits, T, NativeNegElems>;

        if constexpr (Elems < NativeNegElems) {
            ret = native_neg_acc_type::run(acc.template grow<NativeNegElems>()).template extract<Elems>(0);
        }
        else {
            utils::unroll_times<Elems / NativeNegElems>([&](auto idx){
                ret.insert(idx, ::neg(acc.template extract<NativeNegElems>(idx))); 
            });
        }

        return ret;
    }
};

template <unsigned Elems> struct neg_acc_bits_impl<32,     acc32, Elems> : public neg_acc_common_impl<32,     acc32, Elems, 32> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,     acc64, Elems> : public neg_acc_common_impl<64,     acc64, Elems, 16> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,    cacc64, Elems> : public neg_acc_common_impl<64,    cacc64, Elems,  8> {};
template <unsigned Elems> struct neg_acc_bits_impl<32,  accfloat, Elems> : public neg_acc_common_impl<32,  accfloat, Elems, 16> {};
#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems> struct neg_acc_bits_impl<32, caccfloat, Elems> : public neg_acc_common_impl<32, caccfloat, Elems,  8> {};
#endif

}

#endif
