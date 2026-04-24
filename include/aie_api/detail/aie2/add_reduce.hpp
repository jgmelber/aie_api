// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__

#include "../blend.hpp"
#include "../broadcast.hpp"
#include "../vector.hpp"
#include "../utils.hpp"

namespace aie::detail {

// Necessary because some low precission floating point have no constexpr constructors
// For example, float8 and bfloat8
template <typename T> struct vec_one;

template <>
struct vec_one<bfloat16> {
    __aie_inline
    static vector<bfloat16, 32> run() { return ::broadcast_one_to_v32bfloat16(); }
};

#if __AIE_API_FP16_SUPPORT__
template <>
struct vec_one<float16> {
    __aie_inline
    static vector<float16, 32> run() { return ::broadcast_one_to_v32float16(); }
};
#endif

#if __AIE_API_BF8_SUPPORT__
template <>
struct vec_one<bfloat8> {
    __aie_inline
    static vector<bfloat8, 64> run() { return ::broadcast_one_to_v64bfloat8(); }
};
#endif

#if __AIE_API_FP8_SUPPORT__
template <>
struct vec_one<float8> {
    __aie_inline
    static vector<float8, 64> run() { return ::broadcast_one_to_v64float8(); }
};
#endif

// Reducing via accumulator is useful when reducing floating point numbers,
// which can't be added in vector registers
template <typename T, unsigned Elems>
struct add_reduce_accum;

template <typename T, unsigned Elems>
    requires(is_floating_point_class(accum_class_for_tag_v<T>))
struct add_reduce_accum<T, Elems> {
    using accum_type = accum<T, Elems>;

    template <unsigned E2>
    __aie_inline
    static accum<T, Elems> run(accum<T, E2> acc)
    {
        if constexpr (E2 <= Elems) {
            return acc.template grow<Elems>();
        }
        else {
            using add_op = add_accum<T, E2 / 2>;

            const bool zero_acc = false;
            return run(add_op::run(acc.template extract<E2 / 2>(0), zero_acc,
                                   acc.template extract<E2 / 2>(1)));
        }
    }
};

template <typename T>
    requires(is_floating_point_class(accum_class_for_tag_v<T>))
struct add_reduce_accum<T, 1> {
    using elem_type = std::conditional_t<is_complex_class(accum_class_for_tag_v<T>), cfloat, float>;
    static constexpr unsigned native_elems = 512 / type_bits_v<elem_type>;
    using accum_type = accum<T, native_elems>;

    template <unsigned E2, unsigned Remainder = E2>
    __aie_inline
    static accum_type run(accum<T, E2> acc, std::integral_constant<unsigned, Remainder> = {})
    {
        if constexpr (Remainder == 1) {
            return acc.template grow_extract<native_elems>(0);
        }
        else if constexpr (E2 <= native_elems * 2) {
            constexpr unsigned filter_elems = std::max(E2, native_elems);
            using filter_op = filter<T, filter_elems, FilterOp::Odd>;
            using add_op    = add_accum<T, native_elems>;

            accum<T, native_elems>     lo { acc.template grow_extract<native_elems>(0) };
            accum<T, filter_elems / 2> hi { filter_op::run(acc.template grow<filter_elems>(),
                                                           Remainder / 2) };
            bool zero_acc = false;
            return run(add_op::run(lo, zero_acc, hi.template grow<native_elems>()),
                       std::integral_constant<unsigned, Remainder / 2>{});
        }
        else {
            return run(add_reduce_accum<accfloat, native_elems>::run(acc));
        }
    }
};

template <typename T, unsigned Elems>
struct add_reduce_vector {
    using vector_type = vector<T, Elems>;

    template <unsigned E2>
    __aie_inline
    static vector<T, Elems> run(vector<T, E2> v)
    {
        if constexpr (E2 <= Elems) {
            return v.template grow<Elems>();
        }
        else {
            using add_op = add<T, E2 / 2>;

            return run(add_op::run(v.template extract<E2 / 2>(0),
                                   v.template extract<E2 / 2>(1)));
        }
    }
};

template <typename T>
struct add_reduce_vector<T, 1> {
    static constexpr unsigned native_elems = 512 / type_bits_v<T>;

    template <unsigned E2, unsigned Remainder = E2>
    __aie_inline
    static T run(vector<T, E2> v, std::integral_constant<unsigned, Remainder> = {})
    {
        if constexpr (Remainder == 1) {
            return v[0];
        }
        else if constexpr (E2 <= native_elems) {
            using add_op = add<T, native_elems>;
            using filter_op = filter<T, native_elems, FilterOp::Odd>;

            auto lo = v.template grow<native_elems>();
            auto hi = filter_op::run(lo, Remainder / 2).template grow<native_elems>();
            return run(add_op::run(lo, hi),
                       std::integral_constant<unsigned, Remainder / 2>{});
        }
        else {
            return run(add_reduce_vector<T, native_elems>::run(v));
        }
    }
};

template <typename T, unsigned Elems>
struct add_reduce_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;
    using next_type   = utils::get_next_integer_type_t<T>;

