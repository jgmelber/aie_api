// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MUL_ACC32_FP__HPP__
#define __AIE_API_DETAIL_AIE2P_MUL_ACC32_FP__HPP__

#include <algorithm>

#include "../utils.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
#if __AIE_ARCH__ == 21
    requires(std::is_same_v<T1, bfloat16> && std::is_same_v<T2, bfloat16>)
#elif __AIE_ARCH__ == 22
    requires(utils::is_one_of_v<T1, bfloat16, float16> &&
             utils::is_one_of_v<T2, bfloat16, float16>)
         
#endif
struct mul_bits_impl<MulOp, 32, 16, T1, 16, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accfloat, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (Elems <= 32) {
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32_conf(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32_conf(args...); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_64_conf(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_64_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_64_conf(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_64_conf(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign,
                                 const vector_type2<Elems> &v2, bool v2_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned muls_per_mul = Elems <= 32? 32 : 64;

        constexpr unsigned num_mul = Elems < muls_per_mul? 1 : Elems / muls_per_mul;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<muls_per_mul> tmp = mul_op(v1.template grow_extract<muls_per_mul>(idx), v1_sign,
                                                        v2.template grow_extract<muls_per_mul>(idx), v2_sign,
                                                        utils::get_nth<0>(acc.template grow_extract<muls_per_mul>(idx), acc)...,
                                                        utils::get_nth<0>(zero_acc, acc)...,
                                                        sub_mul,
                                                        utils::get_nth<0>(sub_acc, acc)...);

            ret.insert(idx, tmp.template extract<(Elems < muls_per_mul? Elems : muls_per_mul)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a,                         bool a_sign,
                                 const vector_type2<Elems> &v, bool v_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 21
        return run(broadcast<T1, Elems>::run(a), a_sign, v.template grow<Elems>(), v_sign, sub_mul, sub_acc, zero_acc, acc...);
#elif __AIE_ARCH__ == 22
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned muls_per_mul = Elems <= 32? 32 : 64;

        constexpr unsigned num_mul = Elems < muls_per_mul? 1 : Elems / muls_per_mul;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<muls_per_mul> tmp = mul_op(v.template grow_extract<muls_per_mul>(idx), v_sign,
                                                        a,                                          a_sign,
                                                        utils::get_nth<0>(acc.template grow_extract<muls_per_mul>(idx), acc)...,
                                                        utils::get_nth<0>(zero_acc, acc)...,
                                                        sub_mul,
                                                        utils::get_nth<0>(sub_acc, acc)...);

            ret.insert(idx, tmp.template extract<(Elems < muls_per_mul? Elems : muls_per_mul)>(0));
        });

        return ret;
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign,
                                 T2 a, bool a_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
#if __AIE_ARCH__ == 21
        return run(v.template grow<Elems>(), v_sign, broadcast<T2, Elems>::run(a), a_sign, sub_mul, sub_acc, zero_acc, acc...);
#elif __AIE_ARCH__ == 22
        return mul_bits_impl<MulOp, 32, 16, T2, 16, T1>::run(a, a_sign, v, v_sign, sub_mul, sub_acc, zero_acc, acc...);
#endif
    }
};

#if __AIE_API_FP32_EMULATION__
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, float, 32, float>
{
    using T = float;

    template <unsigned Elems>
    using vector_type = vector<float, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accfloat, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::mul_elem_32_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32_conf(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign,
                                 const vector_type<Elems> &v2, bool v2_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned muls_per_mul = 32;

        constexpr unsigned num_mul = Elems < muls_per_mul ? 1 : Elems / muls_per_mul;

        if constexpr (MulOp == MulMacroOp::NegMul)
            sub_mul = sub_mul ^ 1;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<muls_per_mul> tmp = mul_op(v1.template grow_extract<muls_per_mul>(idx),
                                                        v2.template grow_extract<muls_per_mul>(idx),
                                                        utils::get_nth<0>(acc.template grow_extract<muls_per_mul>(idx), acc)...,
                                                        utils::get_nth<0>(zero_acc, acc)...,
                                                        sub_mul,
                                                        utils::get_nth<0>(sub_acc, acc)...);

            ret.insert(idx, tmp.template extract<(Elems < muls_per_mul? Elems : muls_per_mul)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T a,                         bool a_sign,
                                 const vector_type<Elems> &v, bool v_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(broadcast<T, Elems>::run(a), a_sign, v.template grow<Elems>(), v_sign, sub_mul, sub_acc, zero_acc, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign,
                                 T a,                         bool a_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(v.template grow<Elems>(), v_sign, broadcast<T, Elems>::run(a), a_sign, sub_mul, sub_acc, zero_acc, acc...);
    }
};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__

template <MulMacroOp MulOp, typename T1, typename T2>
    requires(is_floating_point_v<T1> && is_floating_point_v<T2> &&
             (is_complex_v<T1> || is_complex_v<T2>))
struct mul_impl_cfloat_common
{
    template <unsigned Elems>
    using  accum_type = accum<caccfloat, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        constexpr auto sub_mask = [&](){
            if      constexpr (has_conj1<MulOp>() && has_conj2<MulOp>()) return OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y;
            else if constexpr (has_conj1<MulOp>())                       return OP_TERM_NEG_COMPLEX_CONJUGATE_X;
            else if constexpr (has_conj2<MulOp>())                       return OP_TERM_NEG_COMPLEX_CONJUGATE_Y;
            else                                                         return OP_TERM_NEG_COMPLEX;
        }();
        
        if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2)
            return [sub_mask](auto x, auto y, auto sub_mul) __aie_inline { return ::mul_elem_16_conf(x, y, sub_mask, sub_mul); };
        if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2)
            return [sub_mask](auto x, auto y, auto sub_mul) __aie_inline { return ::negmul_elem_16_conf(x, y, sub_mask, sub_mul); };
        if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2)
            return [sub_mask](auto x, auto y, auto acc, auto zero_acc, auto sub_mul, auto sub_acc) __aie_inline {
                return ::mac_elem_16_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc);
            };
        if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2)
            return [sub_mask](auto x, auto y, auto acc, auto zero_acc, auto sub_mul, auto sub_acc) __aie_inline {
                return ::msc_elem_16_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc);
            };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector<T1, Elems> &v1, const bool v1_sign,
                                 const vector<T2, Elems> &v2, const bool v2_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned basic_elems = 16;
        constexpr unsigned num_mul = Elems < basic_elems? 1 : Elems / basic_elems;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<basic_elems> tmp = mul_op(v1.template grow_extract<basic_elems>(idx),
                                                       v2.template grow_extract<basic_elems>(idx),
                                                       utils::get_nth<0>(acc.template grow_extract<basic_elems>(idx), acc)...,
                                                       utils::get_nth<0>(zero_acc, acc)...,
                                                       sub_mul,
                                                       utils::get_nth<0>(sub_acc, acc)...);

            ret.insert(idx, tmp.template extract<std::min(Elems, basic_elems)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a,                       const bool a_sign,
                                 const vector<T2, Elems> &v, const bool v_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, sub_mul, sub_acc, zero_acc, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector<T1, Elems> &v, const bool v_sign,
                                 T2 a,                       const bool a_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, sub_mul, sub_acc, zero_acc, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, float, 64, cfloat> : mul_impl_cfloat_common<MulOp, float, cfloat> {};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 32, float> : mul_impl_cfloat_common<MulOp, cfloat, float> {};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 64, cfloat> : mul_impl_cfloat_common<MulOp, cfloat, cfloat> {};

#endif

}
#endif
