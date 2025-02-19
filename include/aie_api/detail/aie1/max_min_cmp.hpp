// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MAX_MIN_CMP__HPP__
#define __AIE_API_DETAIL_AIE1_MAX_MIN_CMP__HPP__

#include "../vector.hpp"
#include "../compare.hpp"

namespace aie::detail {

// TODO: Missing vector_elem_ref overloads

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<8, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static auto run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1_1 = v1.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp1_2 = v1.template extract<Elems / 2>(1);

            const vector<T, Elems / 2> tmp2_1 = v2.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_2 = v2.template extract<Elems / 2>(1);

            vector_type ret;

            const auto [tmp1, m1] = max_min_cmp_bits_impl<8, T, Elems / 2, Op>::run(tmp1_1, tmp2_1);
            ret.insert(0, tmp1);
            const auto [tmp2, m2] = max_min_cmp_bits_impl<8, T, Elems / 2, Op>::run(tmp1_2, tmp2_2);
            ret.insert(1, tmp2);

            const auto m = mask<Elems>::from_masks(m1, m2);

            return std::make_tuple(ret, m);
        }
        else {
            const auto [tmp, m] = max_min_cmp_bits_impl<16, int16, Elems, Op>::run(v1.unpack(), v2.unpack());

            return std::make_tuple(tmp.template pack<T>(), m);
        }
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<16, int16, Elems, Op>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == MaxMinOperation::MaxCmp)
            return [](auto &&... args) __aie_inline { return ::maxcmp32(args...); };
        else if constexpr (Op == MaxMinOperation::MinCmp)
            return [](auto &&... args) __aie_inline { return ::mincmp32(args...); };
    }

    static auto run(const vector_type &a, const vector_type &b)
    {
        constexpr auto op = get_op();

        vector_type ret;

        if constexpr (Elems <= 16) {
            unsigned cmp;
            vector<int16, 32> tmp;

            tmp = op(a.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                     b.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210, cmp);
            ret = tmp.template extract<Elems>(0);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else if constexpr (Elems == 32) {
            unsigned cmp;

            ret = op(a.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                     b.template grow<32>(), 0, 0x06040200, 0x0E0C0A08, 0x3210, cmp);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp1, tmp2;
            unsigned cmp1, cmp2;

            tmp1 = op(a.template extract<32>(0), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                      b.template extract<32>(0), 0, 0x06040200, 0x0E0C0A08, 0x3210, cmp1);
            ret.insert(0, tmp1);
            tmp2 = op(a.template extract<32>(1), 0, 0x06040200, 0x0E0C0A08, 0x3210,
                      b.template extract<32>(1), 0, 0x06040200, 0x0E0C0A08, 0x3210, cmp2);
            ret.insert(1, tmp2);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp1, cmp2));
        }
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<32, int32, Elems, Op>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == MaxMinOperation::MaxCmp)
            return [](auto &&... args) __aie_inline { return ::maxcmp16(args...); };
        else if constexpr (Op == MaxMinOperation::MinCmp)
            return [](auto &&... args) __aie_inline { return ::mincmp16(args...); };
    }

    static auto run(const vector_type &a, const vector_type &b)
    {
        constexpr auto op = get_op();

        vector_type ret;

        if constexpr (Elems <= 8) {
            unsigned cmp;
            vector<int32, 16> tmp;

            tmp = op(a.template grow<16>(), 0, 0x76543210, 0xfedcba98,
                     b.template grow<16>(), 0, 0x76543210, 0xfedcba98, cmp);
            ret = tmp.template extract<Elems>(0);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else if constexpr (Elems == 16) {
            unsigned cmp;

            ret = op(a.template grow<16>(), 0, 0x76543210, 0xfedcba98,
                     b.template grow<16>(), 0, 0x76543210, 0xfedcba98, cmp);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;
            unsigned cmp1, cmp2;

            tmp1 = op(a.template extract<16>(0), 0, 0x76543210, 0xfedcba98,
                      b.template extract<16>(0), 0, 0x76543210, 0xfedcba98, cmp1);
            ret.insert(0, tmp1);
            tmp2 = op(a.template extract<16>(1), 0, 0x76543210, 0xfedcba98,
                      b.template extract<16>(1), 0, 0x76543210, 0xfedcba98, cmp2);
            ret.insert(1, tmp2);

            return std::make_tuple(ret, mask<Elems>::from_uint32((cmp2 << 16) | cmp1));
        }
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<32, float, Elems, Op>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static auto run(const vector_type &a, const vector_type &b)
    {
        vector_type ret;

        const mask<Elems> m = (Op == MaxMinOperation::MaxCmp)? lt<float, Elems>::run(a, b) :
                                                               lt<float, Elems>::run(b, a);

        if constexpr (Elems <= 8) {
            vector<float, 16> tmp;

            tmp = ::fpselect16(m.to_uint32(0), a.template grow<16>(), 0, 0x76543210, 0x0,
                                               b.template grow<16>(), 0, 0x76543210, 0x0);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::fpselect16(m.to_uint32(0), a, 0, 0x76543210, 0xfedcba98,
                                               b, 0, 0x76543210, 0xfedcba98);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2;

            tmp1 = ::fpselect16(m.to_uint32(0),   a.template extract<16>(0), 0, 0x76543210, 0xfedcba98,
                                                  b.template extract<16>(0), 0, 0x76543210, 0xfedcba98);
            ret.insert(0, tmp1);
            tmp2 = ::fpselect16(m.to_uint32(0) >> 16, a.template extract<16>(1), 0, 0x76543210, 0xfedcba98,
                                                      b.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            ret.insert(1, tmp2);
        }

        return std::make_tuple(ret, m);
    }
};

}

#endif