    static T run(const vector_type &v)
    {
        return (T) add_reduce_bits_impl<8, next_type, Elems>::run(v.unpack());
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct add_reduce_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        return add_reduce_vector<T, 1>::run(v);
    }
};

template <typename T, unsigned Elems> struct add_reduce_bits_impl< 8, T, Elems> : public add_reduce_bits_impl_common< 8, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<16, T, Elems> : public add_reduce_bits_impl_common<16, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<32, T, Elems> : public add_reduce_bits_impl_common<32, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<64, T, Elems> : public add_reduce_bits_impl_common<64, T, Elems> {};

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
template <typename T>
static constexpr auto get_add_reduce_mul_op()
{
#if __AIE_ARCH__ == 20
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mul_4x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mul_4x4_4x4(args...); };
#elif __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mul_8x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mul_4x4_4x8(args...); };
#endif
}

template <typename T>
static constexpr auto get_add_reduce_mac_op()
{
#if __AIE_ARCH__ == 20
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mac_4x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mac_4x4_4x4(args...); };
#elif __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mac_8x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mac_4x4_4x8(args...); };
#endif
}

template <typename T, unsigned Elems> requires (!is_complex_v<T>)
struct add_reduce_bits_impl<8, T, Elems>
{
    static constexpr unsigned acc_lanes = __AIE_ARCH__ == 20 ? 32 : 64;
    using acc_type                      = accum<acc32, acc_lanes>;
    using vector_type                   = vector<T, Elems>;

    static constexpr unsigned TypeBits     = 8;
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    using native_op = add_reduce_bits_impl<8, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            constexpr auto mul_op = get_add_reduce_mul_op<T>();
            constexpr auto mac_op = get_add_reduce_mac_op<T>();

            vector<T, native_elems> tmp = v.template grow<native_elems>();

