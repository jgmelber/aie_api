// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_ADD_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE2P_ADD_ACCUM__HPP__

#include "../add.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <AddSubOperation Op>
constexpr auto get_add_sub_accum_op()
{
    if      constexpr (Op == AddSubOperation::Add) return [](const auto &... args) { return ::add_conf(args...); };
    else if constexpr (Op == AddSubOperation::Sub) return [](const auto &... args) { return ::sub_conf(args...); };
}

template <unsigned AccumBits, unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl_common
{
    using vector_type = vector<T, Elems>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag_for_type<T, AccumBits>, Elems2>;

    static constexpr unsigned num_ops = std::max(1u, accum_type<>::bits() / 2048);

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const vector_type &v)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = 2048 / accum_type<>::value_bits();

        if constexpr (accum_type<>::bits() <= 2048) {
            accum_type<native_elems> tmp;

            accum_type<native_elems> acc2;

            if constexpr (type_bits_v<T> * native_elems <= 1024)
                acc2.from_vector(v.template grow<native_elems>());
            else
                acc2 = accum_type<Elems>(v).template grow<native_elems>();

            tmp = op(acc.template grow<native_elems>(), acc2, zero_acc, 0, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else {
            accum_type<> ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                const accum_type<native_elems> tmp(v.template extract<native_elems>(idx));

                ret.template insert<native_elems>(idx, op(acc.template extract<native_elems>(idx), tmp, zero_acc, 0, 0, 0));
            });

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

template <AccumElemBaseType AccumTag, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl_common
{
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<AccumTag, Elems2>;

    static constexpr unsigned num_ops = std::max(1u, accum_type<>::bits() / 2048);

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = 2048 / accum_type<>::value_bits();

        if constexpr (accum_type<>::bits() <= 2048) {
            accum_type<native_elems> tmp;

            tmp = op(acc1.template grow<native_elems>(), acc2.template grow<native_elems>(), zero_acc1, 0, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else {
            accum_type<> ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(acc1.template extract<native_elems>(idx),
                                                          acc2.template extract<native_elems>(idx),
                                                          zero_acc1, 0, 0, 0));
            });

            return ret;
        }
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<accfloat, Elems, Op>
{
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accfloat, Elems2>;

    static constexpr unsigned num_ops = std::max(1u, accum_type<>::bits() / 2048);

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        constexpr auto op = get_add_sub_accum_op<Op>();

        constexpr unsigned native_elems = Elems <= 32 ? 32 : 64;

        if constexpr (accum_type<>::bits() <= 2048) {
            accum_type<native_elems> tmp;

            tmp = op(acc1.template grow<native_elems>(), acc2.template grow<native_elems>(), zero_acc1, 0, 0);

            return tmp.template extract<Elems>(0);
        }
        else {
            accum_type<> ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(acc1.template extract<native_elems>(idx),
                                                          acc2.template extract<native_elems>(idx),
                                                          zero_acc1, 0, 0));
            });

            return ret;
        }
    }
};

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<caccfloat, Elems, Op>
{
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<caccfloat, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc1, bool zero_acc1, const accum_type<> &acc2)
    {
        auto tmp = add_sub_accum_bits_impl<accfloat, Elems * 2, Op>::run(acc1.template cast_to<accfloat>(),
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

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<acc32, Elems, Op>     : public add_sub_accum_bits_impl_common<acc32, Elems, Op> {};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<cacc32, Elems, Op>    : public add_sub_accum_bits_impl_common<cacc32, Elems, Op> {};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<acc64, Elems, Op>     : public add_sub_accum_bits_impl_common<acc64, Elems, Op> {};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_bits_impl<cacc64, Elems, Op>    : public add_sub_accum_bits_impl_common<cacc64, Elems, Op> {};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_t<accum_class_for_type_v<T>, 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    __aie_inline
    static accum_type<> run(const accum_type<> &acc, bool zero_acc, const vector_type &v)
    {
        using accum_op = add_sub_accum_bits_impl<accum_tag, Elems,Op>;

        const accum_type<> acc2(v);

        return accum_op::run(acc, zero_acc, acc2);
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
struct add_sub_accum_vector_bits_impl<32, 16, bfloat16, Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<bfloat16, Elems, Op> {};

#if __AIE_API_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 32, float, Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<float, Elems, Op> {};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 64, cfloat,   Elems, Op> : public add_sub_accum_vector_bits_impl_float_common<cfloat, Elems, Op> {};
#endif

}

#endif
