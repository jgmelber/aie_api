// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__
#define __AIE_API_DETAIL_AIE2_ADD_REDUCE__HPP__

#include "../blend.hpp"
#include "../broadcast.hpp"
#include "../vector.hpp"
#include "../utils.hpp"

namespace aie::detail {

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

    static constexpr unsigned native_elems = native_vector_length_v<T>;
    using native_op = add_reduce_bits_impl<TypeBits, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            vector<T, native_elems> tmp = v.template grow<native_elems>();

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), (Elems >> (idx + 1)) * sizeof(T)));
            });

            return (T) tmp[0];
        }
        else {
            vector<T, native_elems> tmp;

            if constexpr (Elems == 2 * native_elems)
            {
                tmp = add<T, native_elems>::run(v.template extract<native_elems>(0),
                                                v.template extract<native_elems>(1));
            }
            else
            {
                constexpr unsigned num_ops = Elems / native_elems;
                vector<T, native_elems> tmp1 = v.template extract<native_elems>(0);
                vector<T, native_elems> tmp2 = v.template extract<native_elems>(num_ops / 2);

                utils::unroll_for<unsigned, 1, num_ops / 2>([&](unsigned idx) __aie_inline {
                    tmp1 = add<T, native_elems>::run(tmp1, v.template extract<native_elems>(idx));
                    tmp2 = add<T, native_elems>::run(tmp2, v.template extract<native_elems>(num_ops / 2 + idx));
                });

                tmp = add<T, native_elems>::run(tmp1, tmp2);
            }

            return native_op::run(tmp);
        }
    }
};

template <typename T, unsigned Elems> struct add_reduce_bits_impl< 8, T, Elems> : public add_reduce_bits_impl_common< 8, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<16, T, Elems> : public add_reduce_bits_impl_common<16, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<32, T, Elems> : public add_reduce_bits_impl_common<32, T, Elems> {};
template <typename T, unsigned Elems> struct add_reduce_bits_impl<64, T, Elems> : public add_reduce_bits_impl_common<64, T, Elems> {};

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21
template <typename T>
static constexpr auto get_add_reduce_mul_op()
{
#if __AIE_ARCH__ == 20
    if      constexpr (type_bits_v<T> == 8)  return [](auto &&... args) __aie_inline { return ::mul_4x8_8x8(args...); };
    else if constexpr (type_bits_v<T> == 16) return [](auto &&... args) __aie_inline { return ::mul_4x4_4x4(args...); };
#elif __AIE_ARCH__ == 21
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
#elif __AIE_ARCH__ == 21
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
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 32 * sizeof(T)));
            if constexpr (Elems >= 32)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 16 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(SHIFT_BYTES(tmp, vector<T, native_elems>(), 8 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else {
            vector<T, native_elems> tmp;

            if constexpr (Elems == 2 * native_elems)
            {
                tmp = add<T, native_elems>::run(v.template extract<native_elems>(0),
                                                v.template extract<native_elems>(1));
            }
            else
            {
                constexpr unsigned num_ops = Elems / native_elems;
                vector<T, native_elems> tmp1 = v.template extract<native_elems>(0);
                vector<T, native_elems> tmp2 = v.template extract<native_elems>(num_ops / 2);

                utils::unroll_for<unsigned, 1, num_ops / 2>([&](unsigned idx) __aie_inline {
                    tmp1 = add<T, native_elems>::run(tmp1, v.template extract<native_elems>(idx));
                    tmp2 = add<T, native_elems>::run(tmp2, v.template extract<native_elems>(num_ops / 2 + idx));
                });

                tmp = add<T, native_elems>::run(tmp1, tmp2);
            }

            return native_op::run(tmp);
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
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(), 16 * sizeof(T)));
            if constexpr (Elems >= 16)
                tmp = ::add(tmp, SHIFT_BYTES(tmp, vector<T, native_elems>(),  8 * sizeof(T)));

            const auto ones = broadcast<T, native_elems>::run(1);
            acc_type acc = mul_op(            tmp,                                            ones);
                     acc = mac_op(SHIFT_BYTES(tmp, vector<T, native_elems>(), 4 * sizeof(T)), ones, acc);

            return acc.template to_vector<T>()[0];
        }
        else {
            vector<T, native_elems> tmp;

            if constexpr (Elems == 2 * native_elems)
            {
                tmp = add<T, native_elems>::run(v.template extract<native_elems>(0),
                                                v.template extract<native_elems>(1));
            }
            else
            {
                constexpr unsigned num_ops = Elems / native_elems;
                vector<T, native_elems> tmp1 = v.template extract<native_elems>(0);
                vector<T, native_elems> tmp2 = v.template extract<native_elems>(num_ops / 2);

                utils::unroll_for<unsigned, 1, num_ops / 2>([&](unsigned idx) __aie_inline {
                    tmp1 = add<T, native_elems>::run(tmp1, v.template extract<native_elems>(idx));
                    tmp2 = add<T, native_elems>::run(tmp2, v.template extract<native_elems>(num_ops / 2 + idx));
                });

                tmp = add<T, native_elems>::run(tmp1, tmp2);
            }

            return native_op::run(tmp);
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
        using acc_type = accum<accfloat, 16>;
        const vector<bfloat16, native_elems> ones = ::broadcast_one_to_v32bfloat16();

        acc_type acc = ::mul_4x8_8x4(v.template grow_extract<native_elems>(0), ones);

        if constexpr (Elems > native_elems) {
            utils::unroll_for<unsigned, 1, Elems / native_elems>([&](unsigned idx) __aie_inline {
                acc = ::mac_4x8_8x4(v.template extract<native_elems>(idx), ones, acc);
            });
        }

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

    static constexpr unsigned native_elems = 16;
    using native_op = add_reduce_bits_impl<32, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            accum<accfloat, 16> acc(v.template grow<16>());

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                acc = ::add(acc, SHIFT_BYTES(acc, accum<accfloat, 16>(), (Elems >> (idx + 1)) * sizeof(float)));
            });

            return acc.to_vector<float>()[0];
        }
        else {
            vector<T, native_elems> tmp;

            if constexpr (Elems == 2 * native_elems)
            {
                tmp = add<T, native_elems>::run(v.template extract<native_elems>(0),
                                                v.template extract<native_elems>(1));
            }
            else
            {
                constexpr unsigned num_ops = Elems / native_elems;
                vector<T, native_elems> tmp1 = v.template extract<native_elems>(0);
                vector<T, native_elems> tmp2 = v.template extract<native_elems>(num_ops / 2);

                utils::unroll_for<unsigned, 1, num_ops / 2>([&](unsigned idx) __aie_inline {
                    tmp1 = add<T, native_elems>::run(tmp1, v.template extract<native_elems>(idx));
                    tmp2 = add<T, native_elems>::run(tmp2, v.template extract<native_elems>(num_ops / 2 + idx));
                });

                tmp = add<T, native_elems>::run(tmp1, tmp2);
            }

            return native_op::run(tmp);
        }
    }
};

