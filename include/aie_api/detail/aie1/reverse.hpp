// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_REVERSE__HPP__
#define __AIE_API_DETAIL_AIE1_REVERSE__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct reverse_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &a)
    {
        vector_type ret;

        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp_1 = a.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp_2 = a.template extract<Elems / 2>(1);

            ret.insert(0, reverse_impl<8, T, Elems / 2>::run(tmp_2));
            ret.insert(1, reverse_impl<8, T, Elems / 2>::run(tmp_1));
        }
        else {
            ret = reverse_impl<16, int16, Elems>::run(a.unpack()).template pack<T>();
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;
    using native_vector_type = vector<int16, 32>;

    static vector_type run(const vector_type &a)
    {
        vector_type ret;

        if constexpr (Elems == 8) {
            native_vector_type tmp = a.template grow<32>();
            tmp = shuffle32(tmp, 0, 0x0002,     0x0,        0x0123);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            native_vector_type tmp = a.template grow<32>();
            tmp = shuffle32(tmp, 0, 0x00020406, 0x0,        0x0123);

            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            ret = shuffle32(a,   0, 0x080a0c0e, 0x00020406, 0x0123);
        }
        else if constexpr (Elems == 64) {
            auto tmp1 = reverse<T, Elems / 2>::run(vector_cast<T>(a.template extract<32>(0)));
            auto tmp2 = reverse<T, Elems / 2>::run(vector_cast<T>(a.template extract<32>(1)));

            ret.insert(0, vector_cast<T>(tmp2));
            ret.insert(1, vector_cast<T>(tmp1));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<32, T, Elems>
{
    using vector_type        = vector<T, Elems>;
    using native_vector_type = vector<int32, 16>;

    static vector_type run(const vector_type &a)
    {
        vector_type ret;

        if constexpr (Elems == 4) {
            native_vector_type tmp = vector_cast<int32>(a.template grow<16>());
            tmp = shuffle16(tmp, 0, 0x0123,     0x0);

            ret = vector_cast<T>(tmp).template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            native_vector_type tmp = vector_cast<int32>(a.template grow<16>());
            tmp = shuffle16(tmp, 0, 0x01234567, 0x0);

            ret = vector_cast<T>(tmp).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            native_vector_type tmp = vector_cast<int32>(a);
            tmp = shuffle16(tmp, 0, 0x89abcdef, 0x01234567);

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 32) {
            auto tmp1 = reverse<int32, Elems / 2>::run(vector_cast<int32>(a.template extract<16>(0)));
            auto tmp2 = reverse<int32, Elems / 2>::run(vector_cast<int32>(a.template extract<16>(1)));

            ret.insert(0, vector_cast<T>(tmp2));
            ret.insert(1, vector_cast<T>(tmp1));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<64, T, Elems>
{
    using vector_type        = vector<T, Elems>;
    using native_vector_type = vector<cint32, 8>;

    static vector_type run(const vector_type &a)
    {
        vector_type ret;

        if constexpr (Elems == 2) {
            native_vector_type tmp = vector_cast<cint32>(a.template grow<8>());
            tmp = shuffle8(tmp, 0, 0x01);

            ret = vector_cast<T>(tmp).template extract<Elems>(0);
        }
        else if constexpr (Elems == 4) {
            native_vector_type tmp = vector_cast<cint32>(a.template grow<8>());
            tmp = shuffle8(tmp, 0, 0x0123);

            ret = vector_cast<T>(tmp).template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            native_vector_type tmp = vector_cast<cint32>(a);
            tmp = shuffle8(tmp, 0, 0x01234567);

            ret = vector_cast<T>(tmp);
        }
        else if constexpr (Elems == 16) {
            auto tmp1 = reverse<cint32, Elems / 2>::run(vector_cast<cint32>(a.template extract<8>(0)));
            auto tmp2 = reverse<cint32, Elems / 2>::run(vector_cast<cint32>(a.template extract<8>(1)));

            ret.insert(0, vector_cast<T>(tmp2));
            ret.insert(1, vector_cast<T>(tmp1));
        }

        return ret;
    }
};

}

#endif
