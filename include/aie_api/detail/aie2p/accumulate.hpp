// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_ACCUMULATE__HPP__
#define __AIE_API_DETAIL_AIE2P_ACCUMULATE__HPP__

#include "emulated_mul_intrinsics.hpp"
#include "mul.hpp"

namespace aie::detail {

consteval MulMacroOp to_mul_op(Operation op, bool hasAccum)
{
    return !hasAccum ? MulMacroOp::Mul : op == Operation::Acc_Sub ? MulMacroOp::Sub_Mul : MulMacroOp::Add_Mul;
}

template <bool WithInputAccum, Operation Op, unsigned AccumBits, unsigned CoeffBits, unsigned DataBits, bool PostAdd = true>
constexpr auto get_mul_op()
{
    if      constexpr (AccumBits == 32) {
        if      constexpr (CoeffBits == 8 && DataBits == 8) {
            if constexpr (PostAdd == true) {
                if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_64_2(params...); };
                else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_64_2(params...); };
                else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_64_2(params...); };
            }
            else {
                if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_64(params...); };
                else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_64(params...); };
                else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_64(params...); };
            }
        }
        if constexpr (CoeffBits == 16 && DataBits == 16) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_64(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_64(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_64(params...); };
        }
    }
    else if constexpr (AccumBits == 64) {
        if      constexpr (CoeffBits == 16 && DataBits == 16) {
            if constexpr (PostAdd == true) {
                if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_32_2(params...); };
                else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_32_2(params...); };
                else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_32_2(params...); };
            }
            else {
                if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_32(params...); };
                else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_32(params...); };
                else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_32(params...); };
            }
        }
        else if constexpr (CoeffBits == 32 && DataBits == 32) {
#if __AIE_API_HAS_32B_MUL__
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_16(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_16(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_16(params...); };
#elif __AIE_API_COMPLEX_VECTOR_SUPPORT__
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_16_t<MulMacroOp::Mul>(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_16_t<MulMacroOp::Add_Mul>(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_16_t<MulMacroOp::Sub_Mul>(params...); };
#endif
        }
        else if constexpr (CoeffBits == 32 || DataBits == 32) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul_elem_16(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac_elem_16(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc_elem_16(params...); };
        }
    }
}

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 4, T_Coeff, N_Coeff, 8, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using        accum_type = accum<accum_tag, Lanes>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 8, utils::get_next_integer_type_t<T_Coeff>, N_Coeff, 8, T_Data, Op, Step>::consume_2(coeff.unpack(), coeff_start, x, y, acc...);
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 8, utils::get_next_integer_type_t<T_Coeff>, N_Coeff, 8, T_Data, Op, Step>::consume_1(coeff.unpack(), coeff_start, v, acc...);
    }
};

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 8, T_Coeff, N_Coeff, 4, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using        accum_type = accum<accum_tag, Lanes>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
         return consume_terms<AccumBits, Lanes, 8, T_Coeff, N_Coeff, 8, utils::get_next_integer_type_t<T_Data>, Op, Step>::consume_2(coeff, coeff_start, x.unpack(), y.unpack(), acc...);
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 8, T_Coeff, N_Coeff, 8, utils::get_next_integer_type_t<T_Data>, Op, Step>::consume_1(coeff, coeff_start, v.unpack(), acc...);
    }
};


template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 8, T_Coeff, N_Coeff, 8, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, AccumBits, 8, 8, true>();
        constexpr unsigned num_mul = Lanes < 128? 1 : Lanes / 64;

        vector<T_Coeff, 128> coeff_v = concat_vector(broadcast<T_Coeff, 64>::run(coeff[coeff_start]),
                                                     broadcast<T_Coeff, 64>::run(coeff[coeff_start + Step]));

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<64> tmp = mul_op( concat_vector(x.template grow_extract<64>(idx), y.template grow_extract<64>(idx)),
                                               coeff_v,
                                               acc.template grow_extract<64>(idx)...);
            ret.insert(idx, tmp.template extract<(Lanes < 64? Lanes : 64)>(0));
        });
#else
        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);
