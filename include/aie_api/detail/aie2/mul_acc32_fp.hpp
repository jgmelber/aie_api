// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MUL_ACC32_FP__HPP__
#define __AIE_API_DETAIL_AIE2_MUL_ACC32_FP__HPP__

namespace aie::detail {

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 16, bfloat16, 16, bfloat16>
{
    using T = bfloat16;

    template <unsigned Elems>
    using vector_type = vector<bfloat16, Elems>;
    template <unsigned Elems>
    using  accum_type = accum<accfloat, Elems>;

    template <unsigned Elems>
    static constexpr auto get_mul_op()
    {
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_16_2(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16_2(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_16_2(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_16_2(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, bool v1_sign, const vector_type<Elems> &v2, bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        const vector_type<16> z = zeros<bfloat16, 16>::run();

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector_type<32> v1_ = v1.template grow_extract<16>(idx)
                                          .template grow<32>()
                                          .template insert(1, z);
            const vector_type<32> v2_ = v2.template grow_extract<16>(idx)
                                          .template grow<32>()
                                          .template insert(1, z);
            const accum_type<16> tmp = mul_op(v1_, v2_, acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 16u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T a, bool a_sign, const vector_type<Elems> &v, bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, bool v_sign, T a, bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T, Elems>::run(a), a_sign, acc...);
    }
};

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
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v1, const bool v1_sign, const vector_type<Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<16> tmp = mul_op(v1.template grow_extract<16>(idx),
                                              v2.template grow_extract<16>(idx),
                                              acc.template grow_extract<16>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 16u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T a, const bool a_sign, const vector_type<Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, const bool v_sign, T a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T, Elems>::run(a), a_sign, acc...);
    }
};

#if __AIE_API_COMPLEX_FP32_EMULATION__

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
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_8_2(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_8_2(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_8_2(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_8_2(args...); };
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v1, const bool v1_sign, const vector_type2<Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        const auto z1 = zeros<T1, 8>::run();
        const auto z2 = zeros<T2, 8>::run();

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector_type1<16> v1_ = v1.template grow_extract<8>(idx)
                                           .template grow<16>()
                                           .template insert(1, z1);
            const vector_type2<16> v2_ = v2.template grow_extract<8>(idx)
                                           .template grow<16>()
                                           .template insert(1, z2);
            const accum_type<8> tmp = mul_op(v1_, v2_, acc.template grow_extract<8>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 8u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, const bool a_sign, const vector_type2<Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, const bool v_sign, T2 a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, cbfloat16, 32, cbfloat16> : public mul_bits_cbf16_common<MulOp, cbfloat16, cbfloat16> {};
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, cbfloat16, 16,  bfloat16> : public mul_bits_cbf16_common<MulOp, cbfloat16,  bfloat16> {};
template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 16,  bfloat16, 32, cbfloat16> : public mul_bits_cbf16_common<MulOp,  bfloat16, cbfloat16> {};
#endif

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 64, cfloat>
{
    using T = cfloat;

    template <unsigned Elems>
    using vector_type = vector<cfloat, Elems>;
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
    static accum_type<Elems> run(const vector_type<Elems> &v1, const bool v1_sign, const vector_type<Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op( v1.template grow_extract<8>(idx),
                                              v2.template grow_extract<8>(idx),
                                             acc.template grow_extract<8>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 8u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T a, const bool a_sign, const vector_type<Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type<Elems> &v, const bool v_sign, T a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T, Elems>::run(a), a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 64, cfloat, 32, float>
{
    using T1 = cfloat;
    using T2 = float;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
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
    static accum_type<Elems> run(const vector_type1<Elems> &v1, const bool v1_sign, const vector_type2<Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op( v1.template grow_extract<8>(idx),
                                              v2.template grow_extract<8>(idx),
                                             acc.template grow_extract<8>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 8u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, const bool a_sign, const vector_type2<Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, const bool v_sign, T2 a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
    }
};

template <MulMacroOp MulOp>
struct mul_bits_impl<MulOp, 32, 32, float, 64, cfloat>
{
    using T1 = float;
    using T2 = cfloat;

    template <unsigned Elems>
    using vector_type1 = vector<T1, Elems>;
    template <unsigned Elems>
    using vector_type2 = vector<T2, Elems>;
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
    static accum_type<Elems> run(const vector_type1<Elems> &v1, const bool v1_sign, const vector_type2<Elems> &v2, const bool v2_sign, const Acc &... acc)
    {
        constexpr auto mul_op = get_mul_op<Elems>();

        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        accum_type<Elems> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<8> tmp = mul_op( v1.template grow_extract<8>(idx),
                                              v2.template grow_extract<8>(idx),
                                             acc.template grow_extract<8>(idx)...);
            ret.insert(idx, tmp.template extract<std::min(Elems, 8u)>(0));
        });

        return ret;
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(T1 a, const bool a_sign, const vector_type2<Elems> &v, const bool v_sign, const Acc &... acc)
    {
        return run(broadcast<T1, Elems>::run(a), a_sign, v, v_sign, acc...);
    }

    template <unsigned Elems, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type<Elems> run(const vector_type1<Elems> &v, const bool v_sign, T2 a, const bool a_sign, const Acc &... acc)
    {
        return run(v, v_sign, broadcast<T2, Elems>::run(a), a_sign, acc...);
    }
};

#endif

}
#endif
