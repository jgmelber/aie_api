// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MUL_ACC48__HPP__
#define __AIE_API_DETAIL_AIE1_MUL_ACC48__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../broadcast.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename T1, typename T2>
static constexpr auto mul_acc48_get_mul_op()
{
    constexpr unsigned num_complex = []() constexpr {
        unsigned ret = 0;

        if constexpr (is_complex_v<T1>) ++ret;
        if constexpr (is_complex_v<T2>) ++ret;

        return ret;
    }();

    if constexpr (Lanes == 4) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul4(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul4(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac4(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc4(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc4_cc(args...); };
    }
    else if constexpr (Lanes == 8) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul8(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul8(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::mul8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::mul8_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul8_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::negmul8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::negmul8_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul8_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac8(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc8(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::mac8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::mac8_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac8_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::msc8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::msc8_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc8_cc(args...); };
    }
    else if constexpr (Lanes == 16) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul16(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul16(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::mul16_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::negmul16_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac16(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc16(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::mac16_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::msc16_abs(args...); };
    }
}

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 48, 8, T1, 8, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        if constexpr (Elems == 128) {
            accum_type<Elems> ret;
            accum_type<64> tmp;

            tmp = mul<MulOp, 48, int16, int16>::run(v1.template extract<64>(0).unpack(),
                                                    v1_sign,
                                                    v2.template extract<64>(0).unpack(),
                                                    v2_sign,
                                                    acc.template extract<64>(0)...);
            ret.insert(0, tmp);
            tmp = mul<MulOp, 48, int16, int16>::run(v1.template extract<64>(1).unpack(),
                                                    v1_sign,
                                                    v2.template extract<64>(1).unpack(),
                                                    v2_sign,
                                                    acc.template extract<64>(1)...);
            ret.insert(1, tmp);

            return ret;
        }
        else {
            return mul<MulOp, 48, int16, int16>::run(v1.unpack(), v1_sign, v2.unpack(), v2_sign, acc...);
        }
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return run(a.get(), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, a.get(), a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        if constexpr (Elems == 128) {
            accum_type<Elems> ret;
            accum_type<64> tmp;

            // T2 can only be int8 / uint8, so casting it to int16 is safe
            tmp = mul<MulOp, 48, int16, int16>::run((int16)a, a_sign, v.template extract<64>(0).unpack(), v_sign, acc.template extract<64>(0)...);
            ret.insert(0, tmp);
            tmp = mul<MulOp, 48, int16, int16>::run((int16)a, a_sign, v.template extract<64>(1).unpack(), v_sign, acc.template extract<64>(1)...);
            ret.insert(1, tmp);

            return ret;
        }
        else {
            // T2 can only be int8 / uint8, so casting it to int16 is safe
            return mul<MulOp, 48, int16, int16>::run((int16)a, a_sign, v.unpack(), v_sign, acc...);
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        if constexpr (Elems == 128) {
            accum_type<Elems> ret;
            accum_type<64> tmp;

            // T2 can only be int8 / uint8, so casting it to int16 is safe
            tmp = mul<MulOp, 48, int16, int16>::run(v.template extract<64>(0).unpack(), v_sign, (int16)a, a_sign, acc.template extract<64>(0)...);
            ret.insert(0, tmp);
            tmp = mul<MulOp, 48, int16, int16>::run(v.template extract<64>(1).unpack(), v_sign, (int16)a, a_sign, acc.template extract<64>(1)...);
            ret.insert(1, tmp);

            return ret;
        }
        else {
            // T2 can only be int8 / uint8, so casting it to int16 is safe
            return mul<MulOp, 48, int16, int16>::run(v.unpack(), v_sign, (int16)a, a_sign, acc...);
        }
    }
};

template <MulMacroOp MulOp, typename T2>
struct mul_bits_impl<MulOp, 48, 16, int16, 8, T2>
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
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v1, v1_sign, v2.unpack(), v2_sign, acc...);
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(a, a_sign, v.unpack(), v_sign, acc...);
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v.unpack(), v_sign, a, a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(a, a_sign, v.unpack(), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v, v_sign, a, a_sign, acc...);
    }
};

template <MulMacroOp MulOp, typename T1>
struct mul_bits_impl<MulOp, 48, 8, T1, 16, int16>
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
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v1.unpack(), v1_sign, v2, v2_sign, acc...);
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(a, a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v.unpack(), v_sign, a, a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(a, a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 48, int16, int16>::run(v.unpack(), v_sign, a, a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 16, int16, 16, int16>
{
    using T1 = int16;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 16;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp;
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                 ::concat(v1.template grow_extract<16>(idx), zeros<T1, 16>::run()), 0, 0x73727170, 0x77767574, 0x3120,
                                                                                                    0, 0x73727170, 0x77767574, 0x3120);

                    ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<16>(idx)...,
                         ::concat(v1.template grow_extract<16>(idx), zeros<T1, 16>::run()),     0, 0x73727170, 0x77767574, 0x3120,
                                                             v2.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98, 0);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        // TODO: write specialization that can be directly implemented without extracting the element
        return run(a.get(), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        // TODO: write specialization that can be directly implemented without extracting the element
        return run(v, v_sign, a.get(), a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        // Promote the scalar to 32-bit so we can use mul16 with no need for predication
        const vector<int32, 16> scalar(a);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              scalar,                           0, 0x00000000, 0x00000000,
                                              v.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        // Promote the scalar to 32-bit so we can use mul16 with no need for predication
        const vector<int32, 8> scalar(a);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              v.template grow_extract<32>(idx / 2), 16 * (idx % 2), 0x76543210, 0xfedcba98,
                                              scalar,                                            0, 0x00000000, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 32, int32, 16, int16>
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

    static constexpr unsigned lanes_per_mul = 16;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              v1.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98,
                                              v2.template grow_extract<16>(idx), 0, 0x76543210, 0xfedcba98);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              a.parent.template grow<std::max(ElemsRef, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                              v.template grow_extract<16>(idx),                         0, 0x76543210, 0xfedcba98);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        const unsigned subv_idx = a.offset / 16;
        const unsigned local_offset = a.offset % 16;

        const vector<T2, 16> subv_parent = a.parent.template grow_extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                              v.template grow_extract<16>(idx),            0, 0x76543210, 0xfedcba98,
                                              subv_parent,                      local_offset, 0x00000000, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 16> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }

};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 32, cint16, 16, int16>
{
    using T1 = cint16;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 8;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                             ::concat(v1.template grow_extract<8>(idx), zeros<T1, 8>::run()),             0, 0x76543210, 8,
                                             v2.template grow_extract<16>(idx / 2),                           8 * (idx % 2), 0x76543210, 0);
            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                             a.parent.template grow<std::max(ElemsRef, 16u)>(),        a.offset, 0x00000000, 0,
                                             ::concat(v.template grow_extract<8>(idx),  zeros<T2, 8>::run()), 0, 0x76543210, 8);
            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 16;
        const unsigned local_offset = a.offset % 16;

        const vector<T2, 16> subv_parent = a.parent.template grow_extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                             ::concat(v.template grow_extract<8>(idx), zeros<T1, 8>::run()),            0, 0x76543210, 8,
                                             subv_parent,                                                    local_offset, 0x00000000, 0);
            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 16> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 16, int16, 32, int32>
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

    static constexpr unsigned lanes_per_mul = 16;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp;

            tmp = mul_op(acc.template grow_extract<16>(idx)...,
                         v1.template grow_extract<32>(idx / 2), (idx % 2) * 16, 0x76543210, 0x00000000,
                         v2.template grow_extract<8>(idx * 2),               0, 0x76543210, 0x00000000);

            ret.template insert<8>(idx * 2, tmp.template extract<8>(0));

            if constexpr (Elems == 8) return;

            tmp = mul_op(acc.template extract<8>(idx * 2 + 1).template grow<16>()...,
                         v1.template grow_extract<32>(idx / 2), (idx % 2) * 16, 0xfedcba98, 0x00000000,
                         v2.template grow_extract<8>(idx * 2 + 1),           0, 0x76543210, 0x00000000);

            ret.template insert<8>(idx * 2 + 1, tmp.template extract<8>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        const unsigned subv_idx = a.offset / 16;
        const unsigned local_offset = a.offset % 16;

        const vector<T1, 16> subv_parent = a.parent.template grow_extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp;

            tmp = mul_op(acc.template grow_extract<16>(idx)...,
                         subv_parent.template grow<32u>(),    local_offset, 0x00000000, 0x00000000,
                         v.template grow_extract<8>(idx * 2),            0, 0x76543210, 0x00000000);

            ret.template insert<8>(idx * 2, tmp.template extract<8>(0));

            if constexpr (Elems == 8) return;

            tmp = mul_op(acc.template extract<8>(idx * 2 + 1).template grow<16>()...,
                         subv_parent.template grow<32u>(),        local_offset, 0x00000000, 0x00000000,
                         v.template grow_extract<8>(idx * 2 + 1),            0, 0x76543210, 0x00000000);

            ret.template insert<8>(idx * 2 + 1, tmp.template extract<8>(0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp;

            tmp = mul_op(acc.template grow_extract<16>(idx)...,
                         v.template grow_extract<32>(idx / 2), 16 * (idx % 2), 0x76543210, 0xfedcba98,
                         subv_parent,                            local_offset, 0x00000000, 0x00000000);

            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 16, int16, 32, cint16>
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

    static constexpr unsigned lanes_per_mul = 8;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         ::concat(v1.template grow_extract<16>(idx / 2), zeros<T1, 16>::run()), 8 * (idx % 2), 0x76543210, 16,
                         v2.template grow_extract<8>(idx),                                                  0, 0x76543210, 0);

            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 16;
        const unsigned local_offset = a.offset % 16;

        const vector<T1, 16> subv_parent = a.parent.template grow_extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         ::concat(subv_parent, zeros<T1, 16>::run()), local_offset, 0x00000000, 16,
                         v.template grow_extract<8>(idx),                        0, 0x76543210, 0);

            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         ::concat(v.template grow_extract<16>(idx / 2), zeros<T1, 16>::run()), 8 * (idx % 2), 0x76543210, 16,
                         subv_parent,                                                           local_offset, 0x00000000, 0);

            ret.insert(idx, tmp);
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 64, cint32, 16, int16>
{
    using T1 = cint32;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(v1.template grow_extract<4>(idx), zeros<T1, 4>::run()),             0, 0x3210, 4,
                                               v2.template grow_extract<8>(idx / 2).template grow<16>(),        4 * (idx % 2), 0x3210, 0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(a.parent.template grow<std::max(ElemsRef, 4u)>(), zeros<T1, 4>::run()),      a.offset, 0x0000, 4,
                                               v.template grow_extract<8>(idx / 2).template grow<16>(),                         4 * (idx % 2), 0x3210, 0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(v.template grow_extract<4>(idx), zeros<T1, 4>::run()),             0, 0x3210, 4,
                                               subv_parent.template grow<16>(),                                 local_offset, 0x0000, 0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 2> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 16, int16, 64, cint32>
{
    using T1 = int16;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(v1.template grow_extract<16>(idx / 4), zeros<T1, 16>::run()), 4 * (idx % 4), 0x3210, 16,
                                               v2.template extract<4>(idx),                                                       0, 0x3210, 0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 16;
        const unsigned local_offset = a.offset % 16;

        const vector<T1, 16> subv_parent = a.parent.template grow_extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(subv_parent, zeros<T1, 16>::run()), local_offset, 0x0000, 16,
                                               v.template extract<4>(idx),                             0, 0x3210, 0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(v.template grow_extract<16>(idx / 4), zeros<T1, 16>::run()), 4 * (idx % 4), 0x3210, 16,
                                               a.parent.template grow<std::max(ElemsRef, 4u)>(),                          a.offset, 0x0000, 0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 2> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 64, cint32, 32, cint16>
{
    using T1 = cint32;
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               v1.template grow_extract<8>(idx / 2), (idx % 2) * 4, 0x3210,
                                               v2.template grow_extract<8>(idx / 2), (idx % 2) * 4, 0x3210));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               a.parent.template grow<std::max(ElemsRef, 8u)>(),      a.offset, 0x0000,
                                               v.template grow_extract<8>(idx / 2),              (idx % 2) * 4, 0x3210));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               v.template grow_extract<8>(idx / 2), (idx % 2) * 4, 0x3210,
                                               subv_parent,                          local_offset, 0x0000));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 48, 32, cint16, 64, cint32>
{
    using T1 = cint16;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               v1.template grow_extract<16>(idx / 4), (idx % 4) * 4, 0x3210,
                                               v2.template grow_extract<4>(idx),                  0, 0x3210));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               a.parent.template grow<std::max(ElemsRef, 16u)>(), a.offset, 0x0000,
                                               v.template grow_extract<4>(idx),                          0, 0x3210));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 4;
        const unsigned local_offset = a.offset % 4;

        const vector<T2, 4> subv_parent = a.parent.template grow_extract<4>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               v.template grow_extract<16>(idx / 4), (idx % 4) * 4, 0x3210,
                                               subv_parent,                           local_offset, 0x0000));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 48, 32, T1, 32, T2>
{
    template <unsigned Elems>
    using vector1_type = vector<T1, Elems>;

    template <unsigned Elems>
    using vector2_type = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 48>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 8;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v1, bool v1_sign, const vector2_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;
            if constexpr (std::is_same_v<T1, T2> && !has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v1.template grow_extract<16>(idx), (idx % 2) * 8, 0x76543210, (idx % 2) * 8, 0x76543210);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         v1.template grow_extract<16>(idx / 2), (idx % 2) * 8, 0x76543210,
                         v2.template grow_extract<8>(idx),                  0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;
            if constexpr (std::is_same_v<T1, T2> && !has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v.template grow<std::max(Elems, 16u)>(), a.offset, 0x00000000, (idx % 2) * 8, 0x76543210);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         a.parent.template grow<std::max(Elems, 16u)>(), a.offset, 0x00000000,
                         v.template grow_extract<8>(idx),                       0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc48_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;
            if constexpr (std::is_same_v<T1, T2> && !has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v, a.parent))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v.template grow<std::max(Elems, 16u)>(), (idx % 2) * 8, 0x76543210, a.offset, 0x00000000);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         v.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                         subv_parent,                           local_offset, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<T1, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector<T2, 8> tmp(a);
        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

}

#endif
