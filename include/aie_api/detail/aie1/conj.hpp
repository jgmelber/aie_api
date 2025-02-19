// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_CONJ__HPP__
#define __AIE_API_DETAIL_AIE1_CONJ__HPP__

#include "../vector.hpp"
#include "../interleave.hpp"
#include "../neg.hpp"

namespace aie::detail {

template <unsigned Elems>
struct conj_bits_impl<32, cint16, Elems>
{
    using vector_type = vector<cint16, Elems>;

    static vector_type run(const vector_type &v)
    {
        const auto real = v.template cast_to<int16>();
        const vector<int16, 16> zero(0);

        constexpr unsigned num_it = Elems < 16? 1 : Elems / 16;

        vector_type ret;

        utils::unroll_times<num_it>([&](auto idx) __aie_inline {
            vector<int16, 32> tmp = ::sub32(           zero.template grow<32>(), 0, 0x00000000, 0x00000000, 0x0000,
                                            real.template grow_extract<32>(idx), 0, 0x06040200, 0x0E0C0A08, 0x3210);

            tmp = ::select32(0x55555555,                                 tmp, 0, 0x06040200, 0x0E0C0A08, 0x3210,
                                         real.template grow_extract<32>(idx), 0, 0x06040200, 0x0E0C0A08, 0x3210);

            ret.insert(idx, tmp.cast_to<cint16>().extract<(Elems < 16)? Elems : 16>(0));
        });

        return ret;
    }
};

template <unsigned Elems>
struct conj_bits_impl<64, cint32, Elems>
{
    using vector_type = vector<cint32, Elems>;

    static vector_type run(const vector_type &v)
    {
        const auto real = v.template cast_to<int32>();
        const vector<int32, 8> zero(0);

        constexpr unsigned num_it = Elems < 8? 1 : Elems / 8;

        vector_type ret;

        utils::unroll_times<num_it>([&](auto idx) __aie_inline {
            vector<int32, 16> tmp = ::sub16(           zero.template grow<16>(), 0, 0x00000000, 0x00000000,
                                            real.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98);

            tmp = ::select16(0x5555,                                 tmp, 0, 0x76543210, 0xfedcba98,
                                     real.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98);

            ret.insert(idx, tmp.cast_to<cint32>().extract<(Elems < 8)? Elems : 8>(0));
        });

        return ret;
    }
};

template <unsigned Elems>
struct conj_bits_impl<64, cfloat, Elems>
{
    using vector_type = vector<cfloat, Elems>;

    static vector_type run(const vector_type &v)
    {
        const auto real = v.template cast_to<float>();

        constexpr unsigned num_it = Elems < 8? 1 : Elems / 8;

        vector_type ret;

        utils::unroll_times<num_it>([&](auto idx) __aie_inline {
            const vector<float, 8> tmp = ::fpneg(real.template grow_extract<16>(idx), 0, 0xfdb97531);

            const vector<float, 16> tmp2 = ::fpselect16(0x5555,                      tmp.grow<16>(), 0, 0x30201000, 0x70605040,
                                                                real.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98);

            ret.insert(idx, tmp2.cast_to<cfloat>().extract<(Elems < 8)? Elems : 8>(0));
        });

        return ret;
    }
};

}

#endif
