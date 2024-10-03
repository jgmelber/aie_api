// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ADD__HPP__
#define __AIE_API_DETAIL_AIE1_ADD__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<8, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1_1 = v1.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp1_2 = v1.template extract<Elems / 2>(1);

            const vector<T, Elems / 2> tmp2_1 = v2.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_2 = v2.template extract<Elems / 2>(1);

            vector_type ret;

            ret.insert(0, add_sub_bits_impl<8, T, Elems / 2, Op>::run(tmp1_1, tmp2_1));
            ret.insert(1, add_sub_bits_impl<8, T, Elems / 2, Op>::run(tmp1_2, tmp2_2));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp1 = v1.unpack();
            const vector<int16, Elems> tmp2 = v2.unpack();

            return add_sub_bits_impl<16, int16, Elems, Op>::run(tmp1, tmp2).template pack<T>();
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);

            vector_type ret;

            ret.insert(0, add_sub_bits_impl<8, T, Elems / 2, Op>::run(a, tmp_1));
            ret.insert(1, add_sub_bits_impl<8, T, Elems / 2, Op>::run(a, tmp_2));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return add_sub_bits_impl<16, int16, Elems, Op>::run((int16)a, tmp).template pack<T>();
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);

            vector_type ret;

            ret.insert(0, add_sub_bits_impl<8, T, Elems / 2, Op>::run(tmp_1, (int16)a));
            ret.insert(1, add_sub_bits_impl<8, T, Elems / 2, Op>::run(tmp_2, (int16)a));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return add_sub_bits_impl<16, int16, Elems, Op>::run(tmp, (int16)a).template pack<T>();
        }
    }
};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<16, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add)
            return [](auto &&...params) { return ::add32(params...); };
        else if constexpr (Op == AddSubOperation::Sub)
            return [](auto &&...params) { return ::sub32(params...); };
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 32) {
            vector<T, 32> ret;

            if (chess_manifest(utils::is_same_object(v1, v2))) {
                ret = op(v1.template grow<32>(), 0, 0x06040200, 0x0e0c0a08, 0x3210,
                                                 0, 0x06040200, 0x0e0c0a08, 0x3210);
            }
            else {
                ret = op(v1.template grow<32>(), v2.template grow<32>());
            }

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp;
            vector_type ret;

            if (chess_manifest(utils::is_same_object(v1, v2))) {
                tmp = op(v1.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x06040200, 0x0e0c0a08, 0x3210); ret.insert(0, tmp);
                tmp = op(v1.template extract<32>(1), 0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x06040200, 0x0e0c0a08, 0x3210); ret.insert(1, tmp);
            }
            else {
                tmp = op(v1.template extract<32>(0), v2.template extract<32>(0)); ret.insert(0, tmp);
                tmp = op(v1.template extract<32>(1), v2.template extract<32>(1)); ret.insert(1, tmp);
            }

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 32) {
            const unsigned odd = a.offset & 0x1;
            const int offset = a.offset - odd;
            const int square = 0x1111 * odd;

            vector<T, 32> ret;

            if (chess_manifest(utils::is_same_object(a.parent, v))) {
                ret = op(                          v.template grow<32>(), offset, 0x00000000, 0x00000000, square,
                                                                               0, 0x06040200, 0x0e0c0a08, 0x3210);
            }
            else {
                ret = op(a.parent.template grow<std::max(Elems2, 32u)>(), offset, 0x00000000, 0x00000000, square,
                                                   v.template grow<32>(),      0, 0x06040200, 0x0e0c0a08, 0x3210);
            }

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            if constexpr (Elems2 == 64) {
                // TODO: Investigate more efficient implementation if offset is known at compile-time
                ret = run(a.get(), v);
            }
            else {
                ret.insert(0, add_sub_bits_impl<16, T, 32, Op>::run(a, v.template extract<32>(0)));
                ret.insert(1, add_sub_bits_impl<16, T, 32, Op>::run(a, v.template extract<32>(1)));
            }

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 32) {
            const unsigned odd = a.offset & 0x1;
            const int offset = a.offset - odd;
            const int square = 0x1111 * odd;

            vector<T, 32> ret;

            if (chess_manifest(utils::is_same_object(a.parent, v))) {
                ret = op(                          v.template grow<32>(), offset, 0x00000000, 0x00000000, square,
                                                                               0, 0x06040200, 0x0e0c0a08, 0x3210);
            }
            else {
                ret = op(                          v.template grow<32>(),      0, 0x06040200, 0x0e0c0a08, 0x3210,
                         a.parent.template grow<std::max(Elems2, 32u)>(), offset, 0x00000000, 0x00000000, square);
            }

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            if constexpr (Elems2 == 64) {
                // TODO: Investigate more efficient implementation if offset is known at compile-time
                ret = run(v, a.get());
            }
            else {
                ret.insert(0, add_sub_bits_impl<16, T, 32, Op>::run(v.template extract<32>(0), a));
                ret.insert(1, add_sub_bits_impl<16, T, 32, Op>::run(v.template extract<32>(1), a));
            }

            return ret;
        }
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        const vector<T, 32> tmp(a);

        return run(tmp.elem_const_ref(0), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        const vector<T, 32> tmp(a);

        return run(v, tmp.elem_const_ref(0));
    }
};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<32, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (vector_type::is_complex()) {
            if constexpr (Op == AddSubOperation::Add)
                return [](auto &&...params) { return ::add32(params...); };
            else if constexpr (Op == AddSubOperation::Sub)
                return [](auto &&...params) { return ::sub32(params...); };
        }
        else {
            if constexpr (Op == AddSubOperation::Add)
                return [](auto &&...params) { return ::add16(params...); };
            else if constexpr (Op == AddSubOperation::Sub)
                return [](auto &&...params) { return ::sub16(params...); };
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (vector_type::is_complex()) {
            return vector_cast<T>(add_sub_bits_impl<16, int16, Elems * 2, Op>::run(vector_cast<int16>(v1),
                                                                                   vector_cast<int16>(v2)));
        }
        else {
            if constexpr (Elems <= 16) {
                vector<T, 16> ret;

                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    ret = op(v1.template grow<16>(), 0, 0x76543210, 0xfedcba98,
                                                     0, 0x76543210, 0xfedcba98);
                }
                else {
                    ret = op(v1.template grow<16>(), v2.template grow<16>());
                }

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector_type ret;
                vector<T, 16> tmp;

                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    tmp = op(v1.template extract<16>(0), 0, 0x76543210, 0xfedcba98, 0, 0x76543210, 0xfedcba98); ret.insert(0, tmp);
                    tmp = op(v1.template extract<16>(1), 0, 0x76543210, 0xfedcba98, 0, 0x76543210, 0xfedcba98); ret.insert(1, tmp);
                }
                else {
                    tmp = op(v1.template extract<16>(0), v2.template extract<16>(0)); ret.insert(0, tmp);
                    tmp = op(v1.template extract<16>(1), v2.template extract<16>(1)); ret.insert(1, tmp);
                }

                return ret;
            }
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Elems2 == 32) {
            // TODO: Investigate more efficient implementation if offset is known at compile-time
            return run(a.get(), v);
        }
        else if constexpr (vector_type::is_complex()) {
            // TODO: CRVO-2189: add16 doesn't work with cint16 vectors
            vector<int16, Elems * 2>  tmp_v = vector_cast<int16>(v);
            vector<int16, Elems2 * 2> tmp_a = vector_cast<int16>(a.parent);

            if constexpr (Elems <= 16) {
                const vector<int16, 32> ret = op(tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010,
                                                 tmp_v.template grow<std::max(Elems2, 32u)>(),            0, 0x06040200, 0x0e0c0a08, 0x3210);

                return vector_cast<T>(ret).template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector<int16, 64> ret;
                vector<int16, 32> tmp1, tmp2;

                tmp1 = op(tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010,
                                         tmp_v.template extract<32>(0),            0, 0x06040200, 0x0e0c0a08, 0x3210);
                ret.insert(0, tmp1);
                tmp2 = op(tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010,
                                         tmp_v.template extract<32>(1),            0, 0x06040200, 0x0e0c0a08, 0x3210);
                ret.insert(1, tmp2);

                return vector_cast<T>(ret);
            }
        }
        else {
            if constexpr (Elems <= 16) {
                vector<T, 16> ret;

                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    ret = op(v.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                           0, 0x76543210, 0xfedcba98);
                }
                else {
                    ret = op(a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                                       v.template grow<16>(),        0, 0x76543210, 0xfedcba98);
                }

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector_type ret;

                ret.insert(0, add_sub_bits_impl<32, T, 16, Op>::run(a, v.template extract<16>(0)));
                ret.insert(1, add_sub_bits_impl<32, T, 16, Op>::run(a, v.template extract<16>(1)));

                return ret;
            }
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems2 == 32) {
            // TODO: Investigate more efficient implementation if offset is known at compile-time
            return run(v, a.get());
        }
        else if constexpr (vector_type::is_complex()) {
            // TODO: CRVO-2189: add16 doesn't work with cint16 vectors
            vector<int16, Elems * 2>  tmp_v = vector_cast<int16>(v);
            vector<int16, Elems2 * 2> tmp_a = vector_cast<int16>(a.parent);

            if constexpr (Elems <= 16) {
                const vector<int16, 32> ret = op(tmp_v.template grow<std::max(Elems2, 32u)>(),            0, 0x06040200, 0x0e0c0a08, 0x3210,
                                                 tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010);

                return vector_cast<T>(ret).template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector<int16, Elems * 2> ret;
                vector<int16, Elems> tmp1, tmp2;

                tmp1 = op(tmp_v.template extract<32>(0),                           0, 0x06040200, 0x0e0c0a08, 0x3210,
                          tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010);
                ret.insert(0, tmp1);
                tmp2 = op(tmp_v.template extract<32>(1),                           0, 0x06040200, 0x0e0c0a08, 0x3210,
                          tmp_a.template grow<std::max(Elems2, 32u)>(), a.offset * 2, 0x00000000, 0x00000000, 0x1010);
                ret.insert(1, tmp2);

                return vector_cast<T>(ret);
            }
        }
        else {
            if constexpr (Elems <= 16) {
                vector<T, 16> ret;

                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    ret = op(v.template grow<16>(),        0, 0x76543210, 0xfedcba98,
                                                    a.offset, 0x00000000, 0x00000000);
                }
                else {
                    ret = op(v.template grow<16>(),                                  0, 0x76543210, 0xfedcba98,
                             a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000);
                }

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector_type ret;

                ret.insert(0, add_sub_bits_impl<32, T, 16, Op>::run(v.template extract<16>(0), a));
                ret.insert(1, add_sub_bits_impl<32, T, 16, Op>::run(v.template extract<16>(1), a));

                return ret;
            }
        }
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        const vector<T, 16> tmp(a);

        return run(tmp.elem_const_ref(0), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(v, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<32, float, Elems, Op>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add)
            return [](auto &&...params) { return ::fpadd(params...); };
        else if constexpr (Op == AddSubOperation::Sub)
            return [](auto &&...params) { return ::fpsub(params...); };
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 8) {
            const vector<T, 8> ret = op(v1.template grow<8>(), v2.template grow<16>(), 0, 0x76543210);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector_type ret;

            ret.template insert<8>(0, op(v1.template extract<8>(0), v2, 0, 0x76543210));
            ret.template insert<8>(1, op(v1.template extract<8>(1), v2, 8, 0x76543210));

            return ret;
        }
        else if constexpr (Elems == 32) {
            vector_type ret;

            ret.template insert<8>(0, op(v1.template extract<8>(0), v2.template extract<16>(0), 0, 0x76543210));
            ret.template insert<8>(1, op(v1.template extract<8>(1), v2.template extract<16>(0), 8, 0x76543210));
            ret.template insert<8>(2, op(v1.template extract<8>(2), v2.template extract<16>(1), 0, 0x76543210));
            ret.template insert<8>(3, op(v1.template extract<8>(3), v2.template extract<16>(1), 8, 0x76543210));

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Op == AddSubOperation::Sub) {
            vector_type v_neg;

            if constexpr (Elems == 32) {
                v_neg.template insert<8>(0, ::fpneg(v.template extract<16>(0), 0, 0x76543210));
                v_neg.template insert<8>(1, ::fpneg(v.template extract<16>(0), 8, 0x76543210));
                v_neg.template insert<8>(2, ::fpneg(v.template extract<16>(1), 0, 0x76543210));
                v_neg.template insert<8>(3, ::fpneg(v.template extract<16>(1), 8, 0x76543210));
            }
            else if constexpr (Elems == 16) {
                v_neg.template insert<8>(0, ::fpneg(v.template grow<16>(), 0, 0x76543210));
                v_neg.template insert<8>(1, ::fpneg(v.template grow<16>(), 8, 0x76543210));
            }
            else if constexpr (Elems == 8) {
                v_neg = ::fpneg(v.template grow<16>(), 0, 0x76543210);
            }
            else if constexpr (Elems == 4) {
                v_neg = ::ext_v(::fpneg(v.template grow<16>(), 0, 0x76543210), 0);
            }

            return add_sub_bits_impl<32, T, Elems, AddSubOperation::Add>::run(a, v_neg);
        }
        else {
            if constexpr (Elems <= 8) {
                const vector<T, 8> ret = op(v.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000);

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                vector<T, 16> ret;

                ret.template insert<8>(0, op(v.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
                ret.template insert<8>(1, op(v.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));

                return ret;
            }
            else if constexpr (Elems == 32) {
                vector_type ret;

                ret.template insert<8>(0, op(v.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
                ret.template insert<8>(1, op(v.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
                ret.template insert<8>(2, op(v.template extract<8>(2), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
                ret.template insert<8>(3, op(v.template extract<8>(3), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));

                return ret;
            }
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 8) {
            const vector<T, 8> ret = op(v.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector<T, 16> ret;

            ret.template insert<8>(0, op(v.template extract<8>(0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
            ret.template insert<8>(1, op(v.template extract<8>(1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));

            return ret;
        }
        else if constexpr (Elems == 32) {
            vector_type ret;

            ret.template insert<8>(0, op(v.template extract<8>(0), a.parent, a.offset, 0x00000000));
            ret.template insert<8>(1, op(v.template extract<8>(1), a.parent, a.offset, 0x00000000));
            ret.template insert<8>(2, op(v.template extract<8>(2), a.parent, a.offset, 0x00000000));
            ret.template insert<8>(3, op(v.template extract<8>(3), a.parent, a.offset, 0x00000000));

            return ret;
        }
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        const vector<T, 16> tmp(a);

        if constexpr (Op == AddSubOperation::Sub) {
            vector_type v_neg;

            if constexpr (Elems == 32) {
                v_neg.template insert<8>(0, ::fpneg(v.template extract<16>(0), 0, 0x76543210));
                v_neg.template insert<8>(1, ::fpneg(v.template extract<16>(0), 8, 0x76543210));
                v_neg.template insert<8>(2, ::fpneg(v.template extract<16>(1), 0, 0x76543210));
                v_neg.template insert<8>(3, ::fpneg(v.template extract<16>(1), 8, 0x76543210));
            }
            else if constexpr (Elems == 16) {
                v_neg.template insert<8>(0, ::fpneg(v.template grow<16>(), 0, 0x76543210));
                v_neg.template insert<8>(1, ::fpneg(v.template grow<16>(), 8, 0x76543210));
            }
            else if constexpr (Elems == 8) {
                v_neg = ::fpneg(v.template grow<16>(), 0, 0x76543210);
            }
            else if constexpr (Elems == 4) {
                v_neg = ::ext_v(::fpneg(v.template grow<16>(), 0, 0x76543210), 0);
            }

            return add_sub_bits_impl<32, T, Elems, AddSubOperation::Add>::run(tmp.elem_const_ref(0), v_neg.template extract<Elems>(0));
        }

        return run(tmp.elem_const_ref(0), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        const vector<T, 16> tmp(a);

        return run(v, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<64, cint32, Elems, Op>
{
    using           T = cint32;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add)
            return [](auto &&...params) { return ::add8(params...); };
        else if constexpr (Op == AddSubOperation::Sub)
            return [](auto &&...params) { return ::sub8(params...); };
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 8) {
            vector<T, 8> ret;

            if (chess_manifest(utils::is_same_object(v1, v2)))
                ret = op(v1.template grow<8>(), 0, 0x76543210, 0, 0x76543210);
            else
                ret = op(v1.template grow<8>(), 0, 0x76543210, v2.template grow<8>(), 0, 0x76543210);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector_type ret;
            vector<T, 8> tmp;

            if (chess_manifest(utils::is_same_object(v1, v2))) {
                tmp = op(v1.template extract<8>(0), 0, 0x76543210, 0, 0x76543210); ret.insert(0, tmp);
                tmp = op(v1.template extract<8>(1), 0, 0x76543210, 0, 0x76543210); ret.insert(1, tmp);
            }
            else {
                tmp = op(v1.template extract<8>(0), 0, 0x76543210, v2.template extract<8>(0), 0, 0x76543210); ret.insert(0, tmp);
                tmp = op(v1.template extract<8>(1), 0, 0x76543210, v2.template extract<8>(1), 0, 0x76543210); ret.insert(1, tmp);
            }

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Elems2 == 16) {
            // TODO: Investigate more efficient implementation if offset is known at compile-time
            return run(a.get(), v);
        }
        else {
            if constexpr (Elems <= 8) {
                vector<T, 8> ret;

                if (chess_manifest(utils::is_same_object(a.parent, v)))
                    ret = op(v.template grow<8>(), a.offset, 0x00000000, 0, 0x76543210);
                else
                    ret = op(a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x00000000, v.template grow<8>(), 0, 0x76543210);

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                vector_type ret;

                ret.insert(0, add_sub_bits_impl<64, T, 8, Op>::run(a, v.template extract<8>(0)));
                ret.insert(1, add_sub_bits_impl<64, T, 8, Op>::run(a, v.template extract<8>(1)));

                return ret;
            }
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems2 == 16) {
            // TODO: Investigate more efficient implementation if offset is known at compile-time
            return run(v, a.get());
        }
        else {
            if constexpr (Elems <= 8) {
                vector<T, 8> ret;

                if (chess_manifest(utils::is_same_object(a.parent, v)))
                    ret = op(v.template grow<8>(), 0, 0x76543210, a.offset, 0x00000000);
                else
                    ret = op(v.template grow<8>(), 0, 0x76543210, a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x00000000);

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                vector_type ret;

                ret.insert(0, add_sub_bits_impl<64, T, 8, Op>::run(v.template extract<8>(0), a));
                ret.insert(1, add_sub_bits_impl<64, T, 8, Op>::run(v.template extract<8>(1), a));

                return ret;
            }
        }
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        const vector<T, 8> tmp(a);

        return run(tmp.elem_const_ref(0), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        const vector<T, 8> tmp(a);

        return run(v, tmp.elem_const_ref(0));
    }
};

template <unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<64, cfloat, Elems, Op>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == AddSubOperation::Add)
            return [](auto &&...params) { return ::fpadd(params...); };
        else if constexpr (Op == AddSubOperation::Sub)
            return [](auto &&...params) { return ::fpsub(params...); };
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<T, 4> ret = op(v1.template grow<4>(), v2.template grow<8>(), 0, 0x3210);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            vector_type ret;

            ret.template insert<4>(0, op(v1.template extract<4>(0), v2, 0, 0x3210));
            ret.template insert<4>(1, op(v1.template extract<4>(1), v2, 4, 0x3210));

            return ret;
        }
        else if constexpr (Elems == 16) {
            vector_type ret;

            ret.template insert<4>(0, op(v1.template extract<4>(0), v2.template extract<8>(0), 0, 0x3210));
            ret.template insert<4>(1, op(v1.template extract<4>(1), v2.template extract<8>(0), 4, 0x3210));
            ret.template insert<4>(2, op(v1.template extract<4>(2), v2.template extract<8>(1), 0, 0x3210));
            ret.template insert<4>(3, op(v1.template extract<4>(3), v2.template extract<8>(1), 4, 0x3210));

            return ret;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        if constexpr (Op == AddSubOperation::Sub) {
            vector_type v_neg;

            if constexpr (Elems == 16) {
                v_neg.template insert<4>(0, ::fpneg(v.template extract<8>(0), 0, 0x3210));
                v_neg.template insert<4>(1, ::fpneg(v.template extract<8>(0), 4, 0x3210));
                v_neg.template insert<4>(2, ::fpneg(v.template extract<8>(1), 0, 0x3210));
                v_neg.template insert<4>(3, ::fpneg(v.template extract<8>(1), 4, 0x3210));
            }
            else if constexpr (Elems == 8) {
                v_neg.template insert<4>(0, ::fpneg(v, 0, 0x3210));
                v_neg.template insert<4>(1, ::fpneg(v, 4, 0x3210));
            }
            else if constexpr (Elems == 4) {
                v_neg = ::fpneg(v.template grow<8>(), 0, 0x3210);
            }
            else if constexpr (Elems == 2) {
                v_neg = ::ext_v(::fpneg(v.template grow<8>(), 0, 0x3210), 0);
            }

            return add_sub_bits_impl<64, T, Elems, AddSubOperation::Add>::run(a, v_neg.template extract<Elems>(0));
        }
        else {
            if constexpr (Elems <= 4) {
                const vector<T, 4> ret = op(v.template grow<4>(), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000);

                return ret.template extract<Elems>(0);
            }
            else if constexpr (Elems == 8) {
                vector_type ret;

                ret.template insert<4>(0, op(v.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
                ret.template insert<4>(1, op(v.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));

                return ret;
            }
            else if constexpr (Elems == 16) {
                vector_type ret;

                ret.template insert<4>(0, ::fpadd(v.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
                ret.template insert<4>(1, ::fpadd(v.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
                ret.template insert<4>(2, ::fpadd(v.template extract<4>(2), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
                ret.template insert<4>(3, ::fpadd(v.template extract<4>(3), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));

                return ret;
            }
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<T, 4> ret = op(v.template grow<4>(), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000);

            return ret.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            vector_type ret;

            ret.template insert<4>(0, op(v.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
            ret.template insert<4>(1, op(v.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));

            return ret;
        }
        else if constexpr (Elems == 16) {
            vector_type ret;

            ret.template insert<4>(0, op(v.template extract<4>(0), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
            ret.template insert<4>(1, op(v.template extract<4>(1), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
            ret.template insert<4>(2, op(v.template extract<4>(2), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));
            ret.template insert<4>(3, op(v.template extract<4>(3), a.parent.template grow<std::max(Elems2, 8u)>(), a.offset, 0x0000));

            return ret;
        }
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        const vector<T, 8> tmp(a);

        if constexpr (Op == AddSubOperation::Sub) {
            vector_type v_neg;

            if constexpr (Elems == 16) {
                v_neg.template insert<4>(0, ::fpneg(v.template extract<8>(0), 0, 0x3210));
                v_neg.template insert<4>(1, ::fpneg(v.template extract<8>(0), 4, 0x3210));
                v_neg.template insert<4>(2, ::fpneg(v.template extract<8>(1), 0, 0x3210));
                v_neg.template insert<4>(3, ::fpneg(v.template extract<8>(1), 4, 0x3210));
            }
            else if constexpr (Elems == 8) {
                v_neg.template insert<4>(0, ::fpneg(v.template grow<8>(), 0, 0x3210));
                v_neg.template insert<4>(1, ::fpneg(v.template grow<8>(), 4, 0x3210));
            }
            else if constexpr (Elems == 4) {
                v_neg = ::fpneg(v.template grow<8>(), 0, 0x3210);
            }
            else if constexpr (Elems == 2) {
                v_neg = ::ext_v(::fpneg(v.template grow<8>(), 0, 0x3210), 0);
            }

            return add_sub_bits_impl<64, T, Elems, AddSubOperation::Add>::run(tmp.elem_const_ref(0), v_neg.template grow<Elems>());
        }

        return run(tmp.elem_const_ref(0), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        const vector<T, 8> tmp(a);

        return run(v, tmp.elem_const_ref(0));
    }
};

}

#endif
