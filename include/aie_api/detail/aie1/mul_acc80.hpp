// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MUL_ACC80__HPP__
#define __AIE_API_DETAIL_AIE1_MUL_ACC80__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../broadcast.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename T1, typename T2>
static constexpr auto mul_acc80_get_mul_op()
{
    constexpr unsigned num_complex = []() constexpr {
        unsigned ret = 0;

        if constexpr (is_complex_v<T1>) ++ret;
        if constexpr (is_complex_v<T2>) ++ret;

        return ret;
    }();

    if constexpr (Lanes == 2) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul2(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1)                         return [](auto &&... args) __aie_inline { return ::lmul2_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2)                         return [](auto &&... args) __aie_inline { return ::lmul2_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul2_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul2(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1)                      return [](auto &&... args) __aie_inline { return ::lnegmul2_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2)                      return [](auto &&... args) __aie_inline { return ::lnegmul2_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul2_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac2(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc2(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac2_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmac2_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1)                     return [](auto &&... args) __aie_inline { return ::lmac2_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2)                     return [](auto &&... args) __aie_inline { return ::lmac2_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac2_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc2_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmsc2_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1)                     return [](auto &&... args) __aie_inline { return ::lmsc2_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2)                     return [](auto &&... args) __aie_inline { return ::lmsc2_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc2_cc(args...); };
    }
    else if constexpr (Lanes == 4) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul4(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul4(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::lmul4_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::lmul4_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::lnegmul4_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::lnegmul4_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac4(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc4(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac4_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmac4_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac4_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc4_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmsc4_abs_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc4_cc(args...); };
    }
    else if constexpr (Lanes == 8) {
        if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul8(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul8(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::lmul8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::lnegmul8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac8(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc8(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac8_abs(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc8_abs(args...); };
    }
}

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 80, 32, int32, 16, int16>
{
    using T1 = int32;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 8;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 8>([&](auto idx) __aie_inline {
            ret.template insert(idx, mul_op(acc.template grow_extract<8>(idx)...,
                                            ::concat(v1.template extract<8>(idx), zeros<T1, 8>::run()),              0, 0x76543210, 8,
                                            v2.template grow_extract<16>(idx / 2),                       8 * (idx % 2), 0x76543210, 0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        if constexpr (Elems == 8) {
            if constexpr (ElemsRef <= 8) {
                ret = mul_op(acc...,
                             ::concat(a.parent.template grow< 8>(), zeros<T1, 8>::run()), a.offset, 0x00000000, 8,
                                             v.template grow<16>(),                              0, 0x76543210, 8);
            }
            else if constexpr (ElemsRef == 16) {
                ret = mul_op(acc...,
                             ::concat(a.parent, zeros<T1, 16>::run()), a.offset, 0x00000000, 16,
                             v.template grow<16>(),                           0, 0x76543210,  8);
            }
            else if constexpr (ElemsRef == 32) {
                const unsigned          idx = a.offset / 8;
                const unsigned local_offset = a.offset % 8;
                ret = mul_op(acc...,
                             ::concat(a.parent.template extract<8>(idx), zeros<T1, 8>::run()), local_offset, 0x00000000, 8,
                             v.template grow<16>(),                                                       0, 0x76543210, 8);
            }

            return ret;
        }
        else if constexpr (Elems == 16 || Elems == 32) {
            ret.template insert<8>(0, mul<MulOp, 80, T1, T2>::run(a, v.template extract<8>(0), acc.template extract<8>(0)...));
            ret.template insert<8>(1, mul<MulOp, 80, T1, T2>::run(a, v.template extract<8>(1), acc.template extract<8>(1)...));
            ret.template insert<8>(2, mul<MulOp, 80, T1, T2>::run(a, v.template extract<8>(2), acc.template extract<8>(2)...));
            ret.template insert<8>(3, mul<MulOp, 80, T1, T2>::run(a, v.template extract<8>(3), acc.template extract<8>(3)...));

            return ret;
        }
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 16;
        unsigned local_offset = a.offset % 16;

        unsigned off;
        if (local_offset < 8) {
            off = 0x00000000;
        }
        else {
            off = 0x88888888;
            local_offset -= 8;
        }

        const vector<T2, 16> subv_parent = a.parent.template grow<std::max(ElemsRef, 16u)>().template extract<16>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 8>([&](auto idx) __aie_inline {
            ret.template insert(0, mul_op(acc...,
                                          ::concat(v.template extract<8>(idx), zeros<T1, 8>::run()),            0, 0x76543210, 8,
                                          subv_parent,                                               local_offset,        off, 0));
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
        const vector<T2, 16> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 80, 32, T1, 32, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static_assert(is_complex_v<T1> || is_complex_v<T2>);

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(v1.template extract<8>(idx / 2), zeros<T1, 8>::run()),       0, 0x3210, 8,
                                               v2.template grow<std::max(Elems, 8u)>().template extract<8>(idx / 2), 0, 0x3210, 4));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(a.parent.template grow<std::max(ElemsRef, 8u)>(), zeros<T1, 8>::run()), a.offset, 0x0000, 8,
                                               v.template grow<std::max(Elems, 8u)>().template extract<8>(idx / 2),                    0, 0x3210, 4));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template extract<4>(idx)...,
                                               ::concat(a.parent.template grow<std::max(ElemsRef, 8u)>(), zeros<T1, 8>::run()), a.offset, 0x0000, 8,
                                               v.template grow<std::max(Elems, 8u)>().template extract<8>(idx / 2),                    0, 0x3210, 4));
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
struct mul_bits_impl<MulOp, 80, 32, int32, 32, int32>
{
    using T1 = int32;
    using T2 = int32;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 8;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v1.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                                                                        8 * (idx % 2), 0x76543210);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         v1.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                         v2.template grow_extract<8>(idx),                  0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v.template grow<std::max(Elems, 16u)>(), a.offset, 0x00000000,
                                                                           idx * 8, 0x76543210);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         a.parent.template grow<std::max(ElemsRef, 16u)>(), a.offset, 0x00000000,
                         v.template grow_extract<8>(idx),                          0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp;

            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                 v.template grow<std::max(Elems, 16u)>(),  idx * 8, 0x76543210,
                                                                          a.offset, 0x00000000);

                    ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
                    return;
                }
            }

            tmp = mul_op(acc.template grow_extract<8>(idx)...,
                         v.template grow<std::max(Elems, 16u)>(),      idx * 8, 0x76543210,
                         subv_parent,                             local_offset, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector_type<16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        const vector_type<8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 80, 64, cint32, 16, int16>
{
    using T1 = cint32;
    using T2 = int16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        const vector<T1, 4> tmp(a);

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
struct mul_bits_impl<MulOp, 80, 16, int16, 64, cint32>
{
    using T1 = int16;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
struct mul_bits_impl<MulOp, 80, 64, cint32, 32, cint16>
{
    using T1 = cint32;
    using T2 = cint16;
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
    static accum_type<Elems> run(const cint32 &a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<cint32, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, const cint32 &a, bool a_sign, const Acc &... acc)
    {
        const vector<cint32, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 80, 64, cint32, 32, int32>
{
    using T1 = cint32;
    using T2 = int32;
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
    static accum_type<Elems> run(const cint32 &a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<cint32, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, const int32 &a, bool a_sign, const Acc &... acc)
    {
        const vector<int32, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 80, 32, int32, 64, cint32>
{
    using T1 = int32;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 4;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

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
    static accum_type<Elems> run(const int32 &a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<int32, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, const cint32 &a, bool a_sign, const Acc &... acc)
    {
        const vector<cint32, 4> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 80, 64, cint32, 64, cint32>
{
    using T1 = cint32;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    using accum_tag = accum_tag_for_mul_types<T1, T2, 80>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned lanes_per_mul = 2;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 2>([&](auto idx) __aie_inline {
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v1, v2))) {
                    ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                                       v1.template grow_extract<8>(idx / 4), 2 * (idx % 4), 0x10,
                                                                                             2 * (idx % 4), 0x10));
                    return;
                }
            }

            ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                               v1.template grow_extract<8>(idx / 4), 2 * (idx % 4), 0x10,
                                               v2.template grow_extract<4>(idx / 2), 2 * (idx % 2), 0x10));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 2>([&](auto idx) __aie_inline {
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(a.parent, v))) {
                    ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                                       v.template grow<std::max(Elems, 8u)>(),      a.offset, 0x00,
                                                                                               2 * (idx % 4), 0x10));
                    return;
                }
            }

            ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                               a.parent.template grow<std::max(ElemsRef, 8u)>(),      a.offset, 0x00,
                                               v.template grow_extract<4>(idx / 2),              2 * (idx % 2), 0x10));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_acc80_get_mul_op<MulOp, lanes_per_mul, T1, T2>();

        const unsigned subv_idx = a.offset / 4;
        const unsigned local_offset = a.offset % 4;

        const vector<T2, 4> subv_parent = a.parent.template grow_extract<4>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 2>([&](auto idx) __aie_inline {
            if constexpr (!has_abs<MulOp>()) {
                if (chess_manifest(utils::is_same_object(v, a.parent))) {
                    ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                                       v.template grow<std::max(Elems, 8u)>(),      a.offset, 0x00,
                                                                                               2 * (idx % 4), 0x10));
                    return;
                }
            }

            ret.template insert<2>(idx, mul_op(acc.template grow_extract<2>(idx)...,
                                               v.template grow_extract<8>(idx / 4), 2 * (idx % 4), 0x10,
                                               subv_parent,                          local_offset, 0x00));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const cint32 &a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<cint32, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, const cint32 &a, bool a_sign, const Acc &... acc)
    {
        const vector<cint32, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

}

#endif
