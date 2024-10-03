// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_BLEND__HPP__
#define __AIE_API_DETAIL_AIE1_BLEND__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct select_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            const vector<T, Elems / 4> tmp1_1 = v1.template extract<Elems / 4>(0);
            const vector<T, Elems / 4> tmp2_1 = v2.template extract<Elems / 4>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 4>::run(tmp1_1, tmp2_1, m.template get_submask<Elems / 4>(0)));

            const vector<T, Elems / 4> tmp1_2 = v1.template extract<Elems / 4>(1);
            const vector<T, Elems / 4> tmp2_2 = v2.template extract<Elems / 4>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 4>::run(tmp1_2, tmp2_2, m.template get_submask<Elems / 4>(1)));

            const vector<T, Elems / 4> tmp1_3 = v1.template extract<Elems / 4>(2);
            const vector<T, Elems / 4> tmp2_3 = v2.template extract<Elems / 4>(2);
            ret.insert(2, select_bits_impl<8, T, Elems / 4>::run(tmp1_3, tmp2_3, m.template get_submask<Elems / 4>(2)));

            const vector<T, Elems / 4> tmp1_4 = v1.template extract<Elems / 4>(3);
            const vector<T, Elems / 4> tmp2_4 = v2.template extract<Elems / 4>(3);
            ret.insert(3, select_bits_impl<8, T, Elems / 4>::run(tmp1_4, tmp2_4, m.template get_submask<Elems / 4>(3)));

            return ret;
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            const vector<T, Elems / 2> tmp1_1 = v1.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_1 = v2.template extract<Elems / 2>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 2>::run(tmp1_1, tmp2_1, m.template get_submask<Elems / 2>(0)));

            const vector<T, Elems / 2> tmp1_2 = v1.template extract<Elems / 2>(1);
            const vector<T, Elems / 2> tmp2_2 = v2.template extract<Elems / 2>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 2>::run(tmp1_2, tmp2_2, m.template get_submask<Elems / 2>(1)));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp1 = v1.unpack();
            const vector<int16, Elems> tmp2 = v2.unpack();

            return select_bits_impl<16, int16, Elems>::run(tmp1, tmp2, m).template pack<T>();
        }
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            const vector<T, Elems / 4> tmp_1 = v.template extract<Elems / 4>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 4>::run(a, tmp_1, m.template get_submask<Elems / 4>(0)));

            const vector<T, Elems / 4> tmp_2 = v.template extract<Elems / 4>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 4>::run(a, tmp_2, m.template get_submask<Elems / 4>(1)));

            const vector<T, Elems / 4> tmp_3 = v.template extract<Elems / 4>(2);
            ret.insert(2, select_bits_impl<8, T, Elems / 4>::run(a, tmp_3, m.template get_submask<Elems / 4>(2)));

            const vector<T, Elems / 4> tmp_4 = v.template extract<Elems / 4>(3);
            ret.insert(3, select_bits_impl<8, T, Elems / 4>::run(a, tmp_4, m.template get_submask<Elems / 4>(3)));

            return ret;
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 2>::run(a, tmp_1, m.template get_submask<Elems / 2>(0)));

            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 2>::run(a, tmp_2, m.template get_submask<Elems / 2>(1)));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return select_bits_impl<16, int16, Elems>::run((int16)a, tmp, m).template pack<T>();
        }
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            const vector<T, Elems / 4> tmp_1 = v.template extract<Elems / 4>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 4>::run(tmp_1, a, m.template get_submask<Elems / 4>(0)));

            const vector<T, Elems / 4> tmp_2 = v.template extract<Elems / 4>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 4>::run(tmp_2, a, m.template get_submask<Elems / 4>(1)));

            const vector<T, Elems / 4> tmp_3 = v.template extract<Elems / 4>(2);
            ret.insert(2, select_bits_impl<8, T, Elems / 4>::run(tmp_3, a, m.template get_submask<Elems / 4>(2)));

            const vector<T, Elems / 4> tmp_4 = v.template extract<Elems / 4>(3);
            ret.insert(3, select_bits_impl<8, T, Elems / 4>::run(tmp_4, a, m.template get_submask<Elems / 4>(3)));

            return ret;
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            ret.insert(0, select_bits_impl<8, T, Elems / 2>::run(tmp_1, a, m.template get_submask<Elems / 2>(0)));

            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);
            ret.insert(1, select_bits_impl<8, T, Elems / 2>::run(tmp_2, a, m.template get_submask<Elems / 2>(1)));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return select_bits_impl<16, int16, Elems>::run(tmp, (int16)a, m).template pack<T>();
        }
    }
    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            ret.insert(0, select_bits_impl<8, T, Elems / 4>::run(a, b, m.template get_submask<Elems / 4>(0)));
            ret.insert(1, select_bits_impl<8, T, Elems / 4>::run(a, b, m.template get_submask<Elems / 4>(1)));
            ret.insert(2, select_bits_impl<8, T, Elems / 4>::run(a, b, m.template get_submask<Elems / 4>(2)));
            ret.insert(3, select_bits_impl<8, T, Elems / 4>::run(a, b, m.template get_submask<Elems / 4>(3)));

            return ret;
        }
        else if constexpr (Elems == 64) {
            vector_type ret;

            ret.insert(0, select_bits_impl<8, T, Elems / 2>::run(a, b, m.template get_submask<Elems / 2>(0)));
            ret.insert(1, select_bits_impl<8, T, Elems / 2>::run(a, b, m.template get_submask<Elems / 2>(1)));

            return ret;
        }
        else {
            return select_bits_impl<16, int16, Elems>::run((int16)a, (int16)b, m).template pack<T>();
        }
    }
};

