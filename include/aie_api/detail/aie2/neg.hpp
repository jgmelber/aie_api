// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_NEG__HPP__
#define __AIE_API_DETAIL_AIE2_NEG__HPP__

#include "../broadcast.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct neg_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    using next_type = utils::get_next_integer_type_t<T>;
    static constexpr unsigned native_elems = native_vector_length_v<next_type>;
    using native_op = neg_bits_impl<8, next_type, native_elems>;

    static vector_type run(const vector_type &v)
    {
        return neg_bits_impl<8, next_type, Elems>::run(v.unpack()).pack();
    }
};

template <typename T, unsigned Elems>
struct neg_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static constexpr unsigned native_elems = native_vector_length_v<T>;
    using native_op = neg_bits_impl_common<T, native_elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems < native_elems) {
            ret = native_op::run(v.template grow<native_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            ret = ::neg(v);
        }
        else {
            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_op::run(v.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems> struct neg_bits_impl< 8, T, Elems> : neg_bits_impl_common<T, Elems> {};
template <typename T, unsigned Elems> struct neg_bits_impl<16, T, Elems> : neg_bits_impl_common<T, Elems> {};
template <typename T, unsigned Elems> struct neg_bits_impl<32, T, Elems> : neg_bits_impl_common<T, Elems> {};
template <typename T, unsigned Elems> struct neg_bits_impl<64, T, Elems> : neg_bits_impl_common<T, Elems> {};

template <unsigned Elems>
struct neg_bits_impl<16, bfloat16, Elems>
{
    using           T = bfloat16;
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 32;
    using native_op = neg_bits_impl<16, T, native_elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems < native_elems) {
            ret = native_op::run(v.template grow<native_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            vector<uint16, Elems> tmp;

            // Negate bfloat values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint16>(v), broadcast<uint16, Elems>::run(0x8000));

            ret = vector_cast<T>(tmp);
        }
        else {
            vector<uint16, native_elems> tmp;
            auto offset = broadcast<uint16, native_elems>::run(0x8000);

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
                tmp = ::add(vector_cast<uint16>(v.template extract<native_elems>(idx)), offset);
                ret.insert(idx, vector_cast<T>(tmp));
            });
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

    static constexpr unsigned native_elems = 16;
    using native_op = neg_bits_impl<32, T, native_elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems < native_elems) {
            ret = native_op::run(v.template grow<native_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            vector<uint32, Elems> tmp;

            // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint32>(v), broadcast<uint32, Elems>::run(0x80000000));

            ret = vector_cast<T>(tmp);
        }
        else {
            vector<uint32, native_elems> tmp;
            auto offset = broadcast<uint32, native_elems>::run(0x80000000);

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
                tmp = ::add(vector_cast<uint32>(v.template extract<native_elems>(idx)), offset);
                ret.insert(idx, vector_cast<T>(tmp));
            });
        }

        return ret;
    }
};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems>
struct neg_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    static_assert(vector_type::is_signed());

    static constexpr unsigned native_elems = 8;
    using native_op = neg_bits_impl<64, T, native_elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems < native_elems) {
            ret = native_op::run(v.template grow<native_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            vector<uint32, Elems * 2> tmp;

            // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
            tmp = ::add(vector_cast<uint32>(v), broadcast<uint32, Elems * 2>::run(0x80000000));

            ret = vector_cast<T>(tmp);
        }
        else {
            vector<uint32, native_elems * 2> tmp;
            auto offset = broadcast<uint32, native_elems * 2>::run(0x80000000);

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                // Negate float values by flipping the upper bit. XOR is emulated so overflow is leveraged
                tmp = ::add(vector_cast<uint32>(v.template extract<native_elems>(idx)), offset);
                ret.insert(idx, vector_cast<T>(tmp));
            });
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

#if __AIE_ARCH__ == 20
template <unsigned Elems> struct neg_acc_bits_impl<32,     acc32, Elems> : public neg_acc_common_impl<32,     acc32, Elems, 32> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,     acc64, Elems> : public neg_acc_common_impl<64,     acc64, Elems, 16> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,    cacc64, Elems> : public neg_acc_common_impl<64,    cacc64, Elems,  8> {};
template <unsigned Elems> struct neg_acc_bits_impl<32,  accfloat, Elems> : public neg_acc_common_impl<32,  accfloat, Elems, 16> {};
#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems> struct neg_acc_bits_impl<32, caccfloat, Elems> : public neg_acc_common_impl<32, caccfloat, Elems,  8> {};
#endif
#elif __AIE_ARCH__ == 21
template <unsigned Elems> struct neg_acc_bits_impl<32,     acc32, Elems> : public neg_acc_common_impl<32,     acc32, Elems, 64> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,     acc64, Elems> : public neg_acc_common_impl<64,     acc64, Elems, 32> {};
template <unsigned Elems> struct neg_acc_bits_impl<64,    cacc64, Elems> : public neg_acc_common_impl<64,    cacc64, Elems, 16> {};
template <unsigned Elems> struct neg_acc_bits_impl<32,  accfloat, Elems> : public neg_acc_common_impl<32,  accfloat, Elems, 32> {};
#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems> struct neg_acc_bits_impl<32, caccfloat, Elems> : public neg_acc_common_impl<32, caccfloat, Elems, 16> {};
#endif
#endif

}

#endif
