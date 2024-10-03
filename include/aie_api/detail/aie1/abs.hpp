// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ABS__HPP__
#define __AIE_API_DETAIL_AIE1_ABS__HPP__

#include "../vector.hpp"

namespace aie::detail {

// TODO: 16b implementation needs to go before 8b. This doesn't seem to be
// necessary for other operations such as add
template <typename T, unsigned Elems>
struct abs_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        constexpr unsigned native_abs_elems = 32;
        using native_abs_type = abs_bits_impl<16, T, native_abs_elems>;

        if constexpr (Elems <= 16) {
            return native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector_type ret = ::abs32(v);

            return ret;
        }
        else if constexpr (Elems == 64) {
            const vector_type ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                                  native_abs_type::run(v.template extract<native_abs_elems>(1)));

            return ret;
        }
    }
};

template <typename T, unsigned Elems>
struct abs_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned native_abs_elems = 32;
        using native_abs_type = abs_bits_impl<8, T, native_abs_elems>;

        if constexpr (Elems == 16) {
            ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = abs_bits_impl<16, int16, 32>::run(v.unpack()).template pack<T>();
        }
        else if constexpr (Elems == 64) {
            ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                native_abs_type::run(v.template extract<native_abs_elems>(1)));
        }
        else if constexpr (Elems == 128) {
            ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                native_abs_type::run(v.template extract<native_abs_elems>(1)),
                                native_abs_type::run(v.template extract<native_abs_elems>(2)),
                                native_abs_type::run(v.template extract<native_abs_elems>(3)));
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

        if constexpr (Elems == 4) {
            ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            ret = native_abs_type::run(v.template grow<native_abs_elems>()).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            ret = ::abs16(v);
        }
        else if constexpr (Elems == 32) {
            ret = concat_vector(native_abs_type::run(v.template extract<native_abs_elems>(0)),
                                native_abs_type::run(v.template extract<native_abs_elems>(1)));
        }

        return ret;
    }
};

template <unsigned Elems>
struct abs_bits_impl<32, float, Elems>
{
    using vector_type = vector<float, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        if constexpr (Elems == 4) {
            const vector<float, 8> tmp = ::fpabs(v.template grow<16>(), 0, 0x76543210);

            ret = tmp.template extract<4>(0);
        }
        else if constexpr (Elems == 8) {
            ret = ::fpabs(v.template grow<16>(), 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            ret = ::concat(::fpabs(v, 0, 0x76543210),
                           ::fpabs(v, 8, 0x76543210));
        }
        else if constexpr (Elems == 32) {
            const vector<float, 8> tmp1 = ::fpabs(v.template extract<16>(0), 0, 0x76543210); ret.insert(0, tmp1);
            const vector<float, 8> tmp2 = ::fpabs(v.template extract<16>(0), 8, 0x76543210); ret.insert(1, tmp2);
            const vector<float, 8> tmp3 = ::fpabs(v.template extract<16>(1), 0, 0x76543210); ret.insert(2, tmp3);
            const vector<float, 8> tmp4 = ::fpabs(v.template extract<16>(1), 8, 0x76543210); ret.insert(3, tmp4);
        }

        return ret;
    }
};

}

#endif
