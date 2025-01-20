// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ADD_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE1_ADD_ACCUM__HPP__

#include "../add.hpp"

namespace aie::detail {


template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<48, 8, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 48>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const accum_type<Elems / 2> tmp1_1 = acc.template extract<Elems / 2>(0);
            const accum_type<Elems / 2> tmp1_2 = acc.template extract<Elems / 2>(1);

            const vector<T, Elems / 2> tmp2_1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_2 = v.template extract<Elems / 2>(1);

            accum_type<Elems / 2> ret1, ret2;

            ret1 = add_sub_accum_vector_bits_impl<48, 8, T, Elems / 2, Op>::run(tmp1_1, false, tmp2_1);
            ret2 = add_sub_accum_vector_bits_impl<48, 8, T, Elems / 2, Op>::run(tmp1_2, false, tmp2_2);

            return concat_accum(ret1, ret2);
        }
        else {
            const vector<int16, Elems> tmp2 = v.unpack();

            return add_sub_accum_vector_bits_impl<48, 16, int16, Elems, Op>::run(acc, false, tmp2);
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(accum_type<> acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        return run(acc, false, a.get());
    }

    static accum_type<> run(accum_type<> acc, bool /* acc_zero */, const T &a)
    {
        if constexpr (Elems == 128) {
            const accum_type<Elems / 2> tmp_1 = acc.template extract<Elems / 2>(0);
            const accum_type<Elems / 2> tmp_2 = acc.template extract<Elems / 2>(1);

            accum_type<Elems / 2> ret1, ret2;

            ret1 = add_sub_accum_vector_bits_impl<48, 8, T, Elems / 2, Op>::run(tmp_1, false, a);
            ret2 = add_sub_accum_vector_bits_impl<48, 8, T, Elems / 2, Op>::run(tmp_2, false, a);

            return concat_accum(ret1, ret2);
        }
        else {
            return add_sub_accum_vector_bits_impl<48, 16, int16, Elems, Op>::run(acc, false, (int16)a);
        }
    }
};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<48, 16, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 48>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    using binary_op = std::conditional_t<Op == AddSubOperation::Add, std::plus<>, std::minus<>>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == AddSubOperation::Add) return [](auto... params) { return ::mac16(params...); };
        else if constexpr (Op == AddSubOperation::Sub) return [](auto... params) { return ::msc16(params...); };
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        binary_op combine;
        if constexpr (Elems <= 16) {
            accum_type<> ret = combine(acc.to_native(), v.to_native());
            return ret;
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<16>(0, combine(acc.template extract<16>(0), v.template extract<16>(0)));
            ret.template insert<16>(1, combine(acc.template extract<16>(1), v.template extract<16>(1)));

            return ret;
        }
        else if constexpr (Elems == 64) {
            accum_type<> ret;

            ret.template insert<16>(0, combine(acc.template extract<16>(0), v.template extract<16>(0)));
            ret.template insert<16>(1, combine(acc.template extract<16>(1), v.template extract<16>(1)));
            ret.template insert<16>(2, combine(acc.template extract<16>(2), v.template extract<16>(2)));
            ret.template insert<16>(3, combine(acc.template extract<16>(3), v.template extract<16>(3)));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();
        const vector<int32, 8> one(1);

        if constexpr (Elems <= 16) {
            accum_type<16> ret;

            ret = op(acc.template grow<16>(), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<16>(0,
                    op(acc.template extract<16>(0), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));
            ret.template insert<16>(1,
                    op(acc.template extract<16>(1), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));

            return ret;
        }
        else if constexpr (Elems == 64) {
            accum_type<> ret;

            ret.template insert<16>(0, op(acc.template extract<16>(0), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));
            ret.template insert<16>(1, op(acc.template extract<16>(1), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));
            ret.template insert<16>(2, op(acc.template extract<16>(2), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));
            ret.template insert<16>(3, op(acc.template extract<16>(3), a.parent.template grow<std::max(Elems2, 32u)>(), a.offset, 0x00000000, 0x00000000, one, 0, 0x00000000, 0x00000000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector_type tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<48, 32, int32, Elems, Op>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 48>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add) {
            if      constexpr (Elems <= 8)  return [](auto... params) { return ::mac8(params...); };
            else if constexpr (Elems >= 16) return [](auto... params) { return ::mac16(params...); };
        }
        else {
            if      constexpr (Elems <= 8)  return [](auto... params) { return ::msc8(params...); };
            else if constexpr (Elems >= 16) return [](auto... params) { return ::msc16(params...); };
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Elems == 8) {
            const vector<int16, 16> one_zero(1, 0);
            const accum_type<8> ret = op(acc, v.template grow<16>(), 0, 0x76543210, 0,
                                                           one_zero, 0, 0x00000000, 1);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<int16, 16> one(1);
            accum_type<16> ret;

            ret = op(acc, v, 0, 0x76543210, 0xfedcba98, one, 0, 0x00000000, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<int16, 16> one(1);
            accum_type<> ret;

            ret.template insert<16>(0, op(acc.template extract<16>(0), v.template extract<16>(0), 0, 0x76543210, 0xfedcba98, one, 0, 0x00000000, 0x00000000));
            ret.template insert<16>(1, op(acc.template extract<16>(1), v.template extract<16>(1), 0, 0x76543210, 0xfedcba98, one, 0, 0x00000000, 0x00000000));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems == 8) {
            const vector<int16, 16> one_zero(1, 0);
            const accum_type<8> ret = op(acc, a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0,
                                                                                     one_zero,        0, 0x00000000, 1);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<int16, 16> one(1);
            const accum_type<> ret = op(acc, a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                                                                         one,        0, 0x00000000, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<int16, 16> one(1);
            accum_type<> ret;

            ret.template insert<16>(0,
                op(acc.template extract<16>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                                                                            one,        0, 0x00000000, 0x00000000));
            ret.template insert<16>(1,
                op(acc.template extract<16>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                                                                            one,        0, 0x00000000, 0x00000000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<48, 32, cint16, Elems, Op>
{
    using           T = cint16;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 48>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == AddSubOperation::Add) return [](auto... params) { return ::mac8(params...); };
        else if constexpr (Op == AddSubOperation::Sub) return [](auto... params) { return ::msc8(params...); };
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        constexpr auto op = get_op();

        const vector<int32, 8> one(1);

        if constexpr (Elems <= 8) {
            const accum_type<8> ret = op(acc.template grow<8>(), v.template grow<16>(), 0, 0x76543210,
                                                                                   one, 0, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<8>(0, op(acc.template extract<8>(0), v, 0, 0x76543210, 0, one, 0, 0x00000000));
            ret.template insert<8>(1, op(acc.template extract<8>(1), v, 8, 0x76543210, 0, one, 0, 0x00000000));

            return ret;
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<8>(0, op(acc.template extract<8>(0), v, 0,  0x76543210, 0, one, 0, 0x00000000));
            ret.template insert<8>(1, op(acc.template extract<8>(1), v, 8,  0x76543210, 0, one, 0, 0x00000000));
            ret.template insert<8>(2, op(acc.template extract<8>(2), v, 16, 0x76543210, 0, one, 0, 0x00000000));
            ret.template insert<8>(3, op(acc.template extract<8>(3), v, 24, 0x76543210, 0, one, 0, 0x00000000));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        const vector<int32, 8> one(1);

        if constexpr (Elems <= 8) {
            const accum_type<8> ret = op(acc.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                                             one,        0, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<8>(0,
                op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));
            ret.template insert<8>(1,
                op(acc.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));
            return ret;
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<8>(0,
                op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));
            ret.template insert<8>(1,
                op(acc.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));
            ret.template insert<8>(2,
                op(acc.template extract<8>(2), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));
            ret.template insert<8>(3,
                op(acc.template extract<8>(3), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000,
                                                                                           one,        0, 0x00000000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<48, 64, cint32, Elems, Op>
{
    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 48>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add) {
            if      constexpr (Elems <= 4) return [](auto... params) { return ::mac4(params...); };
            else if constexpr (Elems >= 8) return [](auto... params) { return ::mac8(params...); };
        }
        else {
            if      constexpr (Elems <= 4) return [](auto... params) { return ::msc4(params...); };
            else if constexpr (Elems >= 8) return [](auto... params) { return ::msc8(params...); };
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<int16, 16> one_zero(1, 0);

            const accum_type<4> ret = op(acc.template grow<4>(), v.template grow<8>(), 0, 0x3210, 0,
                                                                             one_zero, 0, 0x0000, 1);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<int16, 16> one(1);
            accum_type<> ret;

            ret = op(acc, v, 0, 0x76543210, one, 0, 0x00000000);

            return ret;
        }
        else if constexpr (Elems == 16) {
            const vector<int16, 16> one(1);
            accum_type<> ret;

            ret.template insert<8>(0, op(acc, v, 0, 0x76543210, one, 0, 0x00000000));
            ret.template insert<8>(1, op(acc, v, 8, 0x76543210, one, 0, 0x00000000));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<int16, 16> one_zero(1, 0);

            const accum_type<4> ret = op(acc.template grow<4>(), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x00000000, 0,
                                                                                                                 one_zero, 0x00000000, 1);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<int16, 16> one(1);
            accum_type<16> ret = op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 8u)>, 0, 0x00000000,
                                                                                                         one, 0, 0x00000000);

            return ret;
        }
        else if constexpr (Elems == 32) {
            const vector<int16, 16> one(1);
            accum_type<> ret;

            ret.template insert<8>(0,
                op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 8u)>, 0, 0x00000000,
                                                                                        one, 0, 0x00000000));
            ret.template insert<8>(1,
                op(acc.template extract<8>(1), a.parent.template grow<std::max(Elems2, 8u)>, 0, 0x00000000,
                                                                                        one, 0, 0x00000000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<80, 32, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 80>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    using binary_op = std::conditional_t<Op == AddSubOperation::Add, std::plus<>, std::minus<>>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == AddSubOperation::Add) return [](auto... params) { return ::lmac8(params...); };
        else if constexpr (Op == AddSubOperation::Sub) return [](auto... params) { return ::lmsc8(params...); };
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        binary_op combine;
        if constexpr (Elems <= 8) {
            accum_type<> ret;

            ret = combine(acc.to_native(), v.to_native());

            return ret;
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<8>(0, combine(acc.template extract<8>(0), v.template extract<8>(0)));
            ret.template insert<8>(1, combine(acc.template extract<8>(1), v.template extract<8>(1)));

            return ret;
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<8>(0, combine(acc.template extract<8>(0), v.template extract<8>(0)));
            ret.template insert<8>(1, combine(acc.template extract<8>(1), v.template extract<8>(1)));
            ret.template insert<8>(2, combine(acc.template extract<8>(2), v.template extract<8>(2)));
            ret.template insert<8>(3, combine(acc.template extract<8>(3), v.template extract<8>(3)));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        const vector<int32, 8> one(1);

        if constexpr (Elems <= 8) {
            accum_type<8> ret;

            ret = op(acc.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<8>(0, op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));
            ret.template insert<8>(1, op(acc.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));

            return ret;
        }
        else if constexpr (Elems == 32) {
            accum_type<> ret;

            ret.template insert<8>(0, op(acc.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));
            ret.template insert<8>(1, op(acc.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));
            ret.template insert<8>(2, op(acc.template extract<8>(2), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));
            ret.template insert<8>(3, op(acc.template extract<8>(3), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, one, 0, 0x00000000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<80, 64, cint32, Elems, Op>
{
    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 80>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    using binary_op = std::conditional_t<Op == AddSubOperation::Add, std::plus<>, std::minus<>>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == AddSubOperation::Add) return [](auto... params) { return ::lmac4(params...); };
        else if constexpr (Op == AddSubOperation::Sub) return [](auto... params) { return ::lmsc4(params...); };
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        binary_op combine;
        if constexpr (Elems <= 4) {
            accum_type<> ret;

            ret = combine(acc, v);

            return ret;
        }
        else if constexpr (Elems == 8) {
            accum_type<> ret;

            ret.template insert<4>(0, combine(acc.template extract<4>(0), v.template extract<4>(0)));
            ret.template insert<4>(1, combine(acc.template extract<4>(1), v.template extract<4>(1)));

            return ret;
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<4>(0, combine(acc.template extract<4>(0), v.template extract<4>(0)));
            ret.template insert<4>(1, combine(acc.template extract<4>(1), v.template extract<4>(1)));
            ret.template insert<4>(2, combine(acc.template extract<4>(2), v.template extract<4>(2)));
            ret.template insert<4>(3, combine(acc.template extract<4>(3), v.template extract<4>(3)));

            return ret;
        }
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();
        const vector<int32, 8> one(1);

        if constexpr (Elems <= 4) {
            accum_type<4> ret = op(acc.template grow<4>(), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            accum_type<> ret;

            ret.template insert<4>(0, op(acc.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));
            ret.template insert<4>(1, op(acc.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));

            return ret;
        }
        else if constexpr (Elems == 16) {
            accum_type<> ret;

            ret.template insert<4>(0, op(acc.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));
            ret.template insert<4>(1, op(acc.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));
            ret.template insert<4>(2, op(acc.template extract<4>(2), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));
            ret.template insert<4>(3, op(acc.template extract<4>(3), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000, one, 0, 0x0000));

            return ret;
        }
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 8> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};


template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 32, float, Elems, Op>
{
    using           T = float;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        const accum_type<> ret(add_sub_bits_impl<32, float, Elems, Op>::run(acc.template to_vector<float>(), v));

        return ret;
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        if constexpr (Op == AddSubOperation::Add)
            return accum_type<>(add_sub_bits_impl<32, float, Elems, Op>::run(a, acc.template to_vector<float>()));
        else
            return accum_type<>(add_sub_bits_impl<32, float, Elems, Op>::run(acc.template to_vector<float>(), a));
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_accum_vector_bits_impl<32, 64, cfloat, Elems, Op>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;
    using   accum_tag = accum_tag_for_type<T, 32>;
    template <unsigned Elems2 = Elems>
    using  accum_type = accum<accum_tag, Elems2>;

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const vector_type &v)
    {
        const accum_type<> ret(add_sub_bits_impl<64, cfloat, Elems, Op>::run(acc.template to_vector<cfloat>(), v));

        return ret;
    }

    template <unsigned Elems2>
    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, vector_elem_const_ref<T, Elems2> a)
    {
        if constexpr (Op == AddSubOperation::Add)
            return accum_type<>(add_sub_bits_impl<64, cfloat, Elems, Op>::run(a, acc.template to_vector<cfloat>()));
        else
            return accum_type<>(add_sub_bits_impl<64, cfloat, Elems, Op>::run(acc.template to_vector<cfloat>(), a));
    }

    static accum_type<> run(const accum_type<> &acc, bool /* acc_zero */, const T &a)
    {
        const vector<T, 8> tmp(a);

        return run(acc, false, tmp.elem_const_ref(0));
    }
};

}

#endif
