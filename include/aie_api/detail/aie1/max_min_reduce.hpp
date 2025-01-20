// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MAX_MIN_REDUCE_HPP__
#define __AIE_API_DETAIL_AIE1_MAX_MIN_REDUCE_HPP__

#include "../max_min.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<8, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static T run(const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const T res1 = max_min_reduce_bits<16, int16, Elems / 2, Op>::run(v.template extract<Elems / 2>(0).unpack());
            const T res2 = max_min_reduce_bits<16, int16, Elems / 2, Op>::run(v.template extract<Elems / 2>(1).unpack());

            if constexpr (Op == MaxMinOperation::Max)
                return (T) std::max(res1, res2);
            else
                return (T) std::min(res1, res2);
        }
        else {
            return (T) max_min_reduce_bits<16, int16, Elems, Op>::run(v.unpack());
        }
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<16, int16, Elems, Op>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == MaxMinOperation::Max)
            return [](auto &&... args) __aie_inline { return ::max32(args...); };
        else
            return [](auto &&... args) __aie_inline { return ::min32(args...); };
    }

    static T run(const vector_type &v)
    {
        constexpr auto op = get_op();

        vector<T, 32> v2;

        if constexpr (Elems == 64) {
            v2 = op(v.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210, v.template extract<32>(1),  0, 0x06040200, 0x0e0c0a08, 0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 32) {
            v2 = op(                        v, 0, 0x06040200, 0x0,        0x3210,                            16, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 16) {
            v2 = op(    v.template grow<32>(), 0, 0x06040200, 0x0,        0x3210,                             8, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             4, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             2, 0x06040200, 0x0,        0x3210);
            v2 = op(                       v2, 0, 0x06040200, 0x0,        0x3210,                             0, 0x06040200, 0x0,        0x1);
        }
        else if constexpr (Elems == 8) {
            v2 = op(    v.template grow<32>(), 0, 0x0,        0x0,        0x3210,                             4, 0x0,        0x0,        0x3210);
            v2 = op(                       v2, 0, 0x0,        0x0,        0x3210,                             2, 0x0,        0x0,        0x3210);
            v2 = op(                       v2, 0, 0x0,        0x0,        0x3210,                             0, 0x0,        0x0,        0x1);
        }

        return v2[0];
    }
};


template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<32, int32, Elems, Op>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == MaxMinOperation::Max)
            return [](auto &&... args) __aie_inline { return ::max16(args...); };
        else
            return [](auto &&... args) __aie_inline { return ::min16(args...); };
    }

    static T run(const vector_type &v)
    {
        constexpr auto op = get_op();

        vector<T, 16> v2;

        if constexpr (Elems == 32) {
            v2 = op(v.template extract<16>(0), 0, 0x76543210, 0xfedcba98, v.template extract<16>(1), 0, 0x76543210, 0xfedcba98);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 16) {
            v2 = op(                        v, 0, 0x76543210, 0x0,                                   8, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 8) {
            v2 = op(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   4, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }
        else if constexpr (Elems == 4) {
            v2 = op(    v.template grow<16>(), 0, 0x76543210, 0x0,                                   2, 0x76543210, 0x0);
            v2 = op(                       v2, 0, 0x76543210, 0x0,                                   1, 0x76543210, 0x0);
        }

        return v2[0];
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_reduce_impl<32, float, Elems, Op>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Op == MaxMinOperation::Max)
            return [](auto &&... args) __aie_inline { return ::fpmax(args...); };
        else
            return [](auto &&... args) __aie_inline { return ::fpmin(args...); };
    }

    static T run(const vector_type &v)
    {
        constexpr auto op = get_op();

        vector<T, 8> v2;

        if constexpr (Elems == 32) {
            v2              = v.template extract<8>(0);
            vector<T, 8> v3 = v.template extract<8>(2);

            v2 = op(v2, v.template extract<16>(0), 8, 0x76543210);
            v3 = op(v3, v.template extract<16>(1), 8, 0x76543210);

            v2 = op(v2, v3.template grow<16>(), 0, 0x76543210);

            v2 = op(v2, v2.template grow<16>(), 4, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 2, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 1, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            v2 = v.template extract<8>(0);

            v2 = op(v2, v,                      8, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 4, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 2, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 1, 0x76543210);
        }
        else if constexpr (Elems == 8) {
            v2 = op(v,   v.template grow<16>(), 4, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 2, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 1, 0x76543210);
        }
        else if constexpr (Elems == 4) {
            v2 = v.template grow<8>();

            v2 = op(v2,  v.template grow<16>(), 2, 0x76543210);
            v2 = op(v2, v2.template grow<16>(), 1, 0x76543210);
        }

        return v2[0];
    }
};

}

#endif