template <typename T, unsigned Elems>
struct select_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 16) {
            vector<T, 32> tmp;

            tmp = ::select32(m.to_uint32(0), v1.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                                             v2.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = ::select32(m.to_uint32(0), v1, 0, 0x06040200, 0x0E0C0A08, 0x3210,
                                             v2, 0, 0x06040200, 0x0E0C0A08, 0x3210);
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp1, tmp2;

            tmp1 = ::select32(m.to_uint32(0), v1.template extract<32>(0), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                                              v2.template extract<32>(0), 0, 0x06040200, 0x0E0C0A08, 0x3210);
            ret.insert(0, tmp1);
            tmp2 = ::select32(m.to_uint32(1), v1.template extract<32>(1), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                                              v2.template extract<32>(1), 0, 0x06040200, 0x0E0C0A08, 0x3210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        const unsigned odd = a.offset & 0x1;
        const int offset = a.offset - odd;
        const int square = 0x1111 * odd;

        vector_type ret;

        if constexpr (Elems <= 16) {
            vector<T, 32> tmp;

            tmp = ::select32(m.to_uint32(0), a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square,
                                             v.template grow<32>(),             0, 0x06040200, 0x0E0C0A08, 0x3210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = ::select32(m.to_uint32(0), concat(a.parent.template grow<32>(), v), offset, 0x00000000, 0x00000000, square,
                                                                                          32, 0x06040200, 0x0E0C0A08, 0x3210);
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp1, tmp2;

            tmp1 = ::select32(m.to_uint32(0), a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square,
                                              v.template extract<32>(0),         0, 0x06040200, 0x0E0C0A08, 0x3210);
            ret.insert(0, tmp1);
            tmp2 = ::select32(m.to_uint32(1), a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square,
                                              v.template extract<32>(1),         0, 0x06040200, 0x0E0C0A08, 0x3210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        const unsigned odd = a.offset & 0x1;
        const int offset = a.offset - odd;
        const int square = 0x1111 * odd;

        vector_type ret;

        if constexpr (Elems <= 16) {
            vector<T, 32> tmp;

            tmp = ::select32(m.to_uint32(0), v.template grow<32>(),             0, 0x06040200, 0x0E0C0A08, 0x3210,
                                             a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = ::select32(m.to_uint32(0), concat(a.parent.template grow<32>(), v),     32, 0x06040200, 0x0E0C0A08, 0x3210,
                                                                                      offset, 0x00000000, 0x00000000, square);
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp1, tmp2;

            tmp1 = ::select32(m.to_uint32(0), v.template extract<32>(0),         0, 0x06040200, 0x0E0C0A08, 0x3210,
                                              a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square);
            ret.insert(0, tmp1);
            tmp2 = ::select32(m.to_uint32(1), v.template extract<32>(1),         0, 0x06040200, 0x0E0C0A08, 0x3210,
                                              a.parent.template grow<32>(), offset, 0x00000000, 0x00000000, square);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        const unsigned odd_a = a.offset & 0x1;
        const int offset_a = a.offset - odd_a;
        const int square_a = 0x1111 * odd_a;

        const unsigned odd_b = b.offset & 0x1;
        const int offset_b = b.offset - odd_b;
        const int square_b = 0x1111 * odd_b;

        vector_type ret;

        static_assert(Elems != 64);

        if constexpr (Elems <= 16) {
            vector<T, 32> tmp;

            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                tmp = ::select32(m.to_uint32(0), b.parent.template grow<32>(), offset_a, 0x00000000, 0x00000000, square_a,
                                                                               offset_b, 0x00000000, 0x00000000, square_b);
            }
            else {
                tmp = ::select32(m.to_uint32(0), a.parent.template grow<32>(), offset_a, 0x00000000, 0x00000000, square_a,
                                                 b.parent.template grow<32>(), offset_b, 0x00000000, 0x00000000, square_b);
            }

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                ret = ::select32(m.to_uint32(0), b.parent, offset_a, 0x00000000, 0x00000000, square_a,
                                                           offset_b, 0x00000000, 0x00000000, square_b);
            }
            else {
                ret = ::select32(m.to_uint32(0), concat(a.parent.template grow<32>(), b.parent),      offset_a, 0x00000000, 0x00000000, square_a,
                                                                                                 32 + offset_b, 0x00000000, 0x00000000, square_b);
            }
        }

        return ret;
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        const vector<T, 32> tmp(a);

        return run(tmp[0], v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        const vector<T, 32> tmp(a);

        return run(v, tmp[0], m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        const vector<T, Elems> tmp(a, b);

        return run(tmp[0], tmp[1], m);
    }
};

template <typename T, unsigned Elems>
struct select_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::select16(m.to_uint32(0), v1.template grow<16>(), 0, 0x76543210, 0x0,
                                             v2.template grow<16>(), 0, 0x76543210, 0x0);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::select16(m.to_uint32(0), v1, 0, 0x76543210, 0xfedcba98,
                                             v2, 0, 0x76543210, 0xfedcba98);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::select16(m.to_uint32(0),       v1.template extract<16>(0), 0, 0x76543210, 0xfedcba98,
                                                    v2.template extract<16>(0), 0, 0x76543210, 0xfedcba98);
            ret.insert(0, tmp1);
            tmp2 = ::select16(m.to_uint32(0) >> 16, v1.template extract<16>(1), 0, 0x76543210, 0xfedcba98,
                                                    v2.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::select16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                             v.template grow<16>(),               0, 0x76543210, 0xfedcba98);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::select16(m.to_uint32(0), concat(a.parent.template grow<16>(), v), a.offset, 0x00000000, 0x00000000,
                                                                                            16, 0x76543210, 0xfedcba98);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::select16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                              v.template extract<16>(0),           0, 0x76543210, 0xfedcba98);
            ret.insert(0, tmp1);
            tmp2 = ::select16(m.to_uint32(0) >> 16, a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                    v.template extract<16>(1),           0, 0x76543210, 0xfedcba98);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::select16(m.to_uint32(0), v.template grow<16>(),               0, 0x76543210, 0xfedcba98,
                                             a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::select16(m.to_uint32(0), concat(a.parent.template grow<16>(), v),       16, 0x76543210, 0xfedcba98,
                                                                                      a.offset, 0x00000000, 0x00000000);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::select16(m.to_uint32(0), v.template extract<16>(0),           0, 0x76543210, 0xfedcba98,
                                              a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);
            ret.insert(0, tmp1);
            tmp2 = ::select16(m.to_uint32(0) >> 16, v.template extract<16>(1),           0, 0x76543210, 0xfedcba98,
                                                    a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        vector_type ret;

        static_assert(Elems != 32);

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                tmp = ::select16(m.to_uint32(0), b.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                                               b.offset, 0x00000000, 0x00000000);
            }
            else {
                tmp = ::select16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                 b.parent.template grow<16>(), b.offset, 0x00000000, 0x00000000);
            }

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                ret = ::select16(m.to_uint32(0), b.parent, a.offset, 0x00000000, 0x00000000,
                                                           b.offset, 0x00000000, 0x00000000);
            }
            else {
                ret = ::select16(m.to_uint32(0), concat(a.parent.template grow<16>(), b.parent),      a.offset, 0x00000000, 0x00000000,
                                                                                                 16 + b.offset, 0x00000000, 0x00000000);
            }
        }

        return ret;
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        const vector<T, 16> tmp(a);

        return run(tmp[0], v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        const vector<T, 16> tmp(a);

        return run(v, tmp[0], m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        const vector<T, Elems> tmp(a, b);

        return run(tmp[0], tmp[1], m);
    }
};

template <unsigned Elems>
struct select_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::fpselect16(m.to_uint32(0), v1.template grow<16>(), 0, 0x76543210, 0x0,
                                               v2.template grow<16>(), 0, 0x76543210, 0x0);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::fpselect16(m.to_uint32(0), v1, 0, 0x76543210, 0xfedcba98,
                                               v2, 0, 0x76543210, 0xfedcba98);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::fpselect16(m.to_uint32(0),       v1.template extract<16>(0), 0, 0x76543210, 0xfedcba98,
                                                      v2.template extract<16>(0), 0, 0x76543210, 0xfedcba98);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect16(m.to_uint32(0) >> 16, v1.template extract<16>(1), 0, 0x76543210, 0xfedcba98,
                                                      v2.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::fpselect16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                               v.template grow<16>(),               0, 0x76543210, 0xfedcba98);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::fpselect16(m.to_uint32(0), concat(a.parent.template grow<16>(), v), a.offset, 0x00000000, 0x00000000,
                                                                                              16, 0x76543210, 0xfedcba98);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::fpselect16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                v.template extract<16>(0),           0, 0x76543210, 0xfedcba98);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect16(m.to_uint32(0) >> 16, a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                      v.template extract<16>(1),           0, 0x76543210, 0xfedcba98);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            tmp = ::fpselect16(m.to_uint32(0), v.template grow<16>(),               0, 0x76543210, 0xfedcba98,
                                               a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::fpselect16(m.to_uint32(0), concat(a.parent.template grow<16>(), v),       16, 0x76543210, 0xfedcba98,
                                                                                        a.offset, 0x00000000, 0x00000000);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::fpselect16(m.to_uint32(0), v.template extract<16>(0),           0, 0x76543210, 0xfedcba98,
                                                a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect16(m.to_uint32(0) >> 16, v.template extract<16>(1),           0, 0x76543210, 0xfedcba98,
                                                      a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        vector_type ret;

        static_assert(Elems != 32);

        if constexpr (Elems <= 8) {
            vector<T, 16> tmp;

            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                tmp = ::fpselect16(m.to_uint32(0), b.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                                                 b.offset, 0x00000000, 0x00000000);
            }
            else {
                tmp = ::fpselect16(m.to_uint32(0), a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000,
                                                   b.parent.template grow<16>(), b.offset, 0x00000000, 0x00000000);
            }

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                ret = ::fpselect16(m.to_uint32(0), b.parent, a.offset, 0x00000000, 0x00000000,
                                                             b.offset, 0x00000000, 0x00000000);
            }
            else {
                ret = ::fpselect16(m.to_uint32(0), concat(a.parent.template grow<16>(), b.parent),      a.offset, 0x00000000, 0x00000000,
                                                                                                   16 + b.offset, 0x00000000, 0x00000000);
            }
        }

        return ret;
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        const vector<T, 16> tmp(a);

        return run(tmp[0], v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        const vector<T, 16> tmp(a);

        return run(v, tmp[0], m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        const vector<T, Elems> tmp(a, b);

        return run(tmp[0], tmp[1], m);
    }
};

template <typename T, unsigned Elems>
struct select_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::select8(m.to_uint32(0), v1.template grow<8>(), 0, 0x76543210,
                                            v2.template grow<8>(), 0, 0x76543210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::select8(m.to_uint32(0), v1, 0, 0x76543210, v2, 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::select8(m.to_uint32(0),      v1.template extract<8>(0), 0, 0x76543210,
                                                  v2.template extract<8>(0), 0, 0x76543210);
            ret.insert(0, tmp1);
            tmp2 = ::select8(m.to_uint32(0) >> 8, v1.template extract<8>(1), 0, 0x76543210,
                                                  v2.template extract<8>(1), 0, 0x76543210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::select8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                            v.template grow<8>(),               0, 0x76543210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::select8(m.to_uint32(0), concat(a.parent.template grow<8>(), v), a.offset, 0x00000000,
                                                                                           8, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::select8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                             v.template extract<8>(0),           0, 0x76543210);
            ret.insert(0, tmp1);
            tmp2 = ::select8(m.to_uint32(0) >> 8, a.parent.template grow<8>(), a.offset, 0x00000000,
                                                  v.template extract<8>(1),           0, 0x76543210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::select8(m.to_uint32(0), v.template grow<8>(),               0, 0x76543210,
                                            a.parent.template grow<8>(), a.offset, 0x00000000);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::select8(m.to_uint32(0), concat(a.parent.template grow<8>(), v),        8, 0x76543210,
                                                                                    a.offset, 0x00000000);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::select8(m.to_uint32(0), v.template extract<8>(0),           0, 0x76543210,
                                             a.parent.template grow<8>(), a.offset, 0x00000000);
            ret.insert(0, tmp1);
            tmp2 = ::select8(m.to_uint32(0) >> 8, v.template extract<8>(1),           0, 0x76543210,
                                                  a.parent.template grow<8>(), a.offset, 0x00000000);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        vector_type ret;

        static_assert(Elems != 16);

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                tmp = ::select8(m.to_uint32(0), b.parent.template grow<8>(), a.offset, 0x00000000,
                                                                             b.offset, 0x00000000);
            }
            else {
                tmp = ::select8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                                b.parent.template grow<8>(), b.offset, 0x00000000);
            }

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                ret = ::select8(m.to_uint32(0), b.parent, a.offset, 0x00000000,
                                                          b.offset, 0x00000000);
            }
            else {
                ret = ::select8(m.to_uint32(0), concat(a.parent.template grow<8>(), b.parent),     a.offset, 0x00000000,
                                                                                               8 + b.offset, 0x00000000);
            }
        }

        return ret;
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        const vector<T, 8> tmp(a);

        return run(tmp[0], v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        const vector<T, 8> tmp(a);

        return run(v, tmp[0], m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        const vector<T, Elems> tmp(a, b);

        return run(tmp[0], tmp[1], m);
    }
};

template <unsigned Elems>
struct select_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::fpselect8(m.to_uint32(0), v1.template grow<8>(), 0, 0x76543210,
                                              v2.template grow<8>(), 0, 0x76543210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::fpselect8(m.to_uint32(0), v1, 0, 0x76543210, v2, 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::fpselect8(m.to_uint32(0),      v1.template extract<8>(0), 0, 0x76543210,
                                                    v2.template extract<8>(0), 0, 0x76543210);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect8(m.to_uint32(0) >> 8, v1.template extract<8>(1), 0, 0x76543210,
                                                    v2.template extract<8>(1), 0, 0x76543210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::fpselect8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                              v.template grow<8>(),               0, 0x76543210);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::fpselect8(m.to_uint32(0), concat(a.parent.template grow<8>(), v), a.offset, 0x00000000,
                                                                                             8, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::fpselect8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                               v.template extract<8>(0),           0, 0x76543210);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect8(m.to_uint32(0) >> 8, a.parent.template grow<8>(), a.offset, 0x00000000,
                                                    v.template extract<8>(1),          0, 0x76543210);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        vector_type ret;

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            tmp = ::fpselect8(m.to_uint32(0), v.template grow<8>(),               0, 0x76543210,
                                              a.parent.template grow<8>(), a.offset, 0x00000000);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::fpselect8(m.to_uint32(0), concat(a.parent.template grow<8>(), v),        8, 0x76543210,
                                                                                      a.offset, 0x00000000);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2;

            tmp1 = ::fpselect8(m.to_uint32(0), v.template extract<8>(0),           0, 0x76543210,
                                               a.parent.template grow<8>(), a.offset, 0x00000000);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect8(m.to_uint32(0) >> 8, v.template extract<8>(1),           0, 0x76543210,
                                                    a.parent.template grow<8>(), a.offset, 0x00000000);
            ret.insert(1, tmp2);
        }

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, vector_elem_const_ref<T, Elems> b, const mask<Elems> &m)
    {
        vector_type ret;

        static_assert(Elems != 16);

        if constexpr (Elems <= 4) {
            vector<T, 8> tmp;

            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                tmp = ::fpselect8(m.to_uint32(0), b.parent.template grow<8>(), a.offset, 0x00000000,
                                                                               b.offset, 0x00000000);
            }
            else {
                tmp = ::fpselect8(m.to_uint32(0), a.parent.template grow<8>(), a.offset, 0x00000000,
                                                  b.parent.template grow<8>(), b.offset, 0x00000000);
            }

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            if (chess_manifest(utils::is_same_object(a.parent, b.parent))) {
                ret = ::fpselect8(m.to_uint32(0), b.parent, a.offset, 0x00000000,
                                                             b.offset, 0x00000000);
            }
            else {
                ret = ::fpselect8(m.to_uint32(0), concat(a.parent.template grow<8>(), b.parent),     a.offset, 0x00000000,
                                                                                                 8 + b.offset, 0x00000000);
            }
        }

        return ret;
    }

    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        const vector<T, 8> tmp(a);

        return run(tmp[0], v, m);
    }

    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        const vector<T, 8> tmp(a);

        return run(v, tmp[0], m);
    }

    static vector_type run(const T &a, const T &b, const mask<Elems> &m)
    {
        const vector<T, Elems> tmp(a, b);

        return run(tmp[0], tmp[1], m);
    }
};

}

#endif
