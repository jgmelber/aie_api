// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_BIT__HPP__
#define __AIE_API_DETAIL_AIE2_BIT__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, BitOp Op>
struct bit_bits_impl<4, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return bit_bits_impl<8, next_type, Elems, Op>::run(v1.unpack(), v2.unpack()).pack();
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    static vector_type run(const T &a, const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return bit_bits_impl<8, next_type, Elems, Op>::run(next_type(a), v.unpack()).pack();
    }

    static vector_type run(const vector_type &v, const T &a)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return bit_bits_impl<8, next_type, Elems, Op>::run(v.unpack(), next_type(a)).pack();
    }

    static vector_type run(const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        vector_type ret;

        if constexpr (vector_type::bits() == 1024) {
            const auto v1_1 = v.template extract<128>(0).unpack();
            const auto v1_2 = v.template extract<128>(1).unpack();

            ret.template insert<128>(0, bit_bits_impl<8, next_type, 128, Op>::run(v1_1).pack());
            ret.template insert<128>(1, bit_bits_impl<8, next_type, 128, Op>::run(v1_2).pack());
        }
        else {
            const auto v1 = v.unpack();
            return bit_bits_impl<8, next_type, Elems, Op>::run(v1).pack();
        }

        return ret;
    }
};

template <unsigned TypeBits, typename T, unsigned Elems, BitOp Op>
struct bit_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    using native_op = bit_bits_impl<TypeBits, T, native_elems, Op>;

    static constexpr auto get_op1()
    {
        if constexpr (Op == BitOp::Not) return [](auto &&... args) __aie_inline { return ::bneg(args...); };
    }

    static constexpr auto get_op2()
    {
        if      constexpr (Op == BitOp::And) return [](auto &&... args) __aie_inline { return ::band(args...); };
        else if constexpr (Op == BitOp::Or)  return [](auto &&... args) __aie_inline { return ::bor(args...); };
        else if constexpr (Op == BitOp::Xor) return [](auto &&... args) __aie_inline { return ::bxor(args...); };
    }

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op2();

        vector_type ret;

        if constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = native_op::run(v1.template grow<native_elems>(),
                                                               v2.template grow<native_elems>());
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            ret = op(v1, v2);
        }
        else {
            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v1.template extract<native_elems>(idx),
                                                                      v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    static vector_type run(const T a, const vector_type &v)
    {
        constexpr auto op = get_op2();

        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    static vector_type run(const vector_type &v, const T a)
    {
        constexpr auto op = get_op2();

        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    static vector_type run(const vector_type &v)
    {
        constexpr auto op = get_op1();

        vector_type ret;

        if constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = native_op::run(v.template grow<native_elems>());
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            ret = op(v);
        }
        else {
            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems, BitOp Op> struct bit_bits_impl< 8, T, Elems, Op> : public bit_bits_impl_common< 8, T, Elems, Op> {};
template <typename T, unsigned Elems, BitOp Op> struct bit_bits_impl<16, T, Elems, Op> : public bit_bits_impl_common<16, T, Elems, Op> {};
template <typename T, unsigned Elems, BitOp Op> struct bit_bits_impl<32, T, Elems, Op> : public bit_bits_impl_common<32, T, Elems, Op> {};
template <typename T, unsigned Elems, BitOp Op> struct bit_bits_impl<64, T, Elems, Op> : public bit_bits_impl_common<64, T, Elems, Op> {};

}

#endif
