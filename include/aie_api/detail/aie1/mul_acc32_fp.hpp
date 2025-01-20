// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MUL_ACC32__HPP__
#define __AIE_API_DETAIL_AIE1_MUL_ACC32__HPP__

#include <algorithm>

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
static constexpr auto mul_accfloat_get_mul_op()
{
    constexpr unsigned num_complex = []() constexpr {
        unsigned ret = 0;

        if constexpr (is_complex_v<T1>) ++ret;
        if constexpr (is_complex_v<T2>) ++ret;

        return ret;
    }();

    if      constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::fpmul(args...); };
    else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::fpmul_c(args...); };
    else if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::fpmul_cn(args...); };
    else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::fpmul_c(args...); };
    else if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::fpmul_nc(args...); };
    else if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::fpmul_cc(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::fpneg_mul(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::fpneg_mul_c(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::fpneg_mul_cn(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::fpneg_mul_c(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::fpneg_mul_nc(args...); };
    else if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::fpneg_mul_cc(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::fpmac(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::fpmac_c(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::fpmac_cn(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::fpmac_c(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::fpmac_nc(args...); };
    else if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::fpmac_cc(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::fpmsc(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::fpmsc_c(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::fpmsc_cn(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::fpmsc_c(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::fpmsc_nc(args...); };
    else if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::fpmsc_cc(args...); };
}

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, float, 32, float>
{
    using T1 = float;
    using T2 = float;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    template <unsigned Elems>
    using  accum_type = accum<accfloat, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                       v1.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210,
                                       v2.template grow_extract<8>(idx),                  0, 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
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
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                       v.template grow_extract<16u>(idx / 2), 8 * (idx % 2), 0x76543210,
                                       subv_parent,                            local_offset, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(float a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<float, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, float a, bool a_sign, const Acc &... acc)
    {
        const vector<float, 16> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 32, float>
{
    using T1 = cfloat;
    using T2 =  float;

    template <unsigned Elems>
    using vector1_type = vector<T1, Elems>;

    template <unsigned Elems>
    using vector2_type = vector<T2, Elems>;

    template <unsigned Elems>
    using  accum_type = accum<caccfloat, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v1, bool v1_sign, const vector2_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               v1.template grow_extract<8>(idx / 2), 4 * (idx % 2), 0x3210,
                                               v2.template grow_extract<8>(idx / 2), 4 * (idx % 2), 0x3210));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               a.parent.template grow<std::max(ElemsRef, 8u)>(),      a.offset, 0x0000,
                                               v.template grow_extract<8>(idx / 2),              4 * (idx % 2), 0x3210));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        const unsigned subv_idx = a.offset / 8;
        const unsigned local_offset = a.offset % 8;

        const vector<T2, 8> subv_parent = a.parent.template grow_extract<8>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               v.template grow_extract<8u>(idx / 2), 4 * (idx % 2), 0x3210,
                                               subv_parent,                           local_offset, 0x0000));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(cfloat a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<cfloat, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, float a, bool a_sign, const Acc &... acc)
    {
        const vector<float, 16> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, float, 64, cfloat>
{
    using T1 =  float;
    using T2 = cfloat;

    template <unsigned Elems>
    using vector1_type = vector<T1, Elems>;

    template <unsigned Elems>
    using vector2_type = vector<T2, Elems>;

    template <unsigned Elems>
    using  accum_type = accum<caccfloat, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v1, bool v1_sign, const vector2_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               v1.template grow<16>(),           4 * (idx % 4), 0x3210,
                                               v2.template grow_extract<4>(idx),             0, 0x3210));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               a.parent.template grow<std::max(ElemsRef, 16u)>(), a.offset, 0x0000,
                                               v.template grow_extract<4>(idx),                          0, 0x3210));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, vector_elem_const_ref<T2, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();

        const unsigned subv_idx = a.offset / 4;
        const unsigned local_offset = a.offset % 4;

        const vector<T2, 4> subv_parent = a.parent.template grow<std::max(ElemsRef, 4u)>().template extract<4>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<Elems / 4>([&](auto idx) __aie_inline {
            ret.template insert<4>(idx, mul_op(acc.template grow_extract<4>(idx)...,
                                               v.template grow_extract<16u>(idx / 4), 4 * (idx % 4), 0x3210,
                                               subv_parent,                            local_offset, 0x0000));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(float a, bool a_sign, const vector2_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<float, 16> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector1_type<Elems> &v, bool v_sign, cfloat a, bool a_sign, const Acc &... acc)
    {
        const vector<cfloat, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 64, cfloat>
{
    using T1 = cfloat;
    using T2 = cfloat;

    template <unsigned Elems>
    using vector_type = vector<T1, Elems>;

    template <unsigned Elems>
    using  accum_type = accum<caccfloat, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 4? 1 : Elems / 4;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template grow_extract<4>(idx)...,
                                       v1.template grow_extract<8>(idx / 2), 4 * (idx % 2), 0x3210,
                                       v2.template grow_extract<4>(idx),                 0, 0x3210);
            ret.insert(idx, tmp.template extract<(Elems < 4? Elems : 4)>(0));
        });

        return ret;
    }

    template <unsigned ElemsRef, unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 4? 1 : Elems / 4;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template grow_extract<4>(idx)...,
                                       a.parent.template grow<std::max(ElemsRef, 8u)>(), a.offset, 0x0000,
                                       v.template grow_extract<4>(idx),                         0, 0x3210);
            ret.insert(idx, tmp.template extract<(Elems < 4? Elems : 4)>(0));
        });

        return ret;
    }

    template <unsigned Elems, unsigned ElemsRef, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, vector_elem_const_ref<T1, ElemsRef> a, bool a_sign, const Acc &... acc)
    {
        constexpr auto mul_op = mul_accfloat_get_mul_op<MulOp, T1, T2>();
        constexpr unsigned num_mul = Elems < 4? 1 : Elems / 4;

        const unsigned subv_idx = a.offset / 4;
        const unsigned local_offset = a.offset % 4;

        const vector<T2, 4> subv_parent = a.parent.template grow<std::max(ElemsRef, 4u)>().template extract<4>(subv_idx);

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template grow_extract<4>(idx)...,
                                       v.template grow_extract<8>(idx / 2), 4 * (idx % 2), 0x3210,
                                       subv_parent,                          local_offset, 0x0000);
            ret.insert(idx, tmp.template extract<(Elems < 4? Elems : 4)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(cfloat a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        const vector<cfloat, 8> tmp(a);

        return run(tmp[0], a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, cfloat a, bool a_sign, const Acc &... acc)
    {
        const vector<cfloat, 8> tmp(a);

        return run(v, v_sign, tmp[0], a_sign, acc...);
    }
};

}

#endif
