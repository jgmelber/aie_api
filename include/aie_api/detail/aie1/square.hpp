// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SQUARE__HPP__
#define __AIE_API_DETAIL_AIE1_SQUARE__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T>
struct square_bits_impl<MulOp, 48, 8, T>
{
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using   accum_tag = accum_tag_for_type<T, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        if constexpr (Elems == 128) {
            accum_type<64> tmp1, tmp2;

            tmp1 = square<MulOp, 48, int16>::run(  v.template extract<64>(0).unpack(),
                                                 acc.template extract<64>(0)...);
            tmp2 = square<MulOp, 48, int16>::run(  v.template extract<64>(1).unpack(),
                                                 acc.template extract<64>(1)...);

            return concat_accum(tmp1, tmp2);
        }
        else {
            return square<MulOp, 48, int16>::run(v.unpack(), acc...);
        }
    }
};

template <MulMacroOp MulOp>
struct square_bits_impl<MulOp, 48, 16, int16>
{
    using T = int16;
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using   accum_tag = accum_tag_for_type<T, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (Elems == 8) {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::mul16(args...)); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::mac16(args...)); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::msc16(args...)); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::mul16(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::mac16(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::msc16(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        if constexpr (Elems <= 16) {
            return mul_op(acc.template grow<16>()...,
                          ::concat(v.template grow<16>(), *(const v16int16 *)ZERO), 0, 0x73727170, 0x77767574, 0x3120,
                                                                                    0, 0x73727170, 0x77767574, 0x3120);
        }
        else if constexpr (Elems == 32) {
            return concat_accum(square<MulOp, 48, T>::run(v.template extract<16>(0), acc.template extract<16>(0)...),
                                square<MulOp, 48, T>::run(v.template extract<16>(1), acc.template extract<16>(1)...));
        }
        else if constexpr (Elems == 64) {
            return concat_accum(square<MulOp, 48, T>::run(v.template extract<16>(0), acc.template extract<16>(0)...),
                                square<MulOp, 48, T>::run(v.template extract<16>(1), acc.template extract<16>(1)...),
                                square<MulOp, 48, T>::run(v.template extract<16>(2), acc.template extract<16>(2)...),
                                square<MulOp, 48, T>::run(v.template extract<16>(3), acc.template extract<16>(3)...));
        }
    }
};

template <MulMacroOp MulOp>
struct square_bits_impl<MulOp, 48, 32, cint16>
{
    using T = cint16;
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using   accum_tag = accum_tag_for_type<T, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (Elems == 4) {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::mul8(args...)); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::mac8(args...)); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::msc8(args...)); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::mul8(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::mac8(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::msc8(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        if constexpr (Elems <= 8) {
            return mul_op(acc.template grow<8>()..., v.template grow<16>(), 0, 0x76543210, 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            return concat_accum(square<MulOp, 48, T>::run(v.template extract<8>(0), acc.template extract<8>(0)...),
                                square<MulOp, 48, T>::run(v.template extract<8>(1), acc.template extract<8>(1)...));
        }
        else if constexpr (Elems == 32) {
            return concat_accum(square<MulOp, 48, T>::run(v.template extract<8>(0), acc.template extract<8>(0)...),
                                square<MulOp, 48, T>::run(v.template extract<8>(1), acc.template extract<8>(1)...),
                                square<MulOp, 48, T>::run(v.template extract<8>(2), acc.template extract<8>(2)...),
                                square<MulOp, 48, T>::run(v.template extract<8>(3), acc.template extract<8>(3)...));
        }
    }
};

template <MulMacroOp MulOp>
struct square_bits_impl<MulOp, 80, 32, int32>
{
    using T = int32;
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using   accum_tag = accum_tag_for_type<T, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (Elems == 4) {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::lmul8(args...)); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::lmac8(args...)); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_lo(::lmsc8(args...)); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::lmul8(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::lmac8(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::lmsc8(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        if constexpr (Elems <= 8) {
            return mul_op(acc.template grow<8>()..., v.template grow<16>(), 0, 0x76543210, 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            const accum_type<Elems / 2> acc1 = mul_op(acc.template extract<8>(0)..., v, 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 2> acc2 = mul_op(acc.template extract<8>(1)..., v, 8, 0x76543210, 8, 0x76543210);

            return concat_accum(acc1, acc2);
        }
        else if constexpr (Elems == 32) {
            const accum_type<Elems / 4> acc1 = mul_op(acc.template extract<8>(0)..., v.template extract<16>(0), 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 4> acc2 = mul_op(acc.template extract<8>(1)..., v.template extract<16>(0), 8, 0x76543210, 8, 0x76543210);
            const accum_type<Elems / 4> acc3 = mul_op(acc.template extract<8>(2)..., v.template extract<16>(1), 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 4> acc4 = mul_op(acc.template extract<8>(3)..., v.template extract<16>(1), 8, 0x76543210, 8, 0x76543210);

            return concat_accum(acc1, acc2, acc3, acc4);
        }
    }
};

template <MulMacroOp MulOp>
struct square_bits_impl<MulOp, 32, 32, float>
{
    using T = float;
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using accum_tag   = accum_tag_for_type<T, 32>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (Elems == 4) {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::ext_v(::fpmul(args...), 0); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_v(::fpmac(args...), 0); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::ext_v(::fpmsc(args...), 0); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)
                return [](auto &&... args) __aie_inline { return ::fpmul(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_Mul)
                return [](auto &&... args) __aie_inline { return ::fpmac(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_Mul)
                return [](auto &&... args) __aie_inline { return ::fpmsc(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        if constexpr (Elems <= 8) {
            return mul_op(acc.template grow<8>()..., v.template grow<16>(), 0, 0x76543210, 0, 0x76543210);
        }
        else if constexpr (Elems == 16) {
            const accum_type<Elems / 2> acc1 = mul_op(acc.template extract<8>(0)..., v, 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 2> acc2 = mul_op(acc.template extract<8>(1)..., v, 8, 0x76543210, 8, 0x76543210);

            return concat_accum(acc1, acc2);
        }
        else if constexpr (Elems == 32) {
            const accum_type<Elems / 4> acc1 = mul_op(acc.template extract<8>(0)..., v.template extract<16>(0), 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 4> acc2 = mul_op(acc.template extract<8>(1)..., v.template extract<16>(0), 8, 0x76543210, 8, 0x76543210);
            const accum_type<Elems / 4> acc3 = mul_op(acc.template extract<8>(2)..., v.template extract<16>(1), 0, 0x76543210, 0, 0x76543210);
            const accum_type<Elems / 4> acc4 = mul_op(acc.template extract<8>(3)..., v.template extract<16>(1), 8, 0x76543210, 8, 0x76543210);

            return concat_accum(acc1, acc2, acc3, acc4);
        }
    }
};

template <MulMacroOp MulOp>
struct square_bits_impl<MulOp, 80, 64, cint32>
{
    using T = cint32;
    template <unsigned Elems>
    using vector_type = vector<T, Elems>;

    using   accum_tag = accum_tag_for_type<T, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)
            return [](auto &&... args) __aie_inline { return ::lmul2(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)
            return [](auto &&... args) __aie_inline { return ::lmac2(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)
            return [](auto &&... args) __aie_inline { return ::lmsc2(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type<Elems> &v, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        if constexpr (Elems == 2) {
            return mul_op(acc..., v.template grow<8>(), 0, 0x10, 0, 0x10);
        }
        else if constexpr (Elems == 4) {
            const accum_type<Elems> ret = concat(mul_op(acc.template extract<2>(0)..., v.template grow<8>(), 0, 0x10, 0, 0x10),
                                                 mul_op(acc.template extract<2>(1)..., v.template grow<8>(), 2, 0x10, 2, 0x10));

            return ret;
        }
        else if constexpr (Elems == 8) {
            const accum_type<Elems / 2> acc1 = concat(mul_op(acc.template extract<2>(0)..., v, 0, 0x10, 0, 0x10),
                                                      mul_op(acc.template extract<2>(1)..., v, 2, 0x10, 2, 0x10));
            const accum_type<Elems / 2> acc2 = concat(mul_op(acc.template extract<2>(2)..., v, 4, 0x10, 4, 0x10),
                                                      mul_op(acc.template extract<2>(3)..., v, 6, 0x10, 6, 0x10));

            return concat_accum(acc1, acc2);
        }
        else if constexpr (Elems == 16) {
            const accum_type<Elems / 4> acc1 = ::concat(mul_op(acc.template extract<2>(0)..., v.template extract<8>(0), 0, 0x10, 0, 0x10),
                                                        mul_op(acc.template extract<2>(1)..., v.template extract<8>(0), 2, 0x10, 2, 0x10));
            const accum_type<Elems / 4> acc2 = ::concat(mul_op(acc.template extract<2>(2)..., v.template extract<8>(0), 4, 0x10, 4, 0x10),
                                                        mul_op(acc.template extract<2>(3)..., v.template extract<8>(0), 6, 0x10, 6, 0x10));
            const accum_type<Elems / 4> acc3 = ::concat(mul_op(acc.template extract<2>(4)..., v.template extract<8>(1), 0, 0x10, 0, 0x10),
                                                        mul_op(acc.template extract<2>(5)..., v.template extract<8>(1), 2, 0x10, 2, 0x10));
            const accum_type<Elems / 4> acc4 = ::concat(mul_op(acc.template extract<2>(6)..., v.template extract<8>(1), 4, 0x10, 4, 0x10),
                                                        mul_op(acc.template extract<2>(7)..., v.template extract<8>(1), 6, 0x10, 6, 0x10));

            return concat_accum(acc1, acc2, acc3, acc4);
        }
    }
};

}

#endif