#elif __AIE_ARCH__ == 21

template <typename T, unsigned Elems>
struct add_reduce_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 32;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems < native_elems) {
            accum<accfloat, 16> acc(v.template grow<16>());

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                const accum<accfloat, 16> hi = SHIFT_BYTES(acc, accum<accfloat, 16>(), (Elems >> (idx + 1)) * sizeof(float));

                acc = ::extract_v16accfloat(::add(acc.template grow<32>(), hi.template grow<32>()), 0);
            });

            return acc.template to_vector<T>()[0];
        }
        else if constexpr (Elems == native_elems) {
            accum<accfloat, 32> acc(v);

            acc = ::add(acc.template grow<32>(), acc.template extract<16>(1).template grow<32>());

            utils::unroll_times<utils::log2(16)>([&](auto idx) __aie_inline {
                const accum<accfloat, 16> hi = SHIFT_BYTES(acc.template extract<16>(0), accum<accfloat, 16>(), (16 >> (idx + 1)) * sizeof(float));

                acc = ::add(acc.template grow<32>(), hi.template grow<32>());
            });

            return acc.template to_vector<T>()[0];

        }
        else
        {
            accum<accfloat, 32> acc(v.template extract<32>(0));

            utils::unroll_for<unsigned, 1, Elems / native_elems>([&](unsigned idx) __aie_inline {
                accum<accfloat, 32> hi(v.template extract<32>(idx));

                acc = ::add(acc, hi);
            });

            acc = ::add(acc, acc.extract<16>(1).grow<32>());

            utils::unroll_times<utils::log2(16)>([&](auto idx) __aie_inline {
                accum<accfloat, 16> hi = SHIFT_BYTES(acc.template extract<16>(0), accum<accfloat, 16>(), (16 >> (idx + 1)) * sizeof(float));

                acc = ::add(acc, hi.template grow<32>());
            });

            return acc.template to_vector<T>()[0];
        }
    }
};

template <unsigned Elems> struct add_reduce_bits_impl<16, bfloat16, Elems> : public add_reduce_bits_impl_float_common<bfloat16, Elems> {};
#if __AIE_API_FP32_EMULATION__
template <unsigned Elems> struct add_reduce_bits_impl<32, float, Elems> : public add_reduce_bits_impl_float_common<float, Elems> {};
#endif

#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__

template <unsigned Elems>
struct add_reduce_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = 8;
    using native_op = add_reduce_bits_impl<64, T, native_elems>;

    __aie_inline
    static T run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            using accum_type = accum<accfloat, 16>;
            accum_type acc{v.template grow<8>().template cast_to<float>()};

            utils::unroll_times<utils::log2(Elems)>([&](auto idx) __aie_inline {
                acc = ::add(acc, SHIFT_BYTES(acc, accum_type(), (Elems >> (idx + 1)) * sizeof(cfloat)));
            });

            return acc.to_vector<float>().cast_to<cfloat>()[0];
        }
        else {
            vector<T, native_elems> tmp = add<cfloat, native_elems>::run(v.template extract<native_elems>(0),
                                                                         v.template extract<native_elems>(1));

            utils::unroll_for<unsigned, 2, Elems / native_elems>([&](unsigned idx) {
                tmp = add<T, native_elems>::run(tmp, v.template extract<native_elems>(idx));
            });

            return native_op::run(tmp);
        }
    }
};

#endif

}

#endif
