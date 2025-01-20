// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MUL_MAXMIN_ACC48__HPP__
#define __AIE_API_DETAIL_AIE1_MUL_MAXMIN_ACC48__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_maxmin_bits_impl<MulOp, 48, 8, T1, 8, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        if constexpr (Elems == 128) {
            accum_type<64> tmp1, tmp2;

            tmp1 = mul_maxmin<MulOp, 48, int16, int16>::run( m1.template extract<64>(0).unpack(),
                                                             m2.template extract<64>(0).unpack(),
                                                              v.template extract<64>(0).unpack(),
                                                            acc.template extract<64>(0)...);
            tmp2 = mul_maxmin<MulOp, 48, int16, int16>::run( m1.template extract<64>(1).unpack(),
                                                             m2.template extract<64>(1).unpack(),
                                                              v.template extract<64>(1).unpack(),
                                                            acc.template extract<64>(1)...);

            return concat_accum(tmp1, tmp2);
        }
        else {
            accum_type<Elems> tmp;

            tmp = mul_maxmin<MulOp, 48, int16, int16>::run(m1.unpack(),
                                                           m2.unpack(),
                                                            v.unpack(),
                                                           acc...);

            return tmp;
        }
    }
};

template <MulMacroOp MulOp, typename T2>
struct mul_maxmin_bits_impl<MulOp, 48, 16, int16, 8, T2>
{
    using T1 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        accum_type<Elems> tmp;

        tmp = mul_maxmin<MulOp, 48, int16, int16>::run(m1,
                                                       m2,
                                                       v.unpack(),
                                                       acc...);

        return tmp;
    }
};

template <MulMacroOp MulOp, typename T1>
struct mul_maxmin_bits_impl<MulOp, 48, 8, T1, 16, int16>
{
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        accum_type<Elems> tmp;

        tmp = mul_maxmin<MulOp, 48, int16, int16>::run(m1.unpack(),
                                                       m2.unpack(),
                                                       v,
                                                       acc...);

        return tmp;
    }
};


template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 48, 16, int16, 16, int16>
{
    using T1 = int16;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::mul16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::mac16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::msc16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::mul16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::mac16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::msc16_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &m1,
                                 const vector_type<Elems> &m2,
                                 const vector_type<Elems> &v,
                                 const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              ::concat(m1.template grow_extract<16>(idx), *(const v16int16 *)ZERO), 0, 0x73727170, 0x77767574, 0x3120,
                                              ::concat(m2.template grow_extract<16>(idx), *(const v16int16 *)ZERO), 0,                         0x3120,
                                              v.template grow_extract<16>(idx),                                     0, 0x76543210, 0xfedcba98, 0);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 48, 32, int32, 16, int16>
{
    using T1 = int32;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::mul16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::mac16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::msc16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::mul16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::mac16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::msc16_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              m1.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98,
                                              m2.template grow_extract<16>(idx), 0,
                                              v.template grow_extract<16>(idx),  0, 0x76543210, 0xfedcba98);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 48, 16, int16, 32, int32>
{
    using T1 = int16;
    using T2 = int32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::mul16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::mac16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::msc16_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::mul16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::mac16_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::msc16_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                        m1.template grow_extract<32>(idx / 2), 16 * (idx % 2), 0x76543210, 0x00000000,
                                        m2.template grow_extract<32>(idx / 2), 16 * (idx % 2),
                                        v.template grow_extract<8>(idx * 2),                0, 0x76543210, 0x00000000);
            ret.insert(idx * 2,     tmp.template extract<8>(0));

            if constexpr (Elems == 8) return;

            tmp                = mul_op(acc.template grow_extract<16>(idx)...,
                                        m1.template grow_extract<32>(idx / 2),  16 * (idx % 2), 0x00000000, 0xfedcba98,
                                        m2.template grow_extract<32>(idx / 2),  16 * (idx % 2),
                                        v.template grow_extract<8>(idx * 2 + 1),             0, 0x00000000, 0x76543210);
            ret.insert(idx * 2 + 1, tmp.template extract<8>(1));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_maxmin_bits_impl<MulOp, 48, 16, int16, 32, cint16>
{
    using T1 = int16;
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::MulMax)     return [](auto &&... args) __aie_inline { return ::mul8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMax) return [](auto &&... args) __aie_inline { return ::mac8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMax) return [](auto &&... args) __aie_inline { return ::msc8_max(args...); };
        else if constexpr (MulOp == MulMacroOp::MulMin)     return [](auto &&... args) __aie_inline { return ::mul8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulMin) return [](auto &&... args) __aie_inline { return ::mac8_min(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulMin) return [](auto &&... args) __aie_inline { return ::msc8_min(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &m1,
                                 const vector_type1<Elems> &m2,
                                 const vector_type2<Elems> &v,
                                 const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                       ::concat(m1.template grow_extract<16>(idx / 2), *(const v16int16 *)ZERO), 8 * (idx % 2), 0x76543210, 16,
                                       ::concat(m2.template grow_extract<16>(idx / 2), *(const v16int16 *)ZERO), 8 * (idx % 2),
                                         v.template grow_extract<8>(idx),                                                    0, 0x76543210, 0);
            ret.insert(idx, tmp);
        });

        return ret;
    }
};

}

#endif
