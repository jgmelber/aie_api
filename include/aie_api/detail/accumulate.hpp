// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ACCUMULATE__HPP__
#define __AIE_API_DETAIL_ACCUMULATE__HPP__

#include "accum.hpp"
#include "vector.hpp"
#include "broadcast.hpp"
#include "mul.hpp"

namespace aie::detail {

template <unsigned AccumBits, unsigned Lanes, unsigned CoeffBits, typename T_Coeff, unsigned N_Coeff, unsigned DataBits, typename T_Data, Operation Op, int Step>
struct consume_terms;

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct schedule_1
{
    using     coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using      data_vector_type = vector<T_Data, Lanes>;
    using             accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using            accum_type = accum<accum_tag, Lanes>;
    using consume_terms_type = consume_terms<AccumBits, Lanes, type_bits_v<T_Coeff>, T_Coeff, N_Coeff, type_bits_v<T_Data>, T_Data, Op, Step>;

    template <typename... Terms>
    accum_type run_terms(                       const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v, const Terms & ...terms) const
    {
        const accum_type ret = consume_terms_type::consume_1(coeff, coeff_idx, v);

        if constexpr (sizeof...(terms) > 0)
            return schedule_1<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, Step>::run_terms(ret, coeff, coeff_idx + Step, terms...);
        else
            return ret;
    }

    template <typename... Terms>
    accum_type run_terms(const accum_type &acc, const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v, const Terms & ...terms) const
    {
        const accum_type ret = consume_terms_type::consume_1(coeff, coeff_idx, v, acc);

        if constexpr (sizeof...(terms) > 0)
            return schedule_1<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, Step>::run_terms(ret, coeff, coeff_idx + Step, terms...);
        else
            return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct schedule_2
{
    using     coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using      data_vector_type = vector<T_Data, Lanes>;
    using             accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using            accum_type = accum<accum_tag, Lanes>;
    using consume_terms_type = consume_terms<AccumBits, Lanes, type_bits_v<T_Coeff>, T_Coeff, N_Coeff, type_bits_v<T_Data>, T_Data, Op, Step>;

    static accum_type run_terms(                       const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v)
    {
        return consume_terms_type::consume_1(coeff, coeff_idx, v);
    }

    static accum_type run_terms(const accum_type &acc, const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v)
    {
        return consume_terms_type::consume_1(coeff, coeff_idx, v, acc);
    }

    template <typename... Terms>
    static accum_type run_terms(                       const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v1, const data_vector_type &v2, const Terms & ...terms)
    {
        const accum_type ret = consume_terms_type::consume_2(coeff, coeff_idx, v1, v2);

        if constexpr (sizeof...(terms) > 0)
            return schedule_2<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, Step>::run_terms(ret, coeff, coeff_idx + 2 * Step, terms...);
        else
            return ret;
    }

    template <typename... Terms>
    static accum_type run_terms(const accum_type &acc, const coeff_vector_type &coeff, unsigned coeff_idx, const data_vector_type &v1, const data_vector_type &v2, const Terms & ...terms)
    {
        const accum_type ret = consume_terms_type::consume_2(coeff, coeff_idx, v1, v2, acc);

        if constexpr (sizeof...(terms) > 0)
            return schedule_2<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, Step>::run_terms(ret, coeff, coeff_idx + 2 * Step, terms...);
        else
            return ret;
    }
};

}

#if __AIE_ARCH__ == 10

#include "aie1/accumulate.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/accumulate.hpp"

#endif

namespace aie::detail {

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data>
struct accumulator
{
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    using        accum_type = accum<accum_tag, Lanes>;
    template <Operation Op>
    using     accum_op_type = unary_op<accum_type, Op>;

    template <Operation Op, int CoeffStep, typename... Terms>
    static auto run(const accum_type &acc,
                    const coeff_vector_type &coeff,
                    unsigned coeff_start,
                    const data_vector_type &data,
                    const Terms & ...terms)
    {
        return schedule_2<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, CoeffStep>::run_terms(acc, coeff, coeff_start, data, terms...);
    }

    template <int CoeffStep, typename... Terms>
    static auto run(const coeff_vector_type &coeff,
                    unsigned coeff_start,
                    const data_vector_type &data,
                    const Terms & ...terms)
    {
        constexpr Operation Op = Operation::Acc_Add;
        return schedule_2<AccumBits, Lanes, T_Coeff, N_Coeff, T_Data, Op, CoeffStep>::run_terms(     coeff, coeff_start, data, terms...);
    }
};

}

#endif
