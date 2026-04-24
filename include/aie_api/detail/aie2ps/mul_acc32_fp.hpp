// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2PS_MUL_ACC32_FP__HPP__
#define __AIE_API_DETAIL_AIE2PS_MUL_ACC32_FP__HPP__

#include <algorithm>

#include "../utils.hpp"

#include "emulated_mul_intrinsics.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_fp8_common
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using accum_type   = accum<accfloat, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, bool sub_mul, bool sub_acc, bool zero_acc, const Acc &... acc)
    {
        return mul_elem_conf_common_t<MulOp>(v1, v2, sub_mul, sub_acc, zero_acc, utils::get_nth<0>(acc)...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, bool sub_mul, bool sub_acc, bool zero_acc, const Acc &... acc)
    {
        return mul_bits_fp8_common<MulOp, T2, T1>::run(v, v_sign, a, a_sign, sub_mul, sub_acc, zero_acc, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, bool sub_mul, bool sub_acc, bool zero_acc, const Acc &... acc)
    {
        return mul_elem_conf_common_t<MulOp, T2>(v, a, sub_mul, sub_acc, zero_acc, utils::get_nth<0>(acc)...);
    }
};

#if __AIE_API_FP8_SUPPORT__
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 8, float8, 8, float8> : public mul_bits_fp8_common<MulOp, float8, float8> {};
#endif

#if __AIE_API_BF8_SUPPORT__
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 8, bfloat8, 8, bfloat8> : public mul_bits_fp8_common<MulOp, bfloat8, bfloat8> {};
#endif

#if __AIE_API_CBF16_SUPPORT__
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_cbf16_common
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
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
        
        if constexpr (MulOp == MulMacroOp::Mul || MulOp == MulMacroOp::MulConj1 || MulOp == MulMacroOp::MulConj1Conj2 || MulOp == MulMacroOp::MulConj2) {
            if constexpr (Elems == 16) return [sub_mask](auto x, auto y, bool sub_mul) __aie_inline { return ::mul_elem_16_conf(x, y, sub_mask, sub_mul); };
            else                       return [sub_mask](auto x, auto y, bool sub_mul) __aie_inline { return ::mul_elem_32_conf(x, y, sub_mask, sub_mul); };
        }
        if constexpr (MulOp == MulMacroOp::NegMul || MulOp == MulMacroOp::NegMulConj1 || MulOp == MulMacroOp::NegMulConj1Conj2 || MulOp == MulMacroOp::NegMulConj2) {
            if constexpr (Elems == 16) return [sub_mask](auto x, auto y, bool sub_mul) __aie_inline { return ::negmul_elem_16_conf(x, y, sub_mask, sub_mul); };
            else                       return [sub_mask](auto x, auto y, bool sub_mul) __aie_inline { return ::negmul_elem_32_conf(x, y, sub_mask, sub_mul); };
        }
        if constexpr (MulOp == MulMacroOp::Add_Mul || MulOp == MulMacroOp::Add_MulConj1 || MulOp == MulMacroOp::Add_MulConj1Conj2 || MulOp == MulMacroOp::Add_MulConj2) {
            if constexpr (Elems == 16) return [sub_mask](auto x, auto y, auto acc, bool zero_acc, bool sub_mul, bool sub_acc) __aie_inline { return ::mac_elem_16_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc); };
            else                       return [sub_mask](auto x, auto y, auto acc, bool zero_acc, bool sub_mul, bool sub_acc) __aie_inline { return ::mac_elem_32_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc); };
        }
        if constexpr (MulOp == MulMacroOp::Sub_Mul || MulOp == MulMacroOp::Sub_MulConj1 || MulOp == MulMacroOp::Sub_MulConj1Conj2 || MulOp == MulMacroOp::Sub_MulConj2) {
            if constexpr (Elems == 16) return [sub_mask](auto x, auto y, auto acc, bool zero_acc, bool sub_mul, bool sub_acc) __aie_inline { return ::msc_elem_16_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc); };
            else                       return [sub_mask](auto x, auto y, auto acc, bool zero_acc, bool sub_mul, bool sub_acc) __aie_inline { return ::msc_elem_32_conf(x, y, acc, zero_acc, sub_mask, sub_mul, sub_acc); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, const bool v1_sign,
                                 const vector_type2<Elems> &v2, const bool v2_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        constexpr unsigned native_elems = Elems <= 16? 16 : 32;

        constexpr unsigned num_mul = Elems <= 16? 1 : Elems / native_elems;

        constexpr auto mul_op = get_mul_op<native_elems>();

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<native_elems> tmp = mul_op(v1.template grow_extract<native_elems>(idx),
                                                        v2.template grow_extract<native_elems>(idx),
                                                        utils::get_nth<0>(acc.template grow_extract<native_elems>(idx), acc)...,
                                                        utils::get_nth<0>(zero_acc, acc)...,
                                                        sub_mul,
                                                        utils::get_nth<0>(sub_acc, acc)...);

            ret.insert(idx, tmp.template extract<std::min(Elems, native_elems)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a,                         const bool a_sign,
                                 const vector_type2<Elems> &v, const bool v_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, sub_mul, sub_acc, zero_acc, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, const bool v_sign,
                                 T2 a,                         const bool a_sign,
                                 bool sub_mul, bool sub_acc,
                                 bool zero_acc, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, sub_mul, sub_acc, zero_acc, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, cbfloat16, 32, cbfloat16> : public mul_bits_cbf16_common<MulOp, cbfloat16, cbfloat16> {};
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, cbfloat16, 16,  bfloat16> : public mul_bits_cbf16_common<MulOp, cbfloat16,  bfloat16> {};
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 16,  bfloat16, 32, cbfloat16> : public mul_bits_cbf16_common<MulOp,  bfloat16, cbfloat16> {};
#endif


}
#endif
