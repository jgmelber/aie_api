// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MUL_ACC32_FP__HPP__
#define __AIE_API_DETAIL_AIE2P_MUL_ACC32_FP__HPP__

#include "../utils.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
    requires(std::is_same_v<T1, bfloat16> && std::is_same_v<T2, bfloat16>)
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
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_64(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_64(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_64(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_64(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned muls_per_mul = Elems <= 32? 32 : 64;

        constexpr unsigned num_mul = Elems < muls_per_mul? 1 : Elems / muls_per_mul;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<muls_per_mul> tmp = mul_op(v1.template grow_extract<muls_per_mul>(idx),
                                                        v1_sign,
                                                        v2.template grow_extract<muls_per_mul>(idx),
                                                        v2_sign,
                                                        acc.template grow_extract<muls_per_mul>(idx)...);

            ret.insert(idx, tmp.template extract<(Elems < muls_per_mul? Elems : muls_per_mul)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v.template grow<Elems>(), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return run(v.template grow<Elems>(), v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
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
        if constexpr (Elems <= 16) {
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
        }
        else {
            if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned muls_per_mul = Elems <= 16 ? 16 : 32;

        constexpr unsigned num_mul = Elems < muls_per_mul ? 1 : Elems / muls_per_mul;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<muls_per_mul> tmp = mul_op(v1.template grow_extract<muls_per_mul>(idx),
                                                        v2.template grow_extract<muls_per_mul>(idx),
                                                        acc.template grow_extract<muls_per_mul>(idx)...);

            ret.insert(idx, tmp.template extract<(Elems < muls_per_mul? Elems : muls_per_mul)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T, Elems>::run(a), a_sign, v.template grow<Elems>(), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, T a, bool a_sign, const Acc &... acc)
    {
        return run(v.template grow<Elems>(), v_sign, broadcast<T, Elems>::run(a), a_sign, acc...);
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
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_8(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_8(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_8(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_8(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector<T1, Elems> &v1, const bool v1_sign, const vector<T2, Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned basic_elems = 8;
        constexpr unsigned num_mul = Elems < basic_elems? 1 : Elems / basic_elems;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<basic_elems> tmp = mul_op( v1.template grow_extract<basic_elems>(idx),
                                                        v2.template grow_extract<basic_elems>(idx),
                                                       acc.template grow_extract<basic_elems>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, basic_elems)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, const bool a_sign, const vector<T2, Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector<T1, Elems> &v, const bool v_sign, T2 a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
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
