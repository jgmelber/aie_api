// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC32_FP_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC32_FP_HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <typename CoeffType, typename DataType>
static constexpr auto get_accfloat_num_complex()
{
    unsigned num_complex = 0;

    if constexpr (is_complex_v<CoeffType>) ++num_complex;
    if constexpr (is_complex_v<DataType>)  ++num_complex;

    return num_complex;
}

template <MulMacroOp MulOp, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_accfloat_get_mul_op()
{
    constexpr unsigned num_complex = get_accfloat_num_complex<CoeffType, DataType>();

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

template <unsigned Lanes>
constexpr unsigned offsets_mask(unsigned offsets)
{
    unsigned mask = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = offsets & 0xf;

        if (val >= 8)
            mask |= 1 << i;

        offsets >>= 4;
    }

    return mask;
}

template <unsigned Lanes>
constexpr unsigned adjust_offsets(unsigned offsets)
{
    unsigned ret = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        unsigned val = offsets & 0xf;

        if (val >= 8)
            val -= 8;

        ret |= val << (i * 4);

        offsets >>= 4;
    }

    return ret;
}

template <unsigned Lanes>
constexpr std::pair<unsigned, unsigned> offsets_range(unsigned offsets)
{
    unsigned min = 16;
    unsigned max = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = offsets & 0xf;

        if (val < min) min = val;
        if (val > max) max = val;

        offsets >>= 4;
    }

    return {min, max};
}

template <unsigned Lanes, int DataStepY>
constexpr unsigned offsets_for_accfloat()
{
    unsigned base = 0;

    if      constexpr (Lanes == 4) base = 0x3210;
    else if constexpr (Lanes == 8) base = 0x76543210;

    unsigned ret = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = ((base & 0xf) * DataStepY) % 16;

        ret |= val << (i * 4);

        base >>= 4;
    }

    return ret;
}

template <unsigned Points, int CoeffStep, int DataStepX, int DataStepY>
struct sliding_mul_bits_impl<8, Points, CoeffStep, DataStepX, DataStepY, 32, 32, 32, float, float>
{
    static constexpr unsigned Lanes = 8;
    using  data_type = float;
    using coeff_type = float;
    using accum_tag  = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned   lanes_per_mul = 8;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
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
        constexpr unsigned offsets = offsets_for_accfloat<Lanes, DataStepY>();

        constexpr auto mac_op = sliding_mul_accfloat_get_mul_op<add_to_op<MulOp>(), coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_accfloat_get_mul_op<             MulOp, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);

        accum_type ret;

        // x indices must be adjusted by "data_start". s represents the step
        //
        // c0 * x0s + c1 * x1s
        ret = mul_op(acc...,
                     data.template grow_replicate<data_elems>(),  data_start,  offsets,
                     coeff.template grow_replicate<8>(),          coeff_start, 0x00000000);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret = mac_op(ret,
                         data.template grow_replicate<data_elems>(),  data_start  + columns_per_mul * idx * DataStepX, offsets,
                         coeff.template grow_replicate<8>(),          coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000);
        });

        return ret;
    }
};

template <unsigned Points, int CoeffStep, int DataStepX, int DataStepY>
struct sliding_mul_bits_impl<4, Points, CoeffStep, DataStepX, DataStepY, 32, 64, 32, cfloat, float>
{
    static constexpr unsigned Lanes = 4;
    using  data_type = float;
    using coeff_type = cfloat;
    using accum_tag  = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned         offsets = offsets_for_accfloat<Lanes, DataStepY>();
    static constexpr unsigned   lanes_per_mul = 4;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
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
        constexpr unsigned offsets = offsets_for_accfloat<Lanes, DataStepY>();

        constexpr auto mac_op = sliding_mul_accfloat_get_mul_op<add_to_op<MulOp>(), coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_accfloat_get_mul_op<             MulOp, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);

        accum_type ret;

        // x indices must be adjusted by "data_start". s represents the step
        //
        // c0 * x0s + c1 * x1s
        ret = mul_op(acc...,
                     data.template grow_replicate<data_elems>(),  data_start,  offsets,
                     coeff.template grow_replicate<4>(),          coeff_start, 0x0000);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret = mac_op(ret,
                         data.template grow_replicate<data_elems>(),  data_start  + columns_per_mul * idx * DataStepX, offsets,
                         coeff.template grow_replicate<4>(),          coeff_start + columns_per_mul * idx * CoeffStep, 0x0000);
        });

        return ret;
    }
};

template <unsigned Points, int CoeffStep, int DataStepX, int DataStepY>
struct sliding_mul_bits_impl<4, Points, CoeffStep, DataStepX, DataStepY, 32, 32, 64, float, cfloat>
{
    static constexpr unsigned Lanes = 4;
    using  data_type = cfloat;
    using coeff_type = float;
    using accum_tag  = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned   lanes_per_mul = 4;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
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
        constexpr unsigned offsets = offsets_for_accfloat<Lanes, DataStepY>();
        constexpr auto range = offsets_range<Lanes>(offsets);

