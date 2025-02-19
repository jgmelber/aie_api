// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC32_FP_HPP__
#define __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC32_FP_HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_accfloat_get_mul_op()
{
    if      constexpr (Lanes <= 16) {
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_16(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_16(args...); };
    }
    else if constexpr (Lanes == 32) {
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_32(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_32(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_32(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_32(args...); };
    }
    else if constexpr (Lanes == 64 && std::is_same_v<CoeffType, bfloat16>) {
        if      constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_elem_64(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_elem_64(args...); };
    }
}

template <typename T, unsigned Lanes>
static constexpr unsigned lanes_per_mul_v()
{
    if      constexpr (type_bits_v<T> == 16) {
        if      constexpr (Lanes <= 32) return 32;
        else if constexpr (Lanes == 64) return 64;
    }
    else if constexpr (type_bits_v<T> == 32) {
        if      constexpr (Lanes <= 16) return 16;
        else if constexpr (Lanes == 32) return 32;
    }
}

template <unsigned Lanes, unsigned Points, typename T>
struct sliding_mul_bits_fp_common
{
    using  data_type = T;
    using coeff_type = T;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned   lanes_per_mul = lanes_per_mul_v<T, Lanes>();
    static constexpr unsigned columns_per_mul = 1;
    static_assert(Points >= columns_per_mul);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector<coeff_type, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<data_type, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_accfloat_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_accfloat_get_mul_op<             MulOp, lanes_per_mul, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, lanes_per_mul);

        using accum_fp = decltype(accum_type().template grow_extract<lanes_per_mul>(0));

        accum_type ret;

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            const unsigned data_start_local  = (data_start + idx_y * lanes_per_mul) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            accum_fp tmp;
            vector<data_type, lanes_per_mul> data_local;
            vector<coeff_type, lanes_per_mul> coeff_local;

            coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);
            data_local  = shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                          data_start_local).template extract<lanes_per_mul>(0);

            tmp = mul_op(coeff_local, data_local, (acc.template grow_extract<lanes_per_mul>(idx_y))...);

            utils::unroll_for<unsigned, 1, Points / columns_per_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  = (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);
                data_local = shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                             data_start_local)
                                 .template extract<lanes_per_mul>(0);

                tmp = mac_op(coeff_local, data_local, tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 32, 16, 16, bfloat16, bfloat16> : sliding_mul_bits_fp_common<Lanes, Points, bfloat16> {};

template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 32, 32, 32, float, float> : sliding_mul_bits_fp_common<Lanes, Points, float> {};

} // namespace aie::detail

#endif
