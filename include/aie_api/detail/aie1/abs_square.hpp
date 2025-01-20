// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ABS_SQUARE__HPP__
#define __AIE_API_DETAIL_AIE1_ABS_SQUARE__HPP__

#include "../../accum.hpp"
#include "../../vector.hpp"

namespace aie::detail {

template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cint16, TR, Elems>
{
    using vector_type = vector<cint16, Elems>;
    using comp_type   = vector<int16,  Elems * 2>;
    using return_type = vector<TR,     Elems>;

    template <unsigned Lanes>
    using  accum_type = accum<acc48, Lanes>;

    static return_type run(const vector_type &v, int shift)
    {
        comp_type tmp = v.template cast_to<int16>();

        if      constexpr (Elems < 32) {
            accum_type<16> acc = ::mul16(tmp.template grow<32>(), 0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x06040200, 0x0e0c0a08, 0x3210);

            return (acc.template to_vector<TR>(shift)).template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            return_type res;

            accum_type<16> acc1 = ::mul16(tmp.template extract<32>(0), 0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x06040200, 0x0e0c0a08, 0x3210);
            accum_type<16> acc2 = ::mul16(tmp.template extract<32>(1), 0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x06040200, 0x0e0c0a08, 0x3210);

            res.insert(0, acc1.template to_vector<TR>(shift));
            res.insert(1, acc2.template to_vector<TR>(shift));
            return res;
        }
    }
};

template <typename TR, unsigned Elems>
struct abs_square_bits_impl<cint32, TR, Elems>
{
    using vector_type = vector<cint32, Elems>;
    using comp_type   = vector<int32,  Elems * 2>;
    using return_type = vector<TR,     Elems>;

    template <unsigned Lanes>
    using  accum_type = accum<acc80, Lanes>;

    static return_type run(const vector_type &v, int shift)
    {
        constexpr unsigned num_mul = Elems / 4;

        comp_type tmp = v.template cast_to<int32>();

        accum_type<Elems> res;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<4> tmp_acc = ::lmul4(tmp.template grow_extract<16>(idx / 2),
                                                  (idx % 4) * 8, 0x6420, 1,
                                                  (idx % 4) * 8, 0x6420, 1);
            res.insert(idx, tmp_acc);
        });

        return res.template to_vector<TR>(shift);
    }
};

template <unsigned Elems>
struct abs_square_bits_impl<cfloat, float, Elems>
{
    using vector_type = vector<cfloat, Elems>;
    using comp_type   = vector<float,  Elems * 2>;
    using return_type = vector<float,  Elems>;

    template <unsigned Lanes>
    using  accum_type = accum<accfloat, Lanes>;

    static return_type run(const vector_type &v, int shift)
    {
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        comp_type tmp = v.template cast_to<float>();

        accum_type<Elems> res;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp_acc;

            tmp_acc = ::fpmul(tmp.template grow_extract<16>(idx),
                              0, 0xeca86420,
                              0, 0xeca86420);

            tmp_acc = ::fpmac(tmp_acc,
                              tmp.template grow_extract<16>(idx),
                              1, 0xeca86420,
                              1, 0xeca86420);

            res.insert(idx, tmp_acc.template extract<Elems < 8? Elems : 8>(0));
        });

        return res.template to_vector<float>();
    }
};

}

#endif