        constexpr auto mac_op = sliding_mul_accfloat_get_mul_op<add_to_op<MulOp>(), coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_accfloat_get_mul_op<             MulOp, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 8u);

        accum_type ret;

        // xoffsets for cfloat can only use 3b values (0-7). For the 16-element case we split the computation between
        // elements with offset in the 0-7 range and elements in the 8-15 range (increasing the start accordingly to get
        // to that range). A select operation is used to obtain the final result.
        if constexpr (N_Data <= 8 || range.second < 8) {
            // x indices must be adjusted by "data_start". s represents the step
            //
            // c0 * x0s + c1 * x1s
            ret = mul_op(acc...,
                         data.template grow_replicate<data_elems>(),  data_start,  adjust_offsets<Lanes>(offsets),
                         coeff.template grow_replicate<8>(),          coeff_start, 0x0000);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                ret = mac_op(ret,
                             data.template grow_replicate<data_elems>(),  data_start  + columns_per_mul * idx * DataStepX, adjust_offsets<Lanes>(offsets),
                             coeff.template grow_replicate<8>(),          coeff_start + columns_per_mul * idx * CoeffStep, 0x0000);
            });

            return ret;
        }
        else {
            vector<cfloat, 8> tmp = ::fpselect8(offsets_mask<Lanes>(offsets), data.template grow_replicate<data_elems>(),
                                                data_start,     adjust_offsets<Lanes>(offsets),
                                                data_start + 8, adjust_offsets<Lanes>(offsets));

            // x indices must be adjusted by "data_start". s represents the step
            //
            // c0 * x0s + c1 * x1s
            ret = mul_op(  acc...,
                           tmp.template grow<data_elems>(),            0, 0x3210,
                         coeff.template grow_replicate<8>(), coeff_start, 0x0000);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                tmp = ::fpselect8(offsets_mask<Lanes>(offsets), data.template grow_replicate<data_elems>(),
                                  data_start + columns_per_mul * idx * DataStepX,     adjust_offsets<Lanes>(offsets),
                                  data_start + columns_per_mul * idx * DataStepX + 8, adjust_offsets<Lanes>(offsets));

                ret = mac_op(  ret,
                               tmp.template grow<data_elems>(),                                                0, 0x3210,
                             coeff.template grow_replicate<8>(), coeff_start + columns_per_mul * idx * CoeffStep, 0x0000);
            });

            return ret;
        }
    }
};

template <unsigned Points, int CoeffStep, int DataStepX, int DataStepY>
struct sliding_mul_bits_impl<4, Points, CoeffStep, DataStepX, DataStepY, 32, 64, 64, cfloat, cfloat>
{
    static constexpr unsigned Lanes = 4;
    using  data_type = cfloat;
    using coeff_type = cfloat;
    using accum_tag  = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned   lanes_per_mul = 4;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
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
        constexpr unsigned offsets = offsets_for_accfloat<Lanes, DataStepY>();
        constexpr auto range = offsets_range<Lanes>(offsets);

        constexpr auto mac_op = sliding_mul_accfloat_get_mul_op<add_to_op<MulOp>(), coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_accfloat_get_mul_op<             MulOp, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 8u);

        accum_type ret;

        // xoffsets for cfloat can only use 3b values (0-7). For the 16-element case we split the computation between
        // elements with offset in the 0-7 range and elements in the 8-15 range (increasing the start accordingly to get
        // to that range). A select operation is used to obtain the final result.
        if constexpr (N_Data <= 8 || range.second < 8) {
            // x indices must be adjusted by "data_start". s represents the step
            //
            // c0 * x0s + c1 * x1s
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  data_start, adjust_offsets<Lanes>(offsets),
                         coeff.template grow_replicate<4>(),          coeff_start, 0x0000);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  data_start + columns_per_mul * idx * DataStepX, adjust_offsets<Lanes>(offsets),
                             coeff.template grow_replicate<4>(),          coeff_start + columns_per_mul * idx * CoeffStep, 0x0000);
            });

            return ret;
        }
        else {
            vector<cfloat, 8> tmp = ::fpselect8(offsets_mask<Lanes>(offsets), data.template grow_replicate<data_elems>(),
                                                data_start,     adjust_offsets<Lanes>(offsets),
                                                data_start + 8, adjust_offsets<Lanes>(offsets));

            // x indices must be adjusted by "data_start". s represents the step
            //
            // c0 * x0s + c1 * x1s
            ret = mul_op(  acc...,
                           tmp.template grow<data_elems>(),            0, 0x3210,
                         coeff.template grow_replicate<4>(), coeff_start, 0x0000);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                tmp = ::fpselect8(offsets_mask<Lanes>(offsets), data.template grow_replicate<data_elems>(),
                                  data_start + columns_per_mul * idx * DataStepX,     adjust_offsets<Lanes>(offsets),
                                  data_start + columns_per_mul * idx * DataStepX + 8, adjust_offsets<Lanes>(offsets));

                ret = mac_op(  ret,
                               tmp.template grow<data_elems>(),                                                0, 0x3210,
                             coeff.template grow_replicate<4>(), coeff_start + columns_per_mul * idx * CoeffStep, 0x0000);
            });

            return ret;
        }
    }
};

}


#endif
