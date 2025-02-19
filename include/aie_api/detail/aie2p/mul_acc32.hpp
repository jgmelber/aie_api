// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MUL_ACC32__HPP__
#define __AIE_API_DETAIL_AIE2P_MUL_ACC32__HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 32, 8, T1, 4, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 32>, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(v1, v1_sign, v2.unpack(), v2_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(a, a_sign, v.unpack(), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(v, v_sign, a, a_sign, acc...);
    }
};

template <MulMacroOp MulOp, unsigned Type1Bits, typename T1, unsigned Type2Bits, typename T2> requires((Type1Bits >= 8) && (Type1Bits == Type2Bits))
struct mul_bits_impl<MulOp, 32, Type1Bits, T1, Type2Bits, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 32>, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_64(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 64? 1 : Elems / 64;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<64> tmp = mul_op( v1.template grow_extract<64>(idx),
                                               v1_sign,
                                               v2.template grow_extract<64>(idx),
                                               v2_sign,
                                              acc.template grow_extract<64>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 64? Elems : 64)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
    }
};

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 32, 16, T1, 8, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 32>, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(v1, v1_sign, v2.unpack(), v2_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(a, a_sign, v.unpack(), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, T1, utils::get_next_integer_type_t<T2>>::run(v, v_sign, a, a_sign, acc...);
    }
};

template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 32, 8, T1, 16, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 32>, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, utils::get_next_integer_type_t<T1>, T2>::run(v1.unpack(), v1_sign, v2, v2_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, utils::get_next_integer_type_t<T1>, T2>::run(a, a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 32, utils::get_next_integer_type_t<T1>, T2>::run(v.unpack(), v_sign, a, a_sign, acc...);
    }
};

}

#endif
