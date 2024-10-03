// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ABS__HPP__
#define __AIE_API_DETAIL_AIE2_ABS__HPP__

#include "../broadcast.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct abs_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_abs_elems = 64;
        using native_abs_type = abs_bits_impl<8, T, native_abs_elems>;

        if constexpr (vector_type::is_signed()) {
            if constexpr (Elems == 16 || Elems == 32) {
                ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
            }
            else if constexpr (Elems == 64) {
                ret = ::abs(v);
            }
            else if constexpr (Elems == 128) {
                ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                    native_abs_type::run(v.template extract<native_abs_elems>(1)));
            }
        }
        else {
            ret = v;
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct abs_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_abs_elems = 32;
        using native_abs_type = abs_bits_impl<16, T, native_abs_elems>;

        if constexpr (vector_type::is_signed()) {
            if constexpr (Elems == 8 || Elems == 16) {
                ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                ret = ::abs(v);
            }
            else if constexpr (Elems == 64) {
                ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                    native_abs_type::run(v.template extract<native_abs_elems>(1)));
            }
        }
        else {
            ret = v;
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct abs_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_abs_elems = 16;
        using native_abs_type = abs_bits_impl<32, T, native_abs_elems>;

        if constexpr (vector_type::is_signed()) {
            if constexpr (Elems == 4 || Elems == 8) {
                ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                ret = ::abs(v);
            }
            else if constexpr (Elems == 32) {
                ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                    native_abs_type::run(v.template extract<native_abs_elems>(1)));
            }
        }
        else {
            ret = v;
        }

        return ret;
    }
};

#if __AIE_API_FP32_EMULATION__

template <unsigned Elems>
struct abs_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_abs_elems = 16;
        using native_abs_type = abs_bits_impl<32, T, native_abs_elems>;

        if constexpr (Elems == 4 || Elems == 8) {
            ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector<uint32, Elems> tmp;

            // Implement abs by clearing the upper bit of the elements using an & operation
            tmp = ::band(vector_cast<uint32>(v), broadcast<uint32, Elems>::run(0x7fffffff));

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 32) {
            ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                native_abs_type::run(v.template extract<native_abs_elems>(1)));
        }

        return ret;
    }
};

#endif

}

#endif