            if constexpr (Elems == 64)
                tmp = ::add(tmp, ::shift_bytes(tmp, vector<T, native_elems>(), 32 * sizeof(T)));
            if constexpr (Elems >= 32)
                tmp = ::add(tmp, ::shift_bytes(tmp, vector<T, native_elems>(), 16 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(::shift_bytes(tmp, vector<T, native_elems>(), 8 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else {
            return native_op::run(add_reduce_vector<T, native_elems>::run(v));
        }
    }
};

template <typename T, unsigned Elems> requires (!is_complex_v<T>)
struct add_reduce_bits_impl<16, T, Elems>
{
    static constexpr unsigned acc_lanes = __AIE_ARCH__ == 20 ? 16 : 32;
    using acc_type                      = accum<acc64, acc_lanes>;
    using vector_type                   = vector<T, Elems>;

    static constexpr unsigned TypeBits     = 16;
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    using native_op = add_reduce_bits_impl<16, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            constexpr auto mul_op = get_add_reduce_mul_op<T>();
            constexpr auto mac_op = get_add_reduce_mac_op<T>();

            vector<T, native_elems> tmp = v.template grow<native_elems>();

            if constexpr (Elems == 32)
                tmp = ::add(tmp, ::shift_bytes(tmp, vector<T, native_elems>(), 16 * sizeof(T)));
            if constexpr (Elems >= 16)
                tmp = ::add(tmp, ::shift_bytes(tmp, vector<T, native_elems>(),  8 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(::shift_bytes(tmp, vector<T, native_elems>(), 4 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else {
            return native_op::run(add_reduce_vector<T, native_elems>::run(v));
        }
    }
};
#endif


#if __AIE_ARCH__ == 20

template <unsigned Elems>
struct add_reduce_bits_impl<16, bfloat16, Elems>
{
    using           T = bfloat16;
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 32;

    __aie_inline
    static T run(const vector_type &v)
    {
        static_assert(Elems >= 8);

        using acc_type = accum<accfloat, 16>;
        const vector<bfloat16, native_elems> ones = vec_one<bfloat16>::run();

        acc_type acc = ::mul_4x8_8x4(v.template grow_extract<native_elems>(0), ones);
        utils::unroll_for<unsigned, 1, Elems / native_elems>([&](unsigned idx) __aie_inline {
            acc = ::mac_4x8_8x4(v.template grow_extract<native_elems>(idx), ones, acc);
        });

        using filter_op = filter<accfloat, 16, FilterOp::Odd>;
        if constexpr (Elems > 16)
            acc = ::add(acc, filter_op::run(acc, 8).grow<16>());
        if constexpr (Elems > 8)
            acc = ::add(acc, filter_op::run(acc, 4).grow<16>());
        return acc.to_vector<bfloat16>()[0];
    }

    __aie_inline
    static T run(const accum<accfloat, Elems> &acc)
    {
        return run(acc.template to_vector<T>());
    }
};

template <unsigned Elems>
struct add_reduce_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 16;
    using native_op = add_reduce_bits_impl<32, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        return run(accum<accfloat, Elems>(v));
    }

    __aie_inline
    static T run(accum<accfloat, Elems> acc)
    {
        std::integral_constant<unsigned, Elems> remainder;
        auto r = add_reduce_accum<accfloat, 1>::run(acc, remainder);
        return r.template to_vector<float>()[0];
    }
};

#elif __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22

template <typename T, unsigned Elems>
struct add_reduce_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;

    static constexpr bool reduce_via_mmul = __AIE_ARCH__ == 22 && type_bits_v<T> == 16;

    __aie_inline
    static auto get_mul_ops() {
        const auto ones = vec_one<T>::run().template grow_replicate<64>();
        if constexpr (type_bits_v<T> == 16) {
            // We pass the reduction values as first argument
            // In the event the number of input elements is 32 or smaller, it is
            // simpler to crop rows in the result matrix than it is to crop columns.
            auto mul_op = [ones](auto a)           { return ::mul_4x8_8x8(a, ones); };
            auto mac_op = [ones](auto a, auto acc) { return ::mac_4x8_8x8(a, ones, acc); };
            return std::tuple(mul_op, mac_op);
        }
        if constexpr (type_bits_v<T> == 8) {
            auto mul_op = [ones](auto a)           { return ::mul_8x8_8x8(a, ones); };
            auto mac_op = [ones](auto a, auto acc) { return ::mac_8x8_8x8(a, ones, acc); };
            return std::tuple(mul_op, mac_op);
        }
    }

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (reduce_via_mmul) {
            constexpr unsigned mul_elems = type_bits_v<T> == 8 ? 64 : 32;
            const auto [mul_op, mac_op] = get_mul_ops();

            vector<T, mul_elems> a;
            a = v.template grow_extract<mul_elems>(0);

            accum<accfloat, mul_elems> acc = mul_op(a);
            utils::unroll_for<unsigned, 1, Elems / mul_elems>([&](unsigned idx) __aie_inline {
                a = v.template grow_extract<mul_elems>(idx);
                acc = mac_op(a, acc);
            });

            // Reduce result of multiplications.
            // Sum first and last two halves together. Only used by float8/bfloat8.
            if constexpr (mul_elems > 32 && Elems > 32) {
                using add_op    = add_accum<accfloat, 32>;
                acc.insert(0, add_op::run(acc.template extract<32>(0), false,
                                          acc.template extract<32>(1)));
            }
            // Sum first two and last two rows together
            using add_op = add_accum<accfloat, 16>;
            if constexpr (Elems > 16) {
                acc.insert(0, add_op::run(acc.template extract<16>(0), false,
                                          acc.template extract<16>(1)));
            }
            // Sum first and second rows
            if constexpr (Elems > 8) {
                using filter_op = filter<accfloat, 16, FilterOp::Odd>;
                accum<accfloat, 16> first_half = acc.template extract<16>(0);
                accum<accfloat, 16> second_row = filter_op::run(first_half, 8).template grow<16>();
                acc.insert(0, add_op::run(first_half, false,
                                          second_row));
            }
            return acc.template to_vector<T>()[0];
        }
        else {
            return run(accum<accfloat, Elems>(v));
        }
    }

    __aie_inline
    static T run(accum<accfloat, Elems> acc)
    {
        if constexpr (reduce_via_mmul) {
            return run(acc.template to_vector<T>());
        } else {
            std::integral_constant<unsigned, Elems> remainder;
            auto r = add_reduce_accum<accfloat, 1>::run(acc, remainder);
            return r.template to_vector<T>()[0];
        }
    }
};

template <unsigned Elems> struct add_reduce_bits_impl<16, bfloat16, Elems> : public add_reduce_bits_impl_float_common<bfloat16, Elems> {};
#if __AIE_API_BF8_SUPPORT__
template <unsigned Elems> struct add_reduce_bits_impl<8, bfloat8, Elems> : public add_reduce_bits_impl_float_common<bfloat8, Elems> {};
#endif
#if __AIE_API_FP8_SUPPORT__
template <unsigned Elems> struct add_reduce_bits_impl<8, float8, Elems> : public add_reduce_bits_impl_float_common<float8, Elems> {};
#endif
#if __AIE_API_FP16_SUPPORT__
template <unsigned Elems> struct add_reduce_bits_impl<16, float16, Elems> : public add_reduce_bits_impl_float_common<float16, Elems> {};
#endif
#if __AIE_API_FP32_EMULATION__
template <unsigned Elems> struct add_reduce_bits_impl<32, float, Elems> : public add_reduce_bits_impl_float_common<float, Elems> {};
#endif

#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__

template <unsigned Elems>
struct add_reduce_bits_impl<64, cfloat, Elems>
{
    using vector_type = vector<cfloat, Elems>;

    __aie_inline
    static cfloat run(const vector_type &v)
    {
        accum<caccfloat, Elems> acc(v);
        return add_reduce_accum<caccfloat, 1>::run(acc).template to_vector<cfloat>()[0];
    }
};

#endif

}

#endif
