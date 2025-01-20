// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_REVERSE__HPP__
#define __AIE_API_DETAIL_AIE2P_REVERSE__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct reverse_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        vector_type ret;

        if constexpr (vector_type::bits() == 1024) {
            ret.template insert(1, reverse_impl<8, next_type, Elems / 2>::run(v.template extract<Elems / 2>(0).unpack()).pack());
            ret.template insert(0, reverse_impl<8, next_type, Elems / 2>::run(v.template extract<Elems / 2>(1).unpack()).pack());
        }
        else {
            return reverse_impl<8, next_type, Elems>::run(v.unpack()).pack();
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned iter = Elems <= 32? 1 : Elems / 64;

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            vector<T, 64> v2;

            // This multiplication reverses groups of 16 32b elements
            v2 = ::shuffle(v.template grow_extract<64>(idx), T32_1x16_flip);
            // This multiplication reverses groups of 4 8b elements
            v2 = ::shuffle(v2,                               T8_1x4_flip);

            if constexpr (Elems <= 32)
                ret.insert(0, v2.template extract<Elems>(64 / Elems - 1));
            else
                ret.insert(Elems / 64 - idx - 1, v2);
        });

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned iter = Elems <= 16? 1 : Elems / 32;

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            vector<T, 32> v2;

            // This multiplication reverses groups of 16 32b elements
            v2 = ::shuffle(v.template grow_extract<32>(idx), T32_1x16_flip);
            // This multiplication reverses groups of 2 16b elements
            v2 = ::shuffle(v2,                               T16_1x2_flip);

            if constexpr (Elems <= 16)
                ret.insert(0, v2.template extract<Elems>(32 / Elems - 1));
            else
                ret.insert(Elems / 32 - idx - 1, v2);
        });

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned iter = Elems <= 8? 1 : Elems / 16;

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            vector<T, 16> v2;

            // This multiplication reverses groups of 16 32b elements
            v2 = ::shuffle(v.template grow_extract<16>(idx), T32_1x16_flip);

            if constexpr (Elems <= 8)
                ret.insert(0, v2.template extract<Elems>(16 / Elems - 1));
            else
                ret.insert(Elems / 16 - idx - 1, v2);
        });

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        vector_type ret;

        constexpr unsigned iter = Elems <= 4? 1 : Elems / 8;

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            vector<uint32, 16> v2 = reverse<uint32, 16>::run(v.template grow_extract<8>(idx).template cast_to<uint32>());
            vector<uint32, 16> hi, lo;

            lo = ::shuffle(v2, v2, DINTLV_lo_32o64);
            hi = ::shuffle(v2, v2, DINTLV_hi_32o64);

            v2 = ::shuffle(hi, lo, INTLV_lo_32o64);

            if constexpr (Elems <= 4)
                ret.insert(0, v2.template cast_to<T>().template extract<Elems>(8 / Elems - 1));
            else
                ret.insert(Elems / 8 - idx - 1, v2.template cast_to<T>());
        });

        return ret;
    }
};

}

#endif
