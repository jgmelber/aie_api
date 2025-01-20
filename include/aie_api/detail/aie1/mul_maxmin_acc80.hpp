// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MUL_MAXMIN_ACC80__HPP__
#define __AIE_API_DETAIL_AIE1_MUL_MAXMIN_ACC80__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 80, 32, int32, 16, int16>
{
    using T1 = int32;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::lmul8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::lmac8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::lmsc8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::lmul8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::lmac8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::lmsc8_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 8>([&](auto idx) __aie_inline {
            ret.template insert<8>(idx, mul_op(acc.template grow_extract<16>(idx)...,
                                               m1.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                                               m2.template grow_extract<16>(idx / 2), 8 * (idx % 2),
                                               v.template grow_extract<16>(idx / 2),  8 * (idx % 2), 0x76543210));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 80, 16, int16, 32, int32>
{
    using T1 = int16;
    using T2 = int32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::lmul8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::lmac8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::lmsc8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::lmul8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::lmac8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::lmsc8_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 8>([&](auto idx) __aie_inline {
            ret.template insert<8>(idx, mul_op(acc.template grow_extract<16>(idx)...,
                                               m1.template grow_extract<32>(idx / 4), 8 * (idx % 4), 0x76543210,
                                               m2.template grow_extract<32>(idx / 4), 8 * (idx % 4),
                                               v.template grow_extract<8>(idx),                   0, 0x76543210));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 80, 32, int32, 32, int32>
{
    using T1 = int32;
    using T2 = int32;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::lmul8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::lmac8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::lmsc8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::lmul8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::lmac8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::lmsc8_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &m1,
                                 const vector_type<Elems> &m2,
                                 const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                             m1.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                                             m2.template grow_extract<16>(idx / 2), 8 * (idx % 2),
                                             v.template grow_extract<8>(idx),                   0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }
};

}

#endif
