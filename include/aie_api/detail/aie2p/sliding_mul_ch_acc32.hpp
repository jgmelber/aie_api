// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_SLIDING_MUL_CH_ACC32_HPP__
#define __AIE_API_DETAIL_AIE2P_SLIDING_MUL_CH_ACC32_HPP__

#include "../add.hpp"
#include "../mul.hpp"
#include "../vector.hpp"
#include "../shuffle.hpp"
#include "../filter.hpp"
#include "../utils.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, unsigned Channels, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_ch_acc32_get_mul_op()
{
    if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_8x8_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_8x8_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_8x8_8ch(args...); };
    if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_8x8_8ch(args...); };
}

template <MulMacroOp MulOp, unsigned Lanes, unsigned Channels, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_ch_acc32_get_mul_conf_op()
{
    if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_8x8_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_8x8_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_8x8_8ch_conf(args...); };
    if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_8x8_8ch_conf(args...); };
}

template <unsigned Outputs, unsigned Channels, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_ch_bits_impl<Outputs, Channels, Points, 1, 1, 1, 32, 8, 8, CoeffType, DataType>
{
    static constexpr unsigned Lanes = Outputs * Channels;

    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 32>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type64 = accum<accum_tag, 64>;

    static constexpr unsigned native_channels = 8;
    static constexpr unsigned columns_per_mul = 8;
    static constexpr unsigned         num_mul = utils::ceildiv(Points, columns_per_mul);
    static_assert((Channels == native_channels) || (Channels == (native_channels / 2)));

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
        constexpr unsigned data_elems  = std::max(N_Data,  128u);
        constexpr unsigned coeff_elems = std::max(N_Coeff, 64u);

        if constexpr (Points % columns_per_mul) {
            vector<coeff_type, 64> coeff2;
            if constexpr (coeff_elems > 64)
                coeff2 = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff, coeff_start * Channels).extract<64>(0);
            else
                coeff2 = shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), coeff_start * Channels);
            coeff2 = ::sel(coeff2, zeros<coeff_type, 64>::run(), (~0ull) << (Points * Channels));

            return sliding_mul_ch_bits_impl<Outputs, Channels, 64 / Channels, 1, 1, 1, 32, 8, 8, coeff_type, data_type>::template run<MulOp>(
                    coeff2, /*coeff_start*/0, coeff_sign,
                    data,     data_start,     data_sign,
                    acc...);
        }
        else if constexpr (Channels < native_channels) {
            auto [data_lo, data_hi] = interleave_zip<DataType, N_Data>::run(data, data, Channels);
            auto data2 = data_lo.template grow<N_Data * 2>().template insert(1, data_hi);

            auto [coeff_lo, coeff_hi] = interleave_zip<CoeffType, N_Coeff>::run(coeff, coeff, Channels);
            auto coeff2 = coeff_lo.template grow<N_Coeff * 2>().template insert(1, coeff_hi);
            
            auto result = sliding_mul_ch_bits_impl<Outputs, native_channels, Points, 1, 1, 1, 32, 8, 8, coeff_type, data_type>::template run<MulMacroOp::Mul>(
                    coeff2, coeff_start, coeff_sign,
                    data2, data_start,  data_sign);
            
            auto [tmp_lo, tmp_hi] = interleave_unzip<int32, Lanes>::run(result.template extract<Lanes>(0).template to_vector<int32>(),
                                                                        result.template extract<Lanes>(1).template to_vector<int32>(),
                                                                        Channels);
            
            if constexpr (sizeof...(Acc) > 0) {
                return add_accum<accum_tag, Lanes>::run(acc..., false, accum_type(tmp_lo));
            }
            else {
                return accum_type(tmp_lo);
            }
        }

        constexpr auto mac_op = sliding_mul_ch_acc32_get_mul_op<add_to_op<MulOp>(), Lanes, Channels, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_ch_acc32_get_mul_op<             MulOp, Lanes, Channels, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / 64)>([&](auto idx_y) __aie_inline {
            accum_type64 tmp;
            tmp = mul_op(shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                         ((data_start + idx_y * columns_per_mul) * Channels) % data_elems).template extract<128>(0),
                         data_sign,
                         shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), coeff_start * Channels),
                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<64>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                tmp = mac_op(shuffle_down_rotate<data_type, data_elems>::run(data.template grow_replicate<data_elems>(),
                                                                             ((data_start + (idx_y + idx) * columns_per_mul) * Channels) % data_elems).template extract<128>(0),
                             data_sign,
                             shuffle_down_rotate<coeff_type, coeff_elems>::run(coeff.template grow_replicate<coeff_elems>(), (coeff_start + columns_per_mul * idx) * Channels),
                             coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= 64)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif

