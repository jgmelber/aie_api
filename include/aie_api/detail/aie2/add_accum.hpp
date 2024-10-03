// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ADD_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE2_ADD_ACCUM__HPP__

#include "../add.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <AddSubOperation Op>
constexpr auto get_add_sub_accum_op()
{
    if      constexpr (Op == AddSubOperation::Add) return [](auto... args) { return ::add_conf(args...); };
    else if constexpr (Op == AddSubOperation::Sub) return [](auto... args) { return ::sub_conf(args...); };
}

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl_common
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, AccumBits>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const vector_type &v)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = 1024 / accum_type<>::value_bits();

        if constexpr (accum_type<>::bits() <= 1024) {
            accum_type<native_elems> tmp;

            const accum_type<native_elems> acc2(v.template grow<native_elems>());

            tmp = op(acc.template grow<native_elems>(), acc2, zero_acc, 0, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (accum_type<>::bits() == 2048) {
            accum_type<> ret;

            const accum_type<native_elems> acc2_1(v.template extract<native_elems>(0));
            const accum_type<native_elems> acc2_2(v.template extract<native_elems>(1));

            ret.template insert<native_elems>(0, op(acc.template extract<native_elems>(0), acc2_1, zero_acc, 0, 0, 0));
            ret.template insert<native_elems>(1, op(acc.template extract<native_elems>(1), acc2_2, zero_acc, 0, 0, 0));

            return ret;
        }
        else if constexpr (accum_type<>::bits() == 4096) {
            accum_type<> ret;

            const accum_type<native_elems> acc2_1(v.template extract<native_elems>(0));
            const accum_type<native_elems> acc2_2(v.template extract<native_elems>(1));
            const accum_type<native_elems> acc2_3(v.template extract<native_elems>(2));
            const accum_type<native_elems> acc2_4(v.template extract<native_elems>(3));

            ret.template insert<native_elems>(0, op(acc.template extract<native_elems>(0), acc2_1, zero_acc, 0, 0, 0));
            ret.template insert<native_elems>(1, op(acc.template extract<native_elems>(1), acc2_2, zero_acc, 0, 0, 0));
            ret.template insert<native_elems>(2, op(acc.template extract<native_elems>(2), acc2_3, zero_acc, 0, 0, 0));
            ret.template insert<native_elems>(3, op(acc.template extract<native_elems>(3), acc2_4, zero_acc, 0, 0, 0));

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, vector_elem_const_ref<T, Elems2> a)
    {
        return run(acc, zero_acc, broadcast<T, Elems>::run(a));
    }

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const T &a)
    {
        return run(acc, zero_acc, broadcast<T, Elems>::run(a));
    }
};

template <unsigned AccumBits, AccumClass Class, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl_common
{
    using   accum_tag = accum_tag_t<Class, AccumBits>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = 1024 / accum_type<>::value_bits();

        if constexpr (accum_type<>::bits() <= 1024) {
            accum_type<native_elems> tmp;

            tmp = op(acc1.template grow<native_elems>(), acc2.template grow<native_elems>(), zero_acc1, 0, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (accum_type<>::bits() == 2048) {
            accum_type<> ret;

            ret.template insert<native_elems>(0, op(acc1.template extract<native_elems>(0), acc2.template extract<native_elems>(0), zero_acc1, 0, 0, 0));
            ret.template insert<native_elems>(1, op(acc1.template extract<native_elems>(1), acc2.template extract<native_elems>(1), zero_acc1, 0, 0, 0));

            return ret;
        }
        else if constexpr (accum_type<>::bits() == 4096) {
            accum_type<> ret;

            ret.template insert<native_elems>(0, op(acc1.template extract<native_elems>(0), acc2.template extract<native_elems>(0), zero_acc1, 0, 0, 0));
            ret.template insert<native_elems>(1, op(acc1.template extract<native_elems>(1), acc2.template extract<native_elems>(1), zero_acc1, 0, 0, 0));
            ret.template insert<native_elems>(2, op(acc1.template extract<native_elems>(2), acc2.template extract<native_elems>(2), zero_acc1, 0, 0, 0));
            ret.template insert<native_elems>(3, op(acc1.template extract<native_elems>(3), acc2.template extract<native_elems>(3), zero_acc1, 0, 0, 0));

            return ret;
        }
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl_common<32, AccumClass::FP, Elems, Op>
{
    static constexpr unsigned AccumBits = 32;

    using   accum_tag = accum_tag_t<AccumClass::FP, 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = 512 / accum_type<>::value_bits();

        if constexpr (accum_type<>::bits() <= 512) {
            accum_type<native_elems> tmp;

            tmp = op(acc1.template grow<native_elems>(), acc2.template grow<native_elems>(), zero_acc1, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (accum_type<>::bits() <= 1024) {
            accum_type<> ret;

            accum_type<native_elems> tmp1, tmp2;

            tmp1 = op(acc1.template extract<native_elems>(0), acc2.template extract<native_elems>(0), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(1), acc2.template extract<native_elems>(1), zero_acc1, 0, 0);

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);

            return ret;
        }
        else if constexpr (accum_type<>::bits() == 2048) {
            accum_type<> ret;

            accum_type<native_elems> tmp1, tmp2;

            tmp1 = op(acc1.template extract<native_elems>(0), acc2.template extract<native_elems>(0), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(1), acc2.template extract<native_elems>(1), zero_acc1, 0, 0);

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);

            tmp1 = op(acc1.template extract<native_elems>(2), acc2.template extract<native_elems>(2), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(3), acc2.template extract<native_elems>(3), zero_acc1, 0, 0);

            ret.insert(2, tmp1);
            ret.insert(3, tmp2);

            return ret;
        }
        else if constexpr (accum_type<>::bits() == 4096) {
            accum_type<> ret;

            accum_type<native_elems> tmp1, tmp2;

            tmp1 = op(acc1.template extract<native_elems>(0), acc2.template extract<native_elems>(0), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(1), acc2.template extract<native_elems>(1), zero_acc1, 0, 0);

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);

            tmp1 = op(acc1.template extract<native_elems>(2), acc2.template extract<native_elems>(2), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(3), acc2.template extract<native_elems>(3), zero_acc1, 0, 0);

            ret.insert(2, tmp1);
            ret.insert(3, tmp2);

            tmp1 = op(acc1.template extract<native_elems>(4), acc2.template extract<native_elems>(4), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(5), acc2.template extract<native_elems>(5), zero_acc1, 0, 0);

            ret.insert(4, tmp1);
            ret.insert(5, tmp2);

            tmp1 = op(acc1.template extract<native_elems>(6), acc2.template extract<native_elems>(6), zero_acc1, 0, 0);
            tmp2 = op(acc1.template extract<native_elems>(7), acc2.template extract<native_elems>(7), zero_acc1, 0, 0);

            ret.insert(6, tmp1);
            ret.insert(7, tmp2);

            return ret;
        }
    }
};

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl_common<32, AccumClass::CFP, Elems, Op>
{
    using   accum_tag = accum_tag_t<AccumClass::CFP, 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        auto tmp = add_sub_accum_bits_impl_common<32, AccumClass::FP, Elems * 2, Op>::run(acc1.template cast_to<accfloat>(),
                                                                                          zero_acc1,
                                                                                          acc2.template cast_to<accfloat>());

        return tmp.template cast_to<caccfloat>();
    }
};
#endif

template <unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, TypeBits, T, Elems, Op> : public add_sub_accum_vector_bits_impl_common<32, TypeBits, T, Elems, Op> {};

template <unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<64, TypeBits, T, Elems, Op> : public add_sub_accum_vector_bits_impl_common<64, TypeBits, T, Elems, Op> {};

template <AccumElemBaseType AccumTag, unsigned Elems, AddSubOperation Op>
    requires(accum<AccumTag, Elems>::accum_bits() == 32)
struct add_sub_accum_bits_impl<AccumTag, Elems, Op> : public add_sub_accum_bits_impl_common<32, accum_class_for_tag_v<AccumTag>, Elems, Op> {};

template <AccumElemBaseType AccumTag, unsigned Elems, AddSubOperation Op>
    requires(accum<AccumTag, Elems>::accum_bits() == 64)
struct add_sub_accum_bits_impl<AccumTag, Elems, Op> : public add_sub_accum_bits_impl_common<64, accum_class_for_tag_v<AccumTag>, Elems, Op> {};


template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_t<(vector_type::is_complex() ? AccumClass::CFP : AccumClass::FP), 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const vector_type &v)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = (vector_type::is_complex()? 8 : 16);

        if constexpr (accum_type<>::bits() <= 512) {
            accum_type<native_elems> tmp;

            const accum_type<native_elems> acc2(v.template grow<native_elems>());

            tmp = op(acc.template grow<native_elems>(), acc2, zero_acc, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (accum_type<>::bits() == 1024) {
            accum_type<> ret;

            const accum_type<native_elems> acc2_1(v.template extract<native_elems>(0));
            const accum_type<native_elems> acc2_2(v.template extract<native_elems>(1));

            ret.template insert<native_elems>(0, op(acc.template extract<native_elems>(0), acc2_1, zero_acc, 0, 0));
            ret.template insert<native_elems>(1, op(acc.template extract<native_elems>(1), acc2_2, zero_acc, 0, 0));

            return ret;
        }
        else if constexpr (accum_type<>::bits() == 2048) {
            accum_type<> ret;

            const accum_type<native_elems> acc2_1(v.template extract<native_elems>(0));
            const accum_type<native_elems> acc2_2(v.template extract<native_elems>(1));
            const accum_type<native_elems> acc2_3(v.template extract<native_elems>(2));
            const accum_type<native_elems> acc2_4(v.template extract<native_elems>(3));

            ret.template insert<native_elems>(0, op(acc.template extract<native_elems>(0), acc2_1, zero_acc, 0, 0));
            ret.template insert<native_elems>(1, op(acc.template extract<native_elems>(1), acc2_2, zero_acc, 0, 0));
            ret.template insert<native_elems>(2, op(acc.template extract<native_elems>(2), acc2_3, zero_acc, 0, 0));
            ret.template insert<native_elems>(3, op(acc.template extract<native_elems>(3), acc2_4, zero_acc, 0, 0));

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, vector_elem_const_ref<T, Elems2> a)
    {
        return run(acc, zero_acc, broadcast<T, Elems>::run(a));
    }

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const T &a)
    {
        return run(acc, zero_acc, broadcast<T, Elems>::run(a));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 32, float,    Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<float,    Elems, Op> {};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 16, bfloat16, Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<bfloat16, Elems, Op> {};

#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 32, cbfloat16, Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<cbfloat16, Elems, Op> {};
#endif

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 64, cfloat,    Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<cfloat,    Elems, Op> {};
#endif

}

#endif
