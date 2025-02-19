// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_MUL_ACC64__HPP__
#define __AIE_API_DETAIL_AIE2P_MUL_ACC64__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../conj.hpp"
#include "../interleave.hpp"

#include "emulated_mul_intrinsics.hpp"

namespace aie::detail {

// 16b * 8b
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 64, 16, T1, 8, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, T1, utils::get_next_integer_type_t<T2>>::run(v1, v1_sign, v2.unpack_sign(v2_sign), v2_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, T1, utils::get_next_integer_type_t<T2>>::run(a, a_sign, v.unpack_sign(v_sign), v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, T1, utils::get_next_integer_type_t<T2>>::run(v, v_sign, a, a_sign, acc...);
    }
};

// 8b * 16b
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 64, 8, T1, 16, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, utils::get_next_integer_type_t<T1>, T2>::run(v1.unpack_sign(v1_sign), v1_sign, v2, v2_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, utils::get_next_integer_type_t<T1>, T2>::run(a, a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type2<Elems> &v, bool v_sign, T1 a, bool a_sign, const Acc &... acc)
    {
        return mul<MulOp, 64, utils::get_next_integer_type_t<T1>, T2>::run(v.unpack_sign(v_sign), v_sign, a, a_sign, acc...);
    }
};

// 16b * 16b
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 64, 16, T1, 16, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    static constexpr unsigned native_num_mul = 32;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<32> tmp = mul_op( v1.template grow_extract<32>(idx),
                                               v1_sign,
                                               v2.template grow_extract<32>(idx),
                                               v2_sign,
                                              acc.template grow_extract<32>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

// 16b * 16b
template <MulMacroOp MulOp, typename T2>
struct mul_bits_impl<MulOp, 64, 32, cint16, 16, T2>
{
    using T1 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        const vector<T2, 32> z = zeros<T2, 32>::run();

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<cint16, 16> v2_complex = interleave_zip<int16, 32>::run(v2.template grow_extract<16>(idx).template grow<32>(),
                                                                                 z, 1).first.template cast_to<cint16>();

            const accum_type<16> tmp = mul_op(v1.template grow_extract<16>(idx),
                                              v1_sign,
                                              v2_complex,
                                              v2_sign,
                                              acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp, typename T1>
struct mul_bits_impl<MulOp, 64, 16, T1, 32, cint16>
{
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T2, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0,0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        const vector<T1, 32> z = zeros<T1, 32>::run();

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<cint16, 16> v1_complex = interleave_zip<int16, 32>::run(v1.template grow_extract<16>(idx).template grow<32>(),
                                                                                 z, 1).first.template cast_to<cint16>();

            const accum_type<16> tmp = mul_op( v1_complex,
                                               v1_sign,
                                               v2.template grow_extract<16>(idx),
                                               v2_sign,
                                              acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 32, cint16, 32, cint16>
{
    using T1 = cint16;
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T2, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems, bool reverseOperands = false>
    static constexpr auto get_mul_op()
    {
        constexpr MulMacroOp op = reverseOperands? swap_conjugate_order<MulOp>() : MulOp;
        if constexpr (op == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        if constexpr (op == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        if constexpr (op == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        if constexpr (op == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
        if constexpr (op == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (op == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (op == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (op == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (op == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (op == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (op == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (op == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (op == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
        if constexpr (op == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (op == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (op == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op( v1.template grow_extract<16>(idx),
                                               v1_sign,
                                               v2.template grow_extract<16>(idx),
                                               v2_sign,
                                              acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

// 32b * 16b
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 64, 32, T1, 16, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    static constexpr unsigned native_num_mul = 32;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
#if __AIE_API_HAS_32B_MUL__
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
#else
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32_t(args...); };
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<32> tmp = mul_op( v1.template grow_extract<16>(idx * 2),
                                               Elems > 16? v1.template grow_extract<16>(idx * 2 + 1) : vector_type1<16>(),
                                               v1_sign,
                                               v2.template grow_extract<32>(idx),
                                               v2_sign,
                                              acc.template grow_extract<32>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 32, int32, 32, cint16>
{
    using T1 = int32;
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using   accum_tag = accum_tag_for_mul_types<T1, T2, 64>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    using mul_bits_impl_complex = mul_bits_impl<MulOp, 64, 64, cint32, 32, cint16>;
    using mul_bits_impl_real    = mul_bits_impl<MulOp, 64, 32,  int32, 16,  int16>;

    static constexpr unsigned native_num_mul = has_conj2<MulOp>()? mul_bits_impl_complex::native_num_mul :
                                                                   mul_bits_impl_real::native_num_mul;

    static constexpr unsigned max_t1_elems = 512 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 512 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        accum_type<Elems> res;

        // split v1 to prevent vectors growing larger than 1k
        constexpr unsigned num_muls      = std::max(1u, vector_type1<Elems>::bits() / 512);
        constexpr unsigned elems_per_mul = std::min(Elems, 512 / vector_type1<Elems>::type_bits());

        utils::unroll_times<num_muls>([&](unsigned idx) __aie_inline {
            if constexpr (has_conj2<MulOp>()) {
                auto [tmp1, tmp2] = interleave_zip<T1, elems_per_mul>::run(v1.template extract<elems_per_mul>(idx),
                                                                           zeros<int32, elems_per_mul>::run(),
                                                                           1);

                vector_type2<elems_per_mul> tmp = tmp1.template grow<elems_per_mul * 2>(0)
                                                      .insert(1, tmp2)
                                                      .template cast_to<T2>();

                res.insert(idx, mul_bits_impl_complex::run(tmp,                                     v1_sign,
                                                           v2.template extract<elems_per_mul>(idx), v2_sign,
                                                           acc.template extract<elems_per_mul>(idx)...));

            }
            else {
                auto [tmp1, tmp2] = interleave_zip<T1, elems_per_mul>::run(v1.template extract<elems_per_mul>(idx),
                                                                           v1.template extract<elems_per_mul>(idx),
                                                                           1);

                vector_type1<elems_per_mul * 2> tmp = tmp1.template grow<elems_per_mul * 2>(0)
                                                          .insert(1, tmp2);

                res.insert(idx, mul_bits_impl_real::run(tmp,                                    v1_sign,
                                                        v2.template extract<elems_per_mul>(idx)
                                                          .template cast_to<int16>(),           v2_sign,
                                                        acc.template extract<elems_per_mul>(idx)
                                                           .template cast_to<acc64>()...).template cast_to<cacc64>());
            }
        });

        return res;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc.template grow<elems>()...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc.template grow<elems>()...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 64, cint32, 32, cint16>
{
    using T1 = cint32;
    using T2 = cint16;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T2, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_conf(args..., OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, 0, 0); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_conf(args..., 0, 0, OP_TERM_NEG_COMPLEX_CONJUGATE_Y, 0, 0); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op( v1.template grow_extract<16>(idx),
                                               v1_sign,
                                               v2.template grow_extract<16>(idx),
                                               v2_sign,
                                              acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 64, cint32, 16, int16>
{
    using T1 = cint32;
    using T2 =  int16;

    using base_impl = mul_bits_impl<MulOp, 64, 64, cint32, 32, cint16>;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_type<T1, 64>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        auto [v2_tmp1, v2_tmp2] = interleave_zip<T2, Elems>::run(v2, zeros<T2, Elems>::run(), 1);
        auto v2_tmp = v2_tmp1.template grow<Elems * 2>().template insert(1, v2_tmp2);

        return base_impl::run(v1,                                v1_sign,
                              v2_tmp.template cast_to<cint16>(), v2_sign,
                              acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        auto a_tmp = broadcast<cint16, elems>::run(cint16(a, 0));

        return base_impl::run(v.template grow<elems>(), v_sign,
                              a_tmp,                    a_sign,
                              acc...).template extract<Elems>(0);
    }
};

// 16b * 32b
template <MulMacroOp MulOp, typename T1, typename T2> requires(!is_complex_v<T2>)
struct mul_bits_impl<MulOp, 64, 16, T1, 32, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    static constexpr unsigned native_num_mul = 32;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
#if __AIE_API_HAS_32B_MUL__
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
#else
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32_t(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32_t(args...); };
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<32> tmp = mul_op( v2.template grow_extract<16>(idx * 2),
                                               Elems > 16? v2.template grow_extract<16>(idx * 2 + 1) : vector_type2<16>(),
                                               v2_sign,
                                               v1.template grow_extract<32>(idx),
                                               v1_sign,
                                              acc.template grow_extract<32>(idx)...);
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 32, cint16, 32, int32>
{
    using T1 = cint16;
    using T2 = int32;

    using base_impl = mul_bits_impl<swap_conjugate_order<MulOp>(), 64, 32, int32, 32, cint16>;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;

    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using    accum_tag = accum_tag_for_mul_types<T1, T2, 64>;

    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return base_impl::run(v2, v2_sign, v1, v1_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return base_impl::run(v, v_sign, a, a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return base_impl::run(a, a_sign, v, v_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 16, int16, 64, cint32>
{
    using T1 =  int16;
    using T2 = cint32;

    using base_impl = mul_bits_impl<swap_conjugate_order<MulOp>(), 64, 64, cint32, 16, int16>;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;

    using accum_tag = accum_tag_for_type<T2, 64>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag, Elems>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return base_impl::run(v2, v2_sign, v1, v1_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return base_impl::run(v, v_sign, a, a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return base_impl::run(a, a_sign, v, v_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 32, cint16, 64, cint32>
{
    using T1 = cint16;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T2, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    using base_impl = mul_bits_impl<swap_conjugate_order<MulOp>(), 64, 64, cint32, 32, cint16>;

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        return base_impl::run(v2, v2_sign, v1, v1_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return base_impl::run(v, v_sign, a, a_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        return base_impl::run(a, a_sign, v, v_sign, acc...);
    }
};

// 32b * 32b
template <MulMacroOp MulOp, typename T1, typename T2>
struct mul_bits_impl<MulOp, 64, 32, T1, 32, T2>
{
    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_mul_types<T1, T2, 64>, Elems>;

    static constexpr unsigned native_num_mul = 32;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        accum_type<32> ret;

        if constexpr (Elems < 32) {
            ret = mul_op( v1.template grow<16>(),
                          vector_type1<16>(),
                          v1_sign,
                          v2.template grow<16>(),
                          vector_type2<16>(),
                          v2_sign,
                         acc.template grow<32>()...);
        }
        else {
            ret = mul_op( v1.template extract<16>(0),
                          v1.template extract<16>(1),
                          v1_sign,
                          v2.template extract<16>(0),
                          v2.template extract<16>(1),
                          v2_sign,
                         acc.template grow<32>()...);
        }

        return ret.template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 64, cint32, 32, int32>
{
    using T1 = cint32;
    using T2 = int32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<cacc64, Elems>;

    using mul_bits_impl_complex = mul_bits_impl<MulOp, 64, 64, cint32, 64, cint32>;
    using mul_bits_impl_real    = mul_bits_impl<MulOp, 64, 32, int32, 32, int32>;

    static constexpr unsigned native_num_mul = has_conj1<MulOp>()? mul_bits_impl_complex::native_num_mul :
                                                                   mul_bits_impl_real::native_num_mul;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        if constexpr (has_conj1<MulOp>()) {
            auto [tmp1, tmp2] = interleave_zip<T2, Elems>::run(v2, zeros<int32, Elems>::run(), 1);

            vector_type1<Elems> tmp = tmp1.template grow<Elems * 2>().insert(1, tmp2).template cast_to<T1>();

            return mul_bits_impl_complex::run(v1, v1_sign, tmp, v2_sign, acc...);

        }
        else {
            auto [tmp1, tmp2] = interleave_zip<T2, Elems>::run(v2, v2, 1);

            vector_type2<Elems * 2> tmp = tmp1.template grow<Elems * 2>().insert(1, tmp2);

            auto result = mul_bits_impl_real::run(v1.template cast_to<int32>(), v1_sign, tmp, v2_sign, acc.template cast_to<acc64>()...);
            return result.template cast_to<cacc64>();
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 32, int32, 64, cint32>
{
    using T1 = int32;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T2, 64>, Elems>;

    using mul_bits_impl_complex = mul_bits_impl<MulOp, 64, 64, cint32, 64, cint32>;
    using mul_bits_impl_real    = mul_bits_impl<MulOp, 64, 32, int32, 32, int32>;

    static constexpr unsigned native_num_mul = has_conj2<MulOp>()? mul_bits_impl_complex::native_num_mul :
                                                                   mul_bits_impl_real::native_num_mul;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        if constexpr (has_conj2<MulOp>()) {
            auto [tmp1, tmp2] = interleave_zip<T1, Elems>::run(v1, zeros<int32, Elems>::run(), 1);

            vector_type2<Elems> tmp = tmp1.template grow<Elems * 2>().insert(1, tmp2).template cast_to<T2>();

            return mul_bits_impl_complex::run(tmp, v1_sign, v2, v2_sign, acc...);

        }
        else {
            auto [tmp1, tmp2] = interleave_zip<T1, Elems>::run(v1, v1, 1);

            vector_type1<Elems * 2> tmp = tmp1.template grow<Elems * 2>().insert(1, tmp2);

            return mul_bits_impl_real::run(tmp, v1_sign, v2.template cast_to<int32>(), v2_sign, acc.template cast_to<acc64>()...).template cast_to<cacc64>();
        }
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 64, 64, cint32, 64, cint32>
{
    using T1 = cint32;
    using T2 = cint32;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accum_tag_for_type<T1, 64>, Elems>;

    static constexpr unsigned native_num_mul = 16;

    static constexpr unsigned max_t1_elems = 1024 / type_bits_v<T1>;
    static constexpr unsigned max_t2_elems = 1024 / type_bits_v<T2>;

    static constexpr unsigned max_elems     = std::min(max_t1_elems, max_t2_elems);
    static constexpr unsigned default_elems = std::min(native_num_mul, max_elems);

    static constexpr auto get_mul_op()
    {
#if __AIE_API_HAS_32B_MUL__
        if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
#else
        if constexpr (MulOp == MulMacroOp::Mul)               return [](auto &&... args) __aie_inline { return ::mul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)            return [](auto &&... args) __aie_inline { return ::negmul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)           return [](auto &&... args) __aie_inline { return ::mac_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)           return [](auto &&... args) __aie_inline { return ::msc_elem_16_t<MulOp>(args...); };
#endif

#if __AIE_API_MUL_CONJUGATE_32BIT_INTRINSICS__
        if constexpr (MulOp == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_cn(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_cc(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_nc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_cn(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_cc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_nc(args...); };
#else
        if constexpr (MulOp == MulMacroOp::MulConj1)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2)          return [](auto &&... args) __aie_inline { return ::mul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2)       return [](auto &&... args) __aie_inline { return ::negmul_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::mac_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2)      return [](auto &&... args) __aie_inline { return ::mac_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2) return [](auto &&... args) __aie_inline { return ::msc_elem_16_t<MulOp>(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2)      return [](auto &&... args) __aie_inline { return ::msc_elem_16_t<MulOp>(args...); };
#endif
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, bool v1_sign, const vector_type2<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op();

        accum_type<16> ret;

        if constexpr (Elems < 16) {
            ret = mul_op( v1.template grow<16>(),
                          v2.template grow<8>(),
                          vector_type2<8>(),
                         acc.template grow<16>()...);
        }
        else {
            ret = mul_op( v1.template grow<16>(),
                          v2.template extract<8>(0),
                          v2.template extract<8>(1),
                         acc.template grow<16>()...);
        }

        return ret.template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, bool a_sign, const vector_type2<Elems> &v, bool v_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(broadcast<T1, elems>::run(a), a_sign, v.template grow<elems>(), v_sign, acc...).template extract<Elems>(0);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, bool v_sign, T2 a, bool a_sign, const Acc &... acc)
    {
        constexpr unsigned elems = std::max(Elems, default_elems);
        return run(v.template grow<elems>(), v_sign, broadcast<T2, elems>::run(a), a_sign, acc...).template extract<Elems>(0);
    }
};

}

#endif
