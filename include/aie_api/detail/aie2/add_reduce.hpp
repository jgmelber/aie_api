// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__

#include "../vector.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct add_reduce_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        if constexpr (Elems <= 128) {
            return (T) add_reduce_bits_impl<8, next_type, Elems>::run(v.unpack());
        }
        else if constexpr (Elems == 256) {
            const T tmp1 = add_reduce_bits_impl<8, next_type, Elems / 2>::run(v.template extract<128>(0).unpack());
            const T tmp2 = add_reduce_bits_impl<8, next_type, Elems / 2>::run(v.template extract<128>(1).unpack());

            return tmp1 + tmp2;
        }
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct add_reduce_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 512 / TypeBits;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (vector_type::bits() <= 512) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), (Elems >> (idx + 1)) * sizeof(T)));
            });

            return (T) tmp[0];
        }
        else if constexpr (vector_type::bits() == 1024) {
            const vector<T, native_elems> tmp = add<T, native_elems>::run(v.template extract<native_elems>(0),
                                                                          v.template extract<native_elems>(1));

            return add_reduce_bits_impl<TypeBits, T, native_elems>::run(tmp);
        }
    }
};

template <typename T, unsigned Elems> struct add_reduce_bits_impl< 8, T, Elems> : public add_reduce_bits_impl_common< 8, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<16, T, Elems> : public add_reduce_bits_impl_common<16, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<32, T, Elems> : public add_reduce_bits_impl_common<32, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<64, T, Elems> : public add_reduce_bits_impl_common<64, T, Elems> {};

template <typename T>
static constexpr auto get_add_reduce_mul_op()
{
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mul_4x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mul_4x4_4x4(args...); };
}

template <typename T>
static constexpr auto get_add_reduce_mac_op()
{
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mac_4x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mac_4x4_4x4(args...); };
}

template <typename T, unsigned Elems> requires (!is_complex_v<T>)
struct add_reduce_bits_impl<8, T, Elems>
{
    static constexpr unsigned acc_lanes = 32;
    using acc_type                      = accum<acc32, acc_lanes>;
    using vector_type                   = vector<T, Elems>;

    static constexpr unsigned TypeBits     = 8;
    static constexpr unsigned native_elems = 512 / TypeBits;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (vector_type::bits() <= 512) {
            constexpr auto mul_op = get_add_reduce_mul_op<T>();
            constexpr auto mac_op = get_add_reduce_mac_op<T>();

            vector<T, native_elems> tmp = v.template grow<native_elems>();

            if constexpr (Elems == 64)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 32 * sizeof(T)));
            if constexpr (Elems >= 32)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 16 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(SHIFT_BYTES(tmp, vector<T, native_elems>(), 8 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else if constexpr (vector_type::bits() == 1024) {
            const vector<T, native_elems> tmp = ::add(v.template extract<native_elems>(0),
                                                      v.template extract<native_elems>(1));

            return add_reduce_bits_impl<TypeBits, T, native_elems>::run(tmp);
        }
    }
};

template <typename T, unsigned Elems> requires (!is_complex_v<T>)
struct add_reduce_bits_impl<16, T, Elems>
{
    static constexpr unsigned acc_lanes = 16;
    using acc_type                      = accum<acc64, acc_lanes>;
    using vector_type                   = vector<T, Elems>;

    static constexpr unsigned TypeBits     = 16;
    static constexpr unsigned native_elems = 512 / TypeBits;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (vector_type::bits() <= 512) {
            constexpr auto mul_op = get_add_reduce_mul_op<T>();
            constexpr auto mac_op = get_add_reduce_mac_op<T>();

            vector<T, native_elems> tmp = v.template grow<native_elems>();

            if constexpr (Elems == 32)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 16 * sizeof(T)));
            if constexpr (Elems >= 16)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(),  8 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(SHIFT_BYTES(tmp, vector<T, native_elems>(), 4 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else if constexpr (vector_type::bits() == 1024) {
            const vector<T, native_elems> tmp = ::add(v.template extract<native_elems>(0),
                                                      v.template extract<native_elems>(1));

            return add_reduce_bits_impl<TypeBits, T, native_elems>::run(tmp);
        }
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<16, bfloat16, Elems>
{
    using           T = bfloat16;
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        using acc_type = accum<accfloat, 16>;
        const vector<bfloat16, 32> ones  = ::broadcast_one_to_v32bfloat16();

        acc_type acc = ::mul_4x8_8x4(v.template grow_extract<32>(0), ones);

        if constexpr (Elems == 64)
            acc = ::mac_4x8_8x4(v.template extract<32>(1), ones, acc);

        if constexpr (Elems > 16)
            acc = ::add(acc, SHIFT_BYTES(acc, acc_type(), 8 * sizeof(float)));
        if constexpr (Elems > 8)
            acc = ::add(acc, SHIFT_BYTES(acc, acc_type(), 4 * sizeof(float)));

        return acc.to_vector<bfloat16>()[0];
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= 16) {
            accum<accfloat, 16> acc(v.template grow<16>());

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                acc = ::add(acc, SHIFT_BYTES(acc, accum<accfloat, 16>(), (Elems >> (idx + 1)) * sizeof(float)));
            });

            return acc.to_vector<float>()[0];
        }
        else if constexpr (Elems == 32) {
            vector<float, 16> tmp = add<float, 16>::run(v.template extract<16>(0), v.template extract<16>(1));

            return add_reduce_bits_impl<32, T, 16>::run(tmp);
        }
    }
};


#if __AIE_API_COMPLEX_FP32_EMULATION__

template <unsigned Elems>
struct add_reduce_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= 8) {
            using accum_type = accum<accfloat, 16>;
            accum_type acc{v.template grow<8>().template cast_to<float>()};

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                acc = ::add(acc, SHIFT_BYTES(acc, accum_type(), (Elems >> (idx + 1)) * sizeof(cfloat)));
            });

            return acc.to_vector<float>().cast_to<cfloat>()[0];
        }
        else if constexpr (Elems == 16) {
            vector<cfloat, 8> tmp = add<cfloat, 8>::run(v.template extract<8>(0), v.template extract<8>(1));

            return add_reduce_bits_impl<64, T, 8>::run(tmp);
        }
    }
};

#endif

}

#endif