#endif
        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Data, T_Coeff>;

        return mul_op::run(v, v.is_signed(), coeff[coeff_start], coeff.is_signed(), acc...);
    }
};

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 8, T_Coeff, N_Coeff, 16, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using        accum_type = accum<accum_tag, Lanes>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 16, utils::get_next_integer_type_t<T_Coeff>, N_Coeff, 16, T_Data, Op, Step>::consume_2(coeff.unpack(), coeff_start, x, y, acc...);
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 16, utils::get_next_integer_type_t<T_Coeff>, N_Coeff, 16, T_Data, Op, Step>::consume_1(coeff.unpack(), coeff_start, v, acc...);
    }
};

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 16, T_Coeff, N_Coeff, 8, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using        accum_type = accum<accum_tag, Lanes>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 16, T_Coeff, N_Coeff, 16, utils::get_next_integer_type_t<T_Data>, Op, Step>::consume_2(coeff, coeff_start, x.unpack(), y.unpack(), acc...);
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        return consume_terms<AccumBits, Lanes, 16, T_Coeff, N_Coeff, 16, utils::get_next_integer_type_t<T_Data>, Op, Step>::consume_1(coeff, coeff_start, v.unpack(), acc...);
    };
};


template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<32, Lanes, 16,  T_Coeff, N_Coeff, 16,  T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 32;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;
        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);

        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Data, T_Coeff>;

        return mul_op::run(v, v.is_signed(), coeff[coeff_start], coeff.is_signed(), acc...);
    }
};

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<64, Lanes, 16, T_Coeff, N_Coeff, 16, T_Data, Op, Step>
{
    static constexpr unsigned AccumBits = 64;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, AccumBits, 16, 16, true>();
        constexpr unsigned num_mul = Lanes < 64? 1 : Lanes / 32;

        vector<T_Coeff, 64> coeff_v = concat_vector(broadcast<T_Coeff, 32>::run(coeff[coeff_start]),
                                                    broadcast<T_Coeff, 32>::run(coeff[coeff_start + Step]));

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const accum_type<32> tmp = mul_op(concat_vector(x.template grow_extract<32>(idx), y.template grow_extract<32>(idx)),
                                              coeff_v,
                                              acc.template grow_extract<32>(idx)...);
            ret.insert(idx, tmp.template extract<(Lanes < 32? Lanes : 32)>(0));
        });
#else
        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);
#endif
        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Data, T_Coeff>;

        return mul_op::run(v, v.is_signed(), coeff[coeff_start], coeff.is_signed(), acc...);
    }
};

template <unsigned Lanes, typename T_Coeff, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<64, Lanes, 16,  T_Coeff, N_Coeff, 32,  int32, Op, Step>
{
    using           T_Data = int32;
    static constexpr unsigned AccumBits = 64;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;

        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);

        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Coeff, T_Data>;

        accum_type<Lanes> ret = mul_op::run(coeff[coeff_start], coeff.is_signed(), v, v.is_signed(), acc...);

        return ret;
    }
};

template <unsigned Lanes, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<64, Lanes, 32,  int32, N_Coeff, 16,  T_Data, Op, Step>
{
    using            T_Coeff = int32;
    static constexpr unsigned AccumBits = 64;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;

        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);

        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Data, T_Coeff>;

        return mul_op::run(v, v.is_signed(), coeff[coeff_start], coeff.is_signed(), acc...);
    }
};

template <unsigned Lanes, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<64, Lanes, 32,  int32, N_Coeff, 32,  int32, Op, Step>
{
    static constexpr unsigned AccumBits = 64;
    using           T_Data  = int32;
    using           T_Coeff = int32;

    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;

    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, const Acc &... acc)
    {
        accum_type<Lanes> ret;

        ret = consume_1(coeff, coeff_start,        x, acc...);
        ret = consume_1(coeff, coeff_start + Step, y, ret);

        return ret;
    }

    template <typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, const Acc &... acc)
    {
        constexpr MulMacroOp op = to_mul_op(Op, sizeof...(acc) > 0);
        using mul_op = mul<op, AccumBits, T_Data, T_Coeff>;

        return mul_op::run(v, v.is_signed(), coeff[coeff_start], coeff.is_signed(), acc...);
    }
};

}

#endif
