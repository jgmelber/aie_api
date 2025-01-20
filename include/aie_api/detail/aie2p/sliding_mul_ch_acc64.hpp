// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_SLIDING_MUL_CH_ACC64_HPP__
#define __AIE_API_DETAIL_AIE2P_SLIDING_MUL_CH_ACC64_HPP__

#include "../add.hpp"
#include "../mul.hpp"
#include "../vector.hpp"
#include "../shuffle.hpp"
#include "../filter.hpp"
#include "../utils.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, unsigned Channels, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_ch_acc64_get_mul_op()
{
    if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_4x4_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_4x4_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_4x4_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_4x4_8ch(args...); };
}

template <MulMacroOp MulOp, unsigned Lanes, unsigned Channels, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_ch_acc64_get_mul_conf_op()
{
    if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_4x4_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_4x4_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_4x4_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_4x4_8ch_conf(args...); };
}

template <unsigned Outputs, unsigned Channels, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits_impl<Outputs, Channels, Points, 1, 1, 1, 64, 16, 16, CoeffType, DataType>
{
    static constexpr unsigned Lanes = Outputs * Channels;

    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;

    static constexpr unsigned native_channels = 8;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = utils::ceildiv(Points, columns_per_mul);
    static_assert(Channels == native_channels);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          bool coeff_sign,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          bool data_sign,
                          const Acc &... acc)
    {
        constexpr unsigned data_elems  = std::max(N_Data,  64u);
        constexpr unsigned coeff_elems = std::max(N_Coeff, 32u);

        if constexpr (Points % columns_per_mul) {
            vector<coeff_type, 32> coeff2;
            if constexpr (coeff_elems > 32)
                coeff2 = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff, coeff_start * Channels).extract<32>(0);
            else
                coeff2 = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), coeff_start * Channels);
            coeff2 = ::sel(coeff2, zeros<coeff_type, 32>::run(), (~0u) << (Points * Channels));

            return sliding_mul_ch_bits_impl<Outputs, Channels, 32 / Channels, 1, 1, 1, 64, 16, 16, coeff_type, data_type>::template run<MulOp>(
                    coeff2, /*coeff_start*/0, coeff_sign,
                    data,     data_start,     data_sign,
                    acc...);
        }

        constexpr auto mac_op = sliding_mul_ch_acc64_get_mul_op<add_to_op<MulOp>(), Lanes, Channels, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_ch_acc64_get_mul_op<             MulOp, Lanes, Channels, coeff_type, data_type>();

        accum_type ret(acc...);

        // Points = n * columns_per_mul (e.g. 8) will require 2k vectors - CRVO-7596
        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 32)>([&](auto idx_y) __aie_inline {
            accum_type tmp;
            tmp = mul_op(shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                         ((data_start + idx_y * columns_per_mul) * Channels) % data_elems).template extract<64>(0),
                         data_sign,
                         shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), coeff_start * Channels),
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<32>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                tmp = mac_op(shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                             ((data_start + (idx_y + idx) * columns_per_mul) * Channels) % data_elems).template extract<64>(0),
                             data_sign,
                             shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), (coeff_start + columns_per_mul * idx) * Channels),
                             coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= 32)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif

